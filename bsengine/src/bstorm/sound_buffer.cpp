#include <bstorm/sound_buffer.hpp>

#include <bstorm/ptr_util.hpp>
#include <bstorm/math_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/wave_sample_stream.hpp>
#include <bstorm/sound_device.hpp>

#include <dsound.h>
#include <cassert>

namespace bstorm
{
static inline float TodB(float r)
{
    return 20.0f * log10f(r);
}

static inline float FromdB(float db)
{
    return powf(10.0f, db / 20.0f);
}

static IDirectSoundBuffer8* CreateSoundBuffer(size_t bufSize, const std::unique_ptr<WaveSampleStream>& stream, const std::shared_ptr<SoundDevice>& soundDevice)
{
    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = stream->GetFormatTag();
    waveFormat.nChannels = stream->GetChannelCount();
    waveFormat.nSamplesPerSec = stream->GetSampleRate();
    waveFormat.nAvgBytesPerSec = stream->GetByteRate();
    waveFormat.nBlockAlign = stream->GetBlockAlign();
    waveFormat.wBitsPerSample = stream->GetBitsPerSample();
    waveFormat.cbSize = sizeof(WAVEFORMAT);

    DSBUFFERDESC dsBufferDesc;

    dsBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    dsBufferDesc.dwBufferBytes = bufSize;
    dsBufferDesc.dwReserved = 0;
    dsBufferDesc.dwFlags = DSBCAPS_LOCDEFER
        | DSBCAPS_GLOBALFOCUS
        | DSBCAPS_GETCURRENTPOSITION2
        | DSBCAPS_CTRLVOLUME
        | DSBCAPS_CTRLFREQUENCY
        | DSBCAPS_CTRLPAN; // 3D�@�\�͎g���Ȃ��Ȃ�
    dsBufferDesc.lpwfxFormat = &waveFormat;
    dsBufferDesc.guid3DAlgorithm = GUID_NULL;

    IDirectSoundBuffer* dsTempBuffer = nullptr;
    IDirectSoundBuffer8* dsBuffer = nullptr;

    if (soundDevice->GetDevice()->CreateSoundBuffer(&dsBufferDesc, &dsTempBuffer, nullptr) != DS_OK)
    {
        goto failed_to_create;
    }

    if (dsTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&(dsBuffer)) != DS_OK)
    {
        goto failed_to_create;
    }

    return dsBuffer;
failed_to_create:
    safe_release(dsTempBuffer);
    safe_release(dsBuffer);
    throw Log(LogLevel::LV_ERROR)
        .Msg("Failed to create sound buffer.")
        .Param(LogParam(LogParam::Tag::TEXT, stream->GetPath()));

}

static size_t CalcBufferSize(const std::unique_ptr<WaveSampleStream>& stream)
{
    constexpr float bufSec = 1.0f;
    size_t bufSamples = (size_t)(bufSec * stream->GetSampleRate());
    size_t bytesPerSample = stream->GetBytesPerSample();
    return std::min(bufSamples * bytesPerSample * stream->GetChannelCount(), stream->GetTotalBytes());
}

SoundBuffer::SoundBuffer(std::unique_ptr<WaveSampleStream>&& stream, const std::shared_ptr<SoundDevice>& soundDevice) :
    istream_(std::move(stream)),
    waveFormat_(istream_->GetWaveFormat()),
    bufSize_(CalcBufferSize(istream_)),
    dsBuffer_(CreateSoundBuffer(bufSize_, istream_, soundDevice), com_deleter()),
    isThreadTerminated_(false),
    isLoopEnabled_(false),
    loopRange_(LoopRange(0, istream_->GetTotalBytes())),
    soundMain_(&SoundBuffer::SoundMain, this)
{
}

SoundBuffer::~SoundBuffer()
{
    isThreadTerminated_ = true;
    soundMain_.join();
    Stop(); // NOTE: �Z�J���_���o�b�t�@����O�Ɏ~�߂Ȃ��ƃv���C�}���o�b�t�@�ɉ����c�邱�Ƃ�����
}

#define CRITICAL_SECTION std::lock_guard<std::recursive_mutex> lock(criticalSection_)

void SoundBuffer::Play()
{
    CRITICAL_SECTION;
    dsBuffer_->Play(0, 0, DSBPLAY_LOOPING);
}

void SoundBuffer::Stop()
{
    CRITICAL_SECTION;
    dsBuffer_->Stop();
}

void SoundBuffer::Rewind()
{
    // TODO: �Z���Ԋu�ōs���Ɖ����p�����ĕ�������̂Œ���
    CRITICAL_SECTION;
    dsBuffer_->Stop(); // �Đ���~
    dsBuffer_->SetCurrentPosition(0); // �Đ��J�[�\����擪�ɖ߂�
    istream_->SeekBytes(0); // ���͂�擪�ɖ߂�
    isSetStopCursor_ = false; // �X�g�b�v�J�[�\��������
    FillBufferSector(BufferSector::FirstHalf); // �o�b�t�@�̑O���𖄂߂�
    writeSector_ = BufferSector::LatterHalf; // �o�b�t�@�̌㔼���������ݑΏۂ�
}

void SoundBuffer::SetVolume(float volume)
{
    CRITICAL_SECTION;
    // 1/100dB�Ɋ��Z
    volume = constrain(volume, 0.0f, 1.0f);
    volume = volume == 0.0f ? DSBVOLUME_MIN : (100 * TodB(volume));
    volume = constrain(volume, (float)DSBVOLUME_MIN, (float)DSBVOLUME_MAX);
    dsBuffer_->SetVolume(volume);
}

void SoundBuffer::SetPan(float pan)
{
    CRITICAL_SECTION;
    pan = constrain(pan, -1.0f, 1.0f);
    const bool rightPan = pan > 0;
    pan = 1 - abs(pan); // 0 .. 1
    pan = pan == 0.0f ? DSBPAN_LEFT : (100 * TodB(pan)); // -inf .. 0
    if (rightPan) pan *= -1; // -inf .. 0 .. inf
    pan = constrain(pan, 1.0f*DSBPAN_LEFT, 1.0f*DSBPAN_RIGHT); // -10000 .. 0 .. 10000
    dsBuffer_->SetPan(pan);
}

bool SoundBuffer::IsPlaying() const
{
    CRITICAL_SECTION;
    DWORD status;
    dsBuffer_->GetStatus(&status);
    return (status & DSBSTATUS_PLAYING) != 0;
}

float SoundBuffer::GetVolume() const
{
    CRITICAL_SECTION;
    LONG vol;
    dsBuffer_->GetVolume(&vol);
    return vol == DSBVOLUME_MIN ? 0.0f : FromdB(vol / 100.0f);
}

size_t SoundBuffer::GetCurrentPlayCursor() const
{
    CRITICAL_SECTION;
    DWORD playCursor;
    dsBuffer_->GetCurrentPosition(&playCursor, nullptr);
    return playCursor;
}

void SoundBuffer::SetLoopEnable(bool enable)
{
    isLoopEnabled_ = enable;
}

void SoundBuffer::SetLoopTime(size_t beginSec, size_t endSec)
{
    SetLoopSampleCount(beginSec * waveFormat_.sampleRate, endSec * waveFormat_.sampleRate);
}

void SoundBuffer::SetLoopSampleCount(size_t begin, size_t end)
{
    size_t conv = waveFormat_.bitsPerSample / 8 * waveFormat_.channelCount;
    SetLoopRange(begin * conv, end * conv);
}

void SoundBuffer::SetLoopRange(size_t begin, size_t end)
{
    loopRange_ = LoopRange(begin, end);
}

void SoundBuffer::SoundMain()
{
    Rewind();
    while (!isThreadTerminated_)
    {
        {
            CRITICAL_SECTION;
            if (istream_->IsClosed()) break;

            // fill buffer.
            {
                size_t playCursor = GetCurrentPlayCursor();
                if (writeSector_ == BufferSector::FirstHalf && playCursor >= GetHalfBufferSize() ||
                    writeSector_ == BufferSector::LatterHalf && playCursor < GetHalfBufferSize())
                {
                    FillBufferSector(writeSector_);
                    writeSector_ = writeSector_ == BufferSector::FirstHalf ? BufferSector::LatterHalf : BufferSector::FirstHalf;
                }
            }

            // stop if reach end of stream.
            if (isSetStopCursor_)
            {
                if (IsPlaying())
                {
                    size_t playCursor = GetCurrentPlayCursor();
                    if (stopCursor_ < GetHalfBufferSize())
                    {
                        if (stopCursor_ <= playCursor && playCursor < playCursorOnStop_)
                        {
                            Stop();
                        }
                    } else
                    {
                        if (playCursor < playCursorOnStop_ || stopCursor_ <= playCursor)
                        {
                            Stop();
                        }
                    }
                }
            }
        }
        // NOTE: �X���[�v�������ƃ[�����ߕ����̂���?�Ńm�C�Y���o�邱�Ƃ�����
        Sleep(1);
    }
}

void SoundBuffer::FillBufferSector(BufferSector sector)
{
    CRITICAL_SECTION;

    const size_t sectorOffset = sector == BufferSector::FirstHalf ? 0 : GetHalfBufferSize();
    const size_t sectorSize = sector == BufferSector::FirstHalf ? GetHalfBufferSize() : (bufSize_ - GetHalfBufferSize());

    char *writePtr, *unusedWritePtr;
    DWORD writableBytes, unusedWritableBytes;
    dsBuffer_->Lock(sectorOffset, sectorSize, (LPVOID*)&writePtr, &writableBytes, (LPVOID*)(&unusedWritePtr), &unusedWritableBytes, NULL);

    // never cyclic allocation
    assert(unusedWritePtr == NULL && unusedWritableBytes == 0);
    assert(sectorSize == writableBytes);

    const auto loopRange = loopRange_.load();

    size_t writtenSize = 0;
    while (writtenSize < sectorSize)
    {
        const size_t streamReadPos = istream_->Tell();

        if (isLoopEnabled_ &&
            streamReadPos < waveFormat_.totalBytes && // not stream end
            loopRange.begin < waveFormat_.totalBytes && loopRange.end <= waveFormat_.totalBytes && // valid loop range
            streamReadPos < loopRange.end && loopRange.end <= streamReadPos + (sectorSize - writtenSize) // over loop end
            )
        {
            // loop
            writtenSize += istream_->ReadBytes(loopRange.end - streamReadPos, writePtr + writtenSize);
            istream_->SeekBytes(loopRange.begin);
        } else
        {
            writtenSize += istream_->ReadBytes(sectorSize - writtenSize, writePtr + writtenSize);
        }

        if (istream_->IsEnd() || istream_->IsClosed())
        {
            // �Z�N�^�̎c��𖳉��Ŗ��߂�
            FillMemory(writePtr + writtenSize, sectorSize - writtenSize, waveFormat_.bitsPerSample == 8 ? 0x80 : 0);

            // �ŏ��ɃX�g���[���I�[�ɓ��B�����Ƃ�����, �I���ʒu���L�^
            if (!isSetStopCursor_)
            {
                isSetStopCursor_ = true;
                stopCursor_ = sectorOffset + writtenSize;
                playCursorOnStop_ = GetCurrentPlayCursor();
            }
            break;
        }
    }

    dsBuffer_->Unlock(writePtr, writableBytes, unusedWritePtr, unusedWritableBytes);
}
}

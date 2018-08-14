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
        | DSBCAPS_CTRLPAN; // 3D機能は使えなくなる
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
    throw Log(Log::Level::LV_ERROR)
        .SetMessage("failed to create sound buffer.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, stream->GetPath()));

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
    Stop(); // NOTE: セカンダリバッファ解放前に止めないとプライマリバッファに音が残ることがある
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
    // TODO: 短い間隔で行うと音がパラついて聞こえるので調査
    CRITICAL_SECTION;
    dsBuffer_->Stop(); // 再生停止
    dsBuffer_->SetCurrentPosition(0); // 再生カーソルを先頭に戻す
    istream_->SeekBytes(0); // 入力を先頭に戻す
    isSetStopCursor_ = false; // ストップカーソルを消す
    FillBufferSector(BufferSector::FirstHalf); // バッファの前半を埋める
    writeSector_ = BufferSector::LatterHalf; // バッファの後半を書き込み対象に
}

void SoundBuffer::SetVolume(float volume)
{
    CRITICAL_SECTION;
    // 1/100dBに換算
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
        // NOTE: スリープが長いとゼロ埋め部分のせい?でノイズが出ることがある
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

    size_t writedSize = 0;
    while (writedSize < sectorSize)
    {
        const size_t restWrite = sectorSize - writedSize;
        const size_t streamReadPos = istream_->Tell();

        if (isLoopEnabled_ &&
            streamReadPos < waveFormat_.totalBytes && // not stream end
            loopRange.begin < waveFormat_.totalBytes && loopRange.end <= waveFormat_.totalBytes && // valid loop range
            streamReadPos < loopRange.end && loopRange.end <= streamReadPos + restWrite // over loop end
            )
        {
            // loop
            auto justRead = istream_->ReadBytes(loopRange.end - streamReadPos, writePtr + writedSize);
            writedSize += justRead;
            istream_->SeekBytes(loopRange.begin);
        } else
        {
            auto justRead = istream_->ReadBytes(restWrite, writePtr + writedSize);
            writedSize += justRead;
        }

        if (istream_->IsEnd() || istream_->IsClosed())
        {
            // セクタの残りを無音で埋める
            FillMemory(writePtr + writedSize, sectorSize - writedSize, waveFormat_.bitsPerSample == 8 ? 0x80 : 0);

            // 最初にストリーム終端に到達したときだけ, 終了位置を記録
            if (!isSetStopCursor_)
            {
                isSetStopCursor_ = true;
                stopCursor_ = sectorOffset + writedSize;
                playCursorOnStop_ = GetCurrentPlayCursor();
            }
            break;
        }
    }

    dsBuffer_->Unlock(writePtr, writableBytes, unusedWritePtr, unusedWritableBytes);
}
}

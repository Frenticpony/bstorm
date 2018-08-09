#include <bstorm/sound_buffer.hpp>

#include <bstorm/logger.hpp>
#include <bstorm/ptr_util.hpp>
#include <bstorm/math_util.hpp>
#include <bstorm/sound_device.hpp>
#include <bstorm/wave_sample_stream.hpp>

#include <cassert>

namespace bstorm
{
static size_t CalcBufferSize(double bufSec, const WAVEFORMATEX& waveFormat)
{
    size_t bufSamples = (size_t)(bufSec * waveFormat.nSamplesPerSec);
    size_t bytesPerSample = waveFormat.wBitsPerSample / 8;
    return  bufSamples * bytesPerSample * waveFormat.nChannels;
}

static IDirectSoundBuffer8* CreateSoundBuffer(double bufSize, WAVEFORMATEX* waveFormat, const std::shared_ptr<SoundDevice>& soundDevice)
{
    DSBUFFERDESC dSBufferDesc;

    dSBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    dSBufferDesc.dwBufferBytes = bufSize;
    dSBufferDesc.dwReserved = 0;
    dSBufferDesc.dwFlags = DSBCAPS_LOCDEFER
        | DSBCAPS_GLOBALFOCUS
        | DSBCAPS_GETCURRENTPOSITION2
        | DSBCAPS_CTRLVOLUME
        | DSBCAPS_CTRLFREQUENCY
        | DSBCAPS_CTRLPAN; // 3D機能は使えなくなる
    dSBufferDesc.lpwfxFormat = waveFormat;
    dSBufferDesc.guid3DAlgorithm = GUID_NULL;

    IDirectSoundBuffer* dSoundTempBuffer = nullptr;
    if (soundDevice->GetDevice()->CreateSoundBuffer(&dSBufferDesc, &dSoundTempBuffer, nullptr) != DS_OK)
    {
        return nullptr;
    }

    IDirectSoundBuffer8* dsBuffer;
    dSoundTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&(dsBuffer));
    safe_release(dSoundTempBuffer);
    return dsBuffer;
}

static inline float TodB(float r)
{
    return 20.0f * log10f(r);
}

enum class BufferSector
{
    FirstHalf,
    LatterHalf
};

// データストリームからバッファへと書き込みを行う
static void FillBufferSector(BufferSector writeSector,
                             const std::unique_ptr<IDirectSoundBuffer8, com_deleter>& dsBuffer,
                             const std::unique_ptr<WaveSampleStream>& stream,
                             size_t bufSize,
                             size_t& stopPos,
                             DWORD& playPosOnStop,
                             SoundBuffer* soundBuffer)

{
    //  NOTE: 読み込み, 書き込みは全てバイト単位で行う。size, posなどの変数は全てbyte長
    size_t halfBufSize = bufSize / 2;
    size_t sectorWritePos = 0;
    size_t sectorSize = 0;
    switch (writeSector)
    {
        case BufferSector::FirstHalf:
            sectorWritePos = 0;
            sectorSize = halfBufSize;
            break;
        case BufferSector::LatterHalf:
            sectorWritePos = halfBufSize;
            sectorSize = bufSize - halfBufSize;
            break;
    }

    char *writePtr, *unusedWritePtr;
    DWORD writableBytes, unusedWritableBytes;
    dsBuffer->Lock(sectorWritePos, sectorSize, (LPVOID*)&writePtr, &writableBytes, (LPVOID*)(&unusedWritePtr), &unusedWritableBytes, NULL);

    // never cyclic allocation
    assert(unusedWritePtr == NULL && unusedWritableBytes == 0);
    assert(sectorSize == writableBytes);

    size_t loopStartPos = soundBuffer->GetLoopStartSampleCount() * stream->GetBytesPerSample() * stream->GetChannelCount();
    size_t loopEndPos = soundBuffer->GetLoopEndSampleCount() * stream->GetBytesPerSample() * stream->GetChannelCount();

    size_t streamSize = stream->GetTotalBytes();

    size_t writedSize = 0;
    while (!stream->IsEnd() && writedSize < sectorSize && !stream->IsClosed())
    {
        const size_t restWrite = sectorSize - writedSize;
        const size_t streamOffset = stream->Tell();

        if (soundBuffer->IsLoopEnabled() && streamOffset < loopEndPos && streamOffset + restWrite >= loopEndPos)
        {
            // loop
            auto justRead = stream->ReadBytes(loopEndPos - streamOffset, writePtr + writedSize);
            writedSize += justRead;
            stream->SeekBytes(loopStartPos);
        } else
        {
            auto justRead = stream->ReadBytes(restWrite, writePtr + writedSize);
            writedSize += justRead;
        }
    }

    if (writedSize < sectorSize)
    {
        // 途中で終わったら無音で埋める
        FillMemory(writePtr + writedSize, sectorSize - writedSize, stream->GetBitsPerSample() == 8 ? 0x80 : 0);

        // 最初にストリーム終端に到達したときだけ, 終了位置を記録
        if (writedSize != 0)
        {
            stopPos = sectorWritePos + writedSize;
            dsBuffer->GetCurrentPosition(&playPosOnStop, nullptr);
        }
    }

    dsBuffer->Unlock(writePtr, writableBytes, unusedWritePtr, unusedWritableBytes);
}

// sound thread
static void SoundMain(std::unique_ptr<IDirectSoundBuffer8, com_deleter>&& dsBuffer,
                      std::unique_ptr<WaveSampleStream>&& stream,
                      concurrency::concurrent_queue<SoundRequest>* requestQueue,
                      size_t bufSize,
                      SoundBuffer* soundBuffer)
{
    const size_t halfBufSize = bufSize / 2;
    size_t stopPos = ~0;
    DWORD playPosOnStop = ~0;
    stream->SeekBytes(0);
    FillBufferSector(BufferSector::FirstHalf, dsBuffer, stream, bufSize, stopPos, playPosOnStop, soundBuffer);
    BufferSector writeSector = BufferSector::LatterHalf;
    while (true)
    {
        if (stream->IsClosed()) break;

        // request handling
        SoundRequest request;
        while (requestQueue->try_pop(request))
        {
            switch (request)
            {
                case SoundRequest::PLAY:
                    dsBuffer->Play(0, 0, DSBPLAY_LOOPING);
                    break;
                case SoundRequest::STOP:
                    dsBuffer->Stop();
                    break;
                case SoundRequest::REWIND:
                    dsBuffer->Stop();
                    dsBuffer->SetCurrentPosition(0);
                    stream->SeekBytes(0);
                    stopPos = ~0;
                    FillBufferSector(BufferSector::FirstHalf, dsBuffer, stream, bufSize, stopPos, playPosOnStop, soundBuffer);
                    writeSector = BufferSector::LatterHalf;
                    break;
                case SoundRequest::SET_VOLUME:
                {
                    // 1/100dBに換算
                    float volume = soundBuffer->GetVolume();
                    volume = volume == 0.0f ? DSBVOLUME_MIN : (100 * TodB(volume));
                    volume = constrain(volume, (float)DSBVOLUME_MIN, (float)DSBVOLUME_MAX);
                    dsBuffer->SetVolume(volume);
                }
                break;
                case SoundRequest::SET_PAN:
                {
                    float pan = soundBuffer->GetPan();
                    const bool rightPan = pan > 0;
                    pan = 1 - abs(pan); // 0 .. 1
                    pan = pan == 0.0f ? DSBPAN_LEFT : (100 * TodB(pan)); // -inf .. 0
                    if (rightPan) pan *= -1; // -inf .. 0 .. inf
                    pan = constrain(pan, 1.0f*DSBPAN_LEFT, 1.0f*DSBPAN_RIGHT); // -10000 .. 0 .. 10000
                    dsBuffer->SetPan(pan);
                }
                break;
                case SoundRequest::EXIT:
                default:
                    return;
            }
        }

        DWORD playPos;
        dsBuffer->GetCurrentPosition(&playPos, nullptr);
        // filling sound buffer
        if (writeSector == BufferSector::FirstHalf && playPos >= halfBufSize ||
            writeSector == BufferSector::LatterHalf && playPos < halfBufSize)
        {
            FillBufferSector(writeSector, dsBuffer, stream, bufSize, stopPos, playPosOnStop, soundBuffer);

            // swap write sector
            if (writeSector == BufferSector::FirstHalf)
            {
                writeSector = BufferSector::LatterHalf;
            } else if (writeSector == BufferSector::LatterHalf)
            {
                writeSector = BufferSector::FirstHalf;
            }
        }

        if (stopPos <= bufSize)
        {
            DWORD status;
            dsBuffer->GetStatus(&status);
            bool isPlaying = (status & DSBSTATUS_PLAYING) != 0;
            if (isPlaying)
            {
                bool doStop = false;
                dsBuffer->GetCurrentPosition(&playPos, nullptr);
                if (stopPos < halfBufSize)
                {
                    // stopPosが前半にあるとき
                    if (stopPos <= playPos && playPos < playPosOnStop)
                    {
                        doStop = true;
                    }
                } else
                {
                    // stopPosが後半にあるとき
                    if (stopPos <= playPos || playPos < playPosOnStop)
                    {
                        doStop = true;
                    }
                }

                if (doStop)
                {
                    dsBuffer->Stop();
                    soundBuffer->Stop();
                }
            }
        }
        Sleep(1);
    }
}

SoundBuffer::SoundBuffer(std::unique_ptr<WaveSampleStream>&& stream, const std::shared_ptr<SoundDevice>& soundDevice) :
    path_(stream->GetPath()),
    volume_(1.0f),
    pan_(0.0f),
    sampleRate_(stream->GetSampleRate()),
    isPlaying_(false),
    loopEnable_(false),
    loopStartSampleCnt_(0),
    loopEndSampleCnt_(stream->GetTotalSampleCount())
{
    WAVEFORMATEX waveFormat;

    waveFormat.wFormatTag = stream->GetFormatTag();
    waveFormat.nChannels = stream->GetChannelCount();
    waveFormat.nSamplesPerSec = stream->GetSampleRate();
    waveFormat.nAvgBytesPerSec = stream->GetByteRate();
    waveFormat.nBlockAlign = stream->GetBlockAlign();
    waveFormat.wBitsPerSample = stream->GetBitsPerSample();
    waveFormat.cbSize = sizeof(WAVEFORMAT);

    constexpr double bufSec = 1.0;
    auto bufSize = std::min(CalcBufferSize(bufSec, waveFormat), stream->GetTotalBytes());
    auto buf = CreateSoundBuffer(bufSize, &waveFormat, soundDevice);
    if (buf == nullptr)
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("failed to create sound buffer.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, GetPath()));
    }
    auto dsBuffer = std::unique_ptr<IDirectSoundBuffer8, com_deleter>(buf, com_deleter());
    dsBuffer->SetVolume(DSBVOLUME_MAX);
    dsBuffer->SetPan(DSBPAN_CENTER);
    soundMain_ = std::thread(SoundMain, std::move(dsBuffer), std::move(stream), &soundRequestQueue_, bufSize, this);
}

SoundBuffer::~SoundBuffer()
{
    soundRequestQueue_.push(SoundRequest::EXIT);
    soundMain_.join();
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("release sound.")
        .SetParam(Log::Param(Log::Param::Tag::SOUND, path_))));
}

void SoundBuffer::Play()
{
    isPlaying_ = true;
    soundRequestQueue_.push(SoundRequest::PLAY);
}

void SoundBuffer::Stop()
{
    isPlaying_ = false;
    soundRequestQueue_.push(SoundRequest::STOP);
}

void SoundBuffer::Rewind()
{
    soundRequestQueue_.push(SoundRequest::REWIND);
}

void SoundBuffer::SetVolume(float vol)
{
    volume_ = constrain(vol, 0.0f, 1.0f);
    soundRequestQueue_.push(SoundRequest::SET_VOLUME);
}

void SoundBuffer::SetPanRate(float pan)
{
    pan_ = constrain(pan, -1.0f, 1.0f);
    soundRequestQueue_.push(SoundRequest::SET_PAN);
}

void SoundBuffer::SetLoopEnable(bool enable)
{
    loopEnable_ = enable;
}

void SoundBuffer::SetLoopSampleCount(size_t start, size_t end)
{
    loopStartSampleCnt_ = start;
    loopEndSampleCnt_ = end;
}

void SoundBuffer::SetLoopTime(double startSec, double endSec)
{
    SetLoopSampleCount((size_t)(sampleRate_ * startSec), (size_t)(sampleRate_ * endSec));
}

const std::wstring& SoundBuffer::GetPath() const
{
    return path_;
}

bool SoundBuffer::IsPlaying() const
{
    return isPlaying_;
}

float SoundBuffer::GetVolume() const
{
    return volume_;
}

float SoundBuffer::GetPan() const
{
    return pan_;
}

bool SoundBuffer::IsLoopEnabled() const
{
    return loopEnable_;
}

size_t SoundBuffer::GetLoopStartSampleCount() const
{
    return loopStartSampleCnt_;
}

size_t SoundBuffer::GetLoopEndSampleCount() const
{
    return loopEndSampleCnt_;
}
}
#include <bstorm/sound_device.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>

#include <mmsystem.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace bstorm
{
IDirectSoundBuffer8 * SoundDataLoaderFromSoundFile::LoadSoundData(const std::wstring & path, IDirectSound8 * dSound)
{
    IDirectSoundBuffer8* dSoundBuffer = nullptr;
    // waveデータの書き込み
    std::wstring ext(GetLowerExt(path));
    if (ext == L".wave" || ext == L".wav")
    {
        dSoundBuffer = ReadWaveFile(path, dSound);
    } else if (ext == L".ogg")
    {
        dSoundBuffer = ReadOggVorbisFile(path, dSound);
    } else
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("unsupported sound file format.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    }
    return dSoundBuffer;
}

static DWORD WINAPI SoundControlThread(LPVOID arg)
{
    SoundBuffer* soundBuffer = (SoundBuffer*)arg;
    while (true)
    {
        HANDLE loopEndEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, soundBuffer->GetLoopEndEventName().c_str());
        auto ret = WaitForSingleObject(loopEndEvent, INFINITE);
        ResetEvent(loopEndEvent);
        CloseHandle(loopEndEvent);
        if (ret == WAIT_OBJECT_0)
        {
            OutputDebugStringW(L"catch sound control event.\n");
            if (soundBuffer->IsLoopEnabled())
            {
                soundBuffer->Seek(soundBuffer->GetLoopStartSampleCount());
            }
        } else
        {
            OutputDebugStringW(L"unexpected sound control error occured.\n");
            return 1;
        }
    }
    return 0;
}

SoundBuffer::SoundBuffer(const std::wstring& path, IDirectSoundBuffer8* buf) :
    path_(path),
    dSoundBuffer_(buf),
    loopEnable_(false),
    loopStartSampleCnt_(0),
    loopEndSampleCnt_(DSBPN_OFFSETSTOP),
    controlThread_(nullptr),
    loopEndEvent_(nullptr)
{
    try
    {
        dSoundBuffer_->SetVolume(DSBVOLUME_MAX);
        dSoundBuffer_->SetPan(DSBPAN_CENTER);

        WAVEFORMATEX waveFormat;
        if (FAILED(dSoundBuffer_->GetFormat(&waveFormat, sizeof(waveFormat), nullptr)))
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to get sound buffer format.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        }
        samplePerSec_ = waveFormat.nSamplesPerSec;
        bytesPerSample_ = waveFormat.wBitsPerSample / 8;
        channelCnt_ = waveFormat.nChannels;

        loopEndEvent_ = CreateEvent(nullptr, TRUE, FALSE, GetLoopEndEventName().c_str());
        if (loopEndEvent_ == nullptr)
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to create loop end event.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        }

        controlThread_ = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SoundControlThread, this, 0, nullptr);
        if (controlThread_ == nullptr)
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to create sound control thread.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        }
    } catch (...)
    {
        if (loopEndEvent_)
        {
            CloseHandle(loopEndEvent_);
        }
        if (controlThread_)
        {
            TerminateThread(controlThread_, 1);
            CloseHandle(controlThread_);
        }
        safe_release(dSoundBuffer_);
        throw;
    }
}

SoundBuffer::SoundBuffer(const std::shared_ptr<SoundBuffer>& src, IDirectSound8* dSound) :
    path_(src->path_),
    dSoundBuffer_(nullptr),
    loopEnable_(false),
    loopStartSampleCnt_(0),
    loopEndSampleCnt_(DSBPN_OFFSETSTOP),
    controlThread_(nullptr),
    loopEndEvent_(nullptr)
{
    try
    {
        IDirectSoundBuffer* dSoundTempBuffer = nullptr;
        if (DS_OK != dSound->DuplicateSoundBuffer(src->dSoundBuffer_, &dSoundTempBuffer))
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to duplicate sound buffer.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path_));
        }
        dSoundTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&dSoundBuffer_);
        // バグがあるので、バッファ複製直後に元の音量からちょっとずらす必要があるらしい
        LONG origVolume = 0; // 元の音量
        src->dSoundBuffer_->GetVolume(&origVolume);
        dSoundBuffer_->SetVolume(origVolume - 100);
        // 最大音量に設定する
        dSoundBuffer_->SetVolume(DSBVOLUME_MAX);
        dSoundBuffer_->SetPan(DSBPAN_CENTER);
        safe_release(dSoundTempBuffer);

        WAVEFORMATEX waveFormat;
        if (FAILED(dSoundBuffer_->GetFormat(&waveFormat, sizeof(waveFormat), nullptr)))
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to get sound buffer format.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path_));
        }
        samplePerSec_ = waveFormat.nSamplesPerSec;
        bytesPerSample_ = waveFormat.wBitsPerSample / 8;
        channelCnt_ = waveFormat.nChannels;

        loopEndEvent_ = CreateEvent(nullptr, TRUE, FALSE, GetLoopEndEventName().c_str());
        if (loopEndEvent_ == nullptr)
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to create loop end event.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path_));
        }

        controlThread_ = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SoundControlThread, this, 0, nullptr);
        if (controlThread_ == nullptr)
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to create sound control thread.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path_));
        }
    } catch (...)
    {
        if (loopEndEvent_)
        {
            CloseHandle(loopEndEvent_);
        }
        if (controlThread_)
        {
            TerminateThread(controlThread_, 1);
            CloseHandle(controlThread_);
        }
        safe_release(dSoundBuffer_);
        throw;
    }
}

void SoundBuffer::Play()
{
    dSoundBuffer_->Play(0, 0, loopEnable_ ? DSBPLAY_LOOPING : 0);
}

void SoundBuffer::Stop()
{
    dSoundBuffer_->Stop();
}

void SoundBuffer::SetVolume(float vol)
{
    vol = constrain(vol, 0.0f, 1.0f);
    // NOTE : 1000だと値に比べて大きく聞こえるので3000にした。音量1/2で-9dBぐらいになる調整
    vol = vol == 0.0f ? DSBVOLUME_MIN : (3000.0f * log10f(vol));
    vol = constrain(vol, 1.0f*DSBVOLUME_MIN, 1.0f*DSBVOLUME_MAX);
    dSoundBuffer_->SetVolume(vol);
}

void SoundBuffer::SetPanRate(float pan)
{
    pan = constrain(pan, -1.0f, 1.0f);
    const bool rightPan = pan > 0;
    pan = 1 - abs(pan); // 中央を1(倍)にする
    pan = pan == 0 ? DSBPAN_LEFT : (1000.0f * log10f(pan));
    if (rightPan) pan *= -1;
    pan = constrain(pan, 1.0f*DSBPAN_LEFT, 1.0f*DSBPAN_RIGHT);
    dSoundBuffer_->SetPan(pan);
}

void SoundBuffer::Seek(int sample)
{
    dSoundBuffer_->SetCurrentPosition(sample * bytesPerSample_ * channelCnt_);
}

void SoundBuffer::SetLoopEnable(bool enable)
{
    loopEnable_ = enable;
}

void SoundBuffer::SetLoopSampleCount(DWORD start, DWORD end)
{
    loopStartSampleCnt_ = start;
    loopEndSampleCnt_ = end;
    IDirectSoundNotify8* soundNotify = nullptr;
    dSoundBuffer_->QueryInterface(IID_IDirectSoundNotify8, (void**)&soundNotify);
    DSBPOSITIONNOTIFY notifyPos;
    notifyPos.dwOffset = loopEndSampleCnt_ * bytesPerSample_ * channelCnt_;
    notifyPos.hEventNotify = loopEndEvent_;
    soundNotify->SetNotificationPositions(1, &notifyPos);
    safe_release(soundNotify);
}

void SoundBuffer::SetLoopTime(double startSec, double endSec)
{
    SetLoopSampleCount((DWORD)(samplePerSec_ * startSec), (DWORD)(samplePerSec_ * endSec));
}

bool SoundBuffer::IsPlaying()
{
    DWORD status;
    dSoundBuffer_->GetStatus(&status);
    return (status & DSBSTATUS_PLAYING) != 0;
}

float SoundBuffer::GetVolume()
{
    LONG vol;
    dSoundBuffer_->GetVolume(&vol);
    return vol == DSBVOLUME_MIN ? 0.0f : powf(10.0f, vol / 3000.0f);
}

const std::wstring & SoundBuffer::GetPath() const
{
    return path_;
}

bool SoundBuffer::IsLoopEnabled() const
{
    return loopEnable_;
}

std::wstring SoundBuffer::GetLoopEndEventName() const
{
    size_t id = (size_t)this;
    return L"LOOP_END_" + std::to_wstring(id);
}

DWORD SoundBuffer::GetLoopStartSampleCount() const
{
    return loopStartSampleCnt_;
}

DWORD SoundBuffer::GetLoopEndSampleCount() const
{
    return loopEndSampleCnt_;
}

SoundBuffer::~SoundBuffer()
{
    TerminateThread(controlThread_, 0);
    CloseHandle(controlThread_);
    CloseHandle(loopEndEvent_);
    safe_release(dSoundBuffer_);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("release sound.")
        .SetParam(Log::Param(Log::Param::Tag::SOUND, path_))));
}

SoundDevice::SoundDevice(HWND hWnd) :
    hWnd_(hWnd),
    dSound_(nullptr),
    loader_(std::make_shared<SoundDataLoaderFromSoundFile>())
{
    if (DS_OK != DirectSoundCreate8(nullptr, &dSound_, nullptr))
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("failed to init sound device.");
    }
    dSound_->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
}

SoundDevice::~SoundDevice()
{
    safe_release(dSound_);
}

std::shared_ptr<SoundBuffer> SoundDevice::LoadSound(const std::wstring & path, bool doCache, const std::shared_ptr<SourcePos>& srcPos)
{
    auto uniqPath = GetCanonicalPath(path);
    auto it = cache_.find(uniqPath);
    if (it != cache_.end())
    {
        return std::make_shared<SoundBuffer>(it->second, dSound_);
    }
    IDirectSoundBuffer8* buf = loader_->LoadSoundData(path, dSound_);
    if (buf == nullptr)
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("failed to load sound data.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    }
    auto sound = std::make_shared<SoundBuffer>(uniqPath, buf);
    if (doCache)
    {
        cache_[uniqPath] = sound;
    }
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("load sound.")
        .SetParam(Log::Param(Log::Param::Tag::SOUND, uniqPath)))
        .AddSourcePos(srcPos));
    return sound;
}

void SoundDevice::SetLoader(const std::shared_ptr<SoundDataLoader>& ld)
{
    loader_ = ld;
}

void SoundDevice::RemoveSoundCache(const std::wstring & path)
{
    cache_.erase(path);
}

void SoundDevice::ClearSoundCache()
{
    cache_.clear();
}

static IDirectSoundBuffer8* createSoundBuffer(DWORD dataSize, WAVEFORMATEX* waveFormat, IDirectSound8* dSound)
{
    DSBUFFERDESC dSBufferDesc;

    dSBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    dSBufferDesc.dwBufferBytes = dataSize;
    dSBufferDesc.dwReserved = 0;
    dSBufferDesc.dwFlags = DSBCAPS_LOCDEFER
        | DSBCAPS_GLOBALFOCUS
        | DSBCAPS_GETCURRENTPOSITION2
        | DSBCAPS_CTRLVOLUME
        | DSBCAPS_CTRLFREQUENCY
        | DSBCAPS_CTRLPAN
        | DSBCAPS_CTRLPOSITIONNOTIFY;
    // CTRLFXを追加するとDuplicateSoundBufferで複製できなくなるので注意
    dSBufferDesc.lpwfxFormat = waveFormat;
    dSBufferDesc.guid3DAlgorithm = GUID_NULL;

    IDirectSoundBuffer* dSoundTempBuffer = nullptr;
    if (DS_OK != dSound->CreateSoundBuffer(&dSBufferDesc, &dSoundTempBuffer, nullptr))
    {
        return nullptr;
    }
    IDirectSoundBuffer8* dSoundBuffer;
    dSoundTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&(dSoundBuffer));
    safe_release(dSoundTempBuffer);
    return dSoundBuffer;
}

IDirectSoundBuffer8* ReadWaveFile(const std::wstring & path, IDirectSound8* dSound)
{
    IDirectSoundBuffer8* dSoundBuffer = nullptr;
    HMMIO hMmio = nullptr;

    MMIOINFO mmioInfo;
    memset(&mmioInfo, 0, sizeof(MMIOINFO));
    hMmio = mmioOpen((LPWSTR)path.c_str(), &mmioInfo, MMIO_READ);
    if (!hMmio)
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("can't open file")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    }

    auto illegal_wave_format = Log(Log::Level::LV_ERROR)
        .SetMessage("illegal wave format file.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    auto failed_to_load_wave = Log(Log::Level::LV_ERROR)
        .SetMessage("failed to load wave file.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, path));

    try
    {
        MMRESULT mmResult;
        MMCKINFO riffChunk;
        riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
        mmResult = mmioDescend(hMmio, &riffChunk, nullptr, MMIO_FINDRIFF);
        if (mmResult != MMSYSERR_NOERROR)
        {
            throw illegal_wave_format;
        }

        MMCKINFO formatChunk;
        formatChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
        mmResult = mmioDescend(hMmio, &formatChunk, &riffChunk, MMIO_FINDCHUNK);
        if (mmResult != MMSYSERR_NOERROR)
        {
            throw illegal_wave_format;
        }

        WAVEFORMATEX waveFormat;
        DWORD fmSize = formatChunk.cksize;
        LONG size = mmioRead(hMmio, (HPSTR)&(waveFormat), fmSize);
        if (size != fmSize)
        {
            throw illegal_wave_format;
        }

        mmioAscend(hMmio, &formatChunk, 0);

        MMCKINFO dataChunk;
        dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
        mmResult = mmioDescend(hMmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK);
        if (mmResult != MMSYSERR_NOERROR)
        {
            throw illegal_wave_format;
        }

        DWORD totalDataSize = dataChunk.cksize;

        dSoundBuffer = createSoundBuffer(totalDataSize, &waveFormat, dSound);
        if (dSoundBuffer == nullptr)
        {
            throw failed_to_load_wave;
        }

        char* writePoint = nullptr;
        DWORD writeLength = 0;
        if (DS_OK != dSoundBuffer->Lock(0, 0, (LPVOID*)&writePoint, &writeLength, nullptr, nullptr, DSBLOCK_ENTIREBUFFER))
        {
            throw failed_to_load_wave;
        }
        size = mmioRead(hMmio, (HPSTR)writePoint, totalDataSize);
        if (size != totalDataSize)
        {
            throw failed_to_load_wave;
        }
        dSoundBuffer->Unlock(writePoint, writeLength, nullptr, 0);
    } catch (...)
    {
        safe_release(dSoundBuffer);
        mmioClose(hMmio, 0);
        throw;
    }
    mmioClose(hMmio, 0);
    return dSoundBuffer;
}

IDirectSoundBuffer8 * ReadOggVorbisFile(const std::wstring & path, IDirectSound8 * dSound)
{
    auto cant_open_file = Log(Log::Level::LV_ERROR)
        .SetMessage("can't open ogg-vorbis file.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    auto illegal_vorbis_format = Log(Log::Level::LV_ERROR)
        .SetMessage("illegal ogg-vorbis format file.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    auto failed_to_load_vorbis = Log(Log::Level::LV_ERROR)
        .SetMessage("failed to load ogg-vorbis file.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, path));


    FILE* fp = _wfopen(path.c_str(), L"rb");
    if (fp == nullptr)
    {
        throw cant_open_file;
    }
    OggVorbis_File vf;
    if (ov_open(fp, &vf, nullptr, 0) != 0)
    {
        fclose(fp);
        throw cant_open_file;
    }

    IDirectSoundBuffer8* dSoundBuffer = nullptr;

    try
    {
        vorbis_info *vi = ov_info(&vf, -1);
        if (vi == nullptr)
        {
            throw illegal_vorbis_format;
        }

        DWORD totalDataSize = (DWORD)ov_pcm_total(&vf, -1) * vi->channels * 2;

        WAVEFORMATEX waveFormat;
        waveFormat.cbSize = sizeof(WAVEFORMATEX);
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = vi->channels;
        waveFormat.nSamplesPerSec = vi->rate;
        waveFormat.nAvgBytesPerSec = vi->rate * vi->channels * 2;
        waveFormat.nBlockAlign = vi->channels * 2;
        waveFormat.wBitsPerSample = 16;

        dSoundBuffer = createSoundBuffer(totalDataSize, &waveFormat, dSound);
        if (dSoundBuffer == nullptr)
        {
            throw failed_to_load_vorbis;
        }

        char* writePoint = nullptr;
        DWORD writeLength = 0;
        if (DS_OK != dSoundBuffer->Lock(0, 0, (LPVOID*)&writePoint, &writeLength, nullptr, nullptr, DSBLOCK_ENTIREBUFFER))
        {
            throw failed_to_load_vorbis;
        }

        int bs;
        char* writeEnd = writePoint + totalDataSize;
        while (true)
        {
            auto size = ov_read(&vf, writePoint, writeEnd - writePoint, 0, sizeof(WORD), 1, &bs);
            if (size < 0)
            {
                throw failed_to_load_vorbis;
            } else if (size == 0)
            {
                if (writePoint != writeEnd)
                {
                    throw illegal_vorbis_format;
                }
                break;
            }
            writePoint += size;
        }
        dSoundBuffer->Unlock(writePoint, writeLength, nullptr, 0);
    } catch (...)
    {
        ov_clear(&vf);
        fclose(fp);
        safe_release(dSoundBuffer);
        throw;
    }

    ov_clear(&vf);
    fclose(fp);
    return dSoundBuffer;
}
}
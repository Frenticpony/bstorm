#include <bstorm/sound_device.hpp>

#include <bstorm/file_util.hpp>
#include <bstorm/ptr_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/sound_buffer.hpp>

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

std::shared_ptr<SoundBuffer> SoundDevice::LoadSound(const std::wstring & path)
{
    auto uniqPath = GetCanonicalPath(path);
    IDirectSoundBuffer8* buf = loader_->LoadSoundData(path, dSound_);
    if (buf == nullptr)
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("failed to load sound data.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    }
    auto sound = std::make_shared<SoundBuffer>(uniqPath, buf);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("load sound.")
        .SetParam(Log::Param(Log::Param::Tag::SOUND, uniqPath))));
    return sound;
}

void SoundDevice::SetLoader(const std::shared_ptr<SoundDataLoader>& ld)
{
    loader_ = ld;
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
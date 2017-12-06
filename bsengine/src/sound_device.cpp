#include <dsound.h>
#include <mmsystem.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <bstorm/util.hpp>
#include <bstorm/sound_device.hpp>

namespace bstorm {
  IDirectSoundBuffer8 * SoundDataLoaderFromSoundFile::loadSoundData(const std::wstring & path, IDirectSound8 * dSound) {
    IDirectSoundBuffer8* dSoundBuffer = NULL;
    // waveデータの書き込み
    std::wstring ext(getLowerExt(path));
    if (ext == L".wave" || ext == L".wav") {
      dSoundBuffer = readWaveFile(path, dSound);
    } else if (ext == L".ogg") {
      dSoundBuffer = readOggVorbisFile(path, dSound);
    } else {
      throw std::runtime_error("unsupported sound file format: " + toUTF8(path));
    }
    return dSoundBuffer;
  }

  static DWORD WINAPI SoundControlThread(LPVOID arg) {
    SoundBuffer* soundBuffer = (SoundBuffer*)arg;
    while (true) {
      HANDLE loopEndEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, soundBuffer->getLoopEndEventName().c_str());
      auto ret = WaitForSingleObject(loopEndEvent, INFINITE);
      ResetEvent(loopEndEvent);
      CloseHandle(loopEndEvent);
      if (ret == WAIT_OBJECT_0) {
        OutputDebugStringW(L"catch sound control event.\n");
        if (soundBuffer->isLoopEnabled()) {
          soundBuffer->seek(soundBuffer->getLoopStartSampleCount());
        }
      } else {
        OutputDebugStringW(L"unexpected sound control error occured.\n");
        return 1;
      }
    }
    return 0;
  }

  SoundBuffer::SoundBuffer(const std::wstring& path, IDirectSoundBuffer8* buf) :
    path(path),
    dSoundBuffer(buf),
    loopEnable(false),
    loopStartSampleCnt(0),
    loopEndSampleCnt(DSBPN_OFFSETSTOP),
    controlThread(NULL),
    loopEndEvent(NULL)
  {
    try {
      dSoundBuffer->SetVolume(DSBVOLUME_MAX);
      dSoundBuffer->SetPan(DSBPAN_CENTER);

      WAVEFORMATEX waveFormat;
      if (FAILED(dSoundBuffer->GetFormat(&waveFormat, sizeof(waveFormat), NULL))) {
        throw std::runtime_error("failed to get sound buffer format: " + toUTF8(path));
      }
      samplePerSec = waveFormat.nSamplesPerSec;
      bytesPerSample = waveFormat.wBitsPerSample / 8;
      channelCnt = waveFormat.nChannels;

      loopEndEvent = CreateEvent(NULL, TRUE, FALSE, getLoopEndEventName().c_str());
      if (loopEndEvent == NULL) {
        throw std::runtime_error("failed to create loop end event: " + toUTF8(path));
      }

      controlThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SoundControlThread, this, 0, NULL);
      if (controlThread == NULL) {
        throw std::runtime_error("failed to create sound control thread: " + toUTF8(path));
      }
    } catch (...) {
      if (loopEndEvent) {
        CloseHandle(loopEndEvent);
      }
      if (controlThread) {
        TerminateThread(controlThread, 1);
        CloseHandle(controlThread);
      }
      safe_release(dSoundBuffer);
      throw;
    }
  }

  SoundBuffer::SoundBuffer(const std::shared_ptr<SoundBuffer>& src, IDirectSound8* dSound) :
    path(src->path),
    dSoundBuffer(NULL),
    loopEnable(false),
    loopStartSampleCnt(0),
    loopEndSampleCnt(DSBPN_OFFSETSTOP),
    controlThread(NULL),
    loopEndEvent(NULL)
  {
    try {
      IDirectSoundBuffer* dSoundTempBuffer = NULL;
      if (DS_OK != dSound->DuplicateSoundBuffer(src->dSoundBuffer, &dSoundTempBuffer)) {
        throw std::runtime_error("failed to duplicate sound buffer: " + toUTF8(path));
      }
      dSoundTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&(this->dSoundBuffer));
      // バグがあるので、バッファ複製直後に元の音量からちょっとずらす必要があるらしい
      LONG origVolume = NULL; // 元の音量
      src->dSoundBuffer->GetVolume(&origVolume);
      dSoundBuffer->SetVolume(origVolume - 100);
      // 最大音量に設定する
      dSoundBuffer->SetVolume(DSBVOLUME_MAX);
      dSoundBuffer->SetPan(DSBPAN_CENTER);
      safe_release(dSoundTempBuffer);

      WAVEFORMATEX waveFormat;
      if (FAILED(dSoundBuffer->GetFormat(&waveFormat, sizeof(waveFormat), NULL))) {
        throw std::runtime_error("failed to get sound buffer format: " + toUTF8(path));
      }
      samplePerSec = waveFormat.nSamplesPerSec;
      bytesPerSample = waveFormat.wBitsPerSample / 8;
      channelCnt = waveFormat.nChannels;

      loopEndEvent = CreateEvent(NULL, TRUE, FALSE, getLoopEndEventName().c_str());
      if (loopEndEvent == NULL) {
        throw std::runtime_error("failed to create loop end event: " + toUTF8(path));
      }

      controlThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SoundControlThread, this, 0, NULL);
      if (controlThread == NULL) {
        throw std::runtime_error("failed to create sound control thread: " + toUTF8(path));
      }
    } catch (...) {
      if (loopEndEvent) {
        CloseHandle(loopEndEvent);
      }
      if (controlThread) {
        TerminateThread(controlThread, 1);
        CloseHandle(controlThread);
      }
      safe_release(dSoundBuffer);
      throw;
    }
  }

  void SoundBuffer::play() {
    dSoundBuffer->Play(0, 0, loopEnable ? DSBPLAY_LOOPING : 0);
  }

  void SoundBuffer::stop() {
    dSoundBuffer->Stop();
  }

  void SoundBuffer::setVolume(float vol) {
    vol = constrain(vol, 0.0f, 1.0f);
    // NOTE : 1000だと値に比べて大きく聞こえるので3000にした。音量1/2で-9dBぐらいになる調整
    vol = vol == 0.0f ? DSBVOLUME_MIN : (3000.0f * log10f(vol));
    vol = constrain(vol, 1.0f*DSBVOLUME_MIN, 1.0f*DSBVOLUME_MAX);
    dSoundBuffer->SetVolume(vol);
  }

  void SoundBuffer::setPanRate(float pan) {
    pan = constrain(pan, -1.0f, 1.0f);
    const bool rightPan = pan > 0;
    pan = 1 - abs(pan); // 中央を1(倍)にする
    pan = pan == 0 ? DSBPAN_LEFT : (1000.0f * log10f(pan));
    if (rightPan) pan *= -1;
    pan = constrain(pan, 1.0f*DSBPAN_LEFT, 1.0f*DSBPAN_RIGHT);
    dSoundBuffer->SetPan(pan);
  }

  void SoundBuffer::seek(int sample) {
    dSoundBuffer->SetCurrentPosition(sample * bytesPerSample * channelCnt);
  }

  void SoundBuffer::setLoopEnable(bool enable) {
    loopEnable = enable;
  }

  void SoundBuffer::setLoopSampleCount(DWORD start, DWORD end) {
    loopStartSampleCnt = start;
    loopEndSampleCnt = end;
    IDirectSoundNotify8* soundNotify = NULL;
    dSoundBuffer->QueryInterface(IID_IDirectSoundNotify8, (void**)&soundNotify);
    DSBPOSITIONNOTIFY notifyPos;
    notifyPos.dwOffset = loopEndSampleCnt * bytesPerSample * channelCnt;
    notifyPos.hEventNotify = loopEndEvent;
    soundNotify->SetNotificationPositions(1, &notifyPos);
    safe_release(soundNotify);
  }

  void SoundBuffer::setLoopTime(double startSec, double endSec) {
    setLoopSampleCount((DWORD)(samplePerSec * startSec), (DWORD)(samplePerSec * endSec));
  }

  bool SoundBuffer::isPlaying() {
    DWORD status;
    dSoundBuffer->GetStatus(&status);
    return (status & DSBSTATUS_PLAYING) != 0;
  }

  float SoundBuffer::getVolume() {
    LONG vol;
    dSoundBuffer->GetVolume(&vol);
    return vol == DSBVOLUME_MIN ? 0.0f : powf(10.0f, vol / 3000.0f);
  }

  const std::wstring & SoundBuffer::getPath() const {
    return path;
  }

  bool SoundBuffer::isLoopEnabled() const {
    return loopEnable;
  }

  std::wstring SoundBuffer::getLoopEndEventName() const {
    size_t id = (size_t)this;
    return L"LOOP_END_" + std::to_wstring(id);
  }

  DWORD SoundBuffer::getLoopStartSampleCount() const {
    return loopStartSampleCnt;
  }

  DWORD SoundBuffer::getLoopEndSampleCount() const {
    return loopEndSampleCnt;
  }

  SoundBuffer::~SoundBuffer() {
    TerminateThread(controlThread, 0);
    CloseHandle(controlThread);
    CloseHandle(loopEndEvent);
    safe_release(dSoundBuffer);
  }

  SoundDevice::SoundDevice(HWND hWnd) :
    hWnd(hWnd),
    dSound(NULL),
    loader(std::make_shared<SoundDataLoaderFromSoundFile>())
  {
    if (DS_OK != DirectSoundCreate8(NULL, &(this->dSound), NULL)) {
      throw std::runtime_error("failed to init sound device.");
    }
    dSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
  }

  SoundDevice::~SoundDevice() {
    safe_release(dSound);
  }

  std::shared_ptr<SoundBuffer> SoundDevice::loadSound(const std::wstring & path, bool doCache) {
    auto uniqPath = canonicalPath(path);
    auto it = cache.find(uniqPath);
    if (it != cache.end()) {
      return std::make_shared<SoundBuffer>(it->second, dSound);
    }
    IDirectSoundBuffer8* buf = loader->loadSoundData(path, dSound);
    if (buf == NULL) {
      throw std::runtime_error("failed to load sound data: " + toUTF8(path));
    }
    auto sound = std::make_shared<SoundBuffer>(uniqPath, buf);
    if (doCache) {
      cache[uniqPath] = sound;
    }
    return sound;
  }

  void SoundDevice::setLoader(const std::shared_ptr<SoundDataLoader>& ld) {
    loader = ld;
  }

  void SoundDevice::removeSoundCache(const std::wstring & path) {
    cache.erase(path);
  }

  void SoundDevice::clearSoundCache() {
    cache.clear();
  }

  static IDirectSoundBuffer8* createSoundBuffer(DWORD dataSize, WAVEFORMATEX* waveFormat, IDirectSound8* dSound) {
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

    IDirectSoundBuffer* dSoundTempBuffer = NULL;
    if (DS_OK != dSound->CreateSoundBuffer(&dSBufferDesc, &dSoundTempBuffer, NULL)) {
      return NULL;
    }
    IDirectSoundBuffer8* dSoundBuffer;
    dSoundTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&(dSoundBuffer));
    safe_release(dSoundTempBuffer);
    return dSoundBuffer;
  }

  IDirectSoundBuffer8* readWaveFile(const std::wstring & path, IDirectSound8* dSound) {
    IDirectSoundBuffer8* dSoundBuffer = NULL;
    HMMIO hMmio = NULL;

    MMIOINFO mmioInfo;
    memset(&mmioInfo, 0, sizeof(MMIOINFO));
    hMmio = mmioOpen((LPWSTR)path.c_str(), &mmioInfo, MMIO_READ);
    if (!hMmio) {
      throw std::runtime_error("can't open file: " + toUTF8(path));
    }

    auto illegal_wave_format = std::runtime_error("illegal wave format file: " + toUTF8(path));
    auto failed_to_load_wave = std::runtime_error("failed to load wave file: " + toUTF8(path));

    try {
      MMRESULT mmResult;
      MMCKINFO riffChunk;
      riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
      mmResult = mmioDescend(hMmio, &riffChunk, NULL, MMIO_FINDRIFF);
      if (mmResult != MMSYSERR_NOERROR) {
        throw illegal_wave_format;
      }

      MMCKINFO formatChunk;
      formatChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
      mmResult = mmioDescend(hMmio, &formatChunk, &riffChunk, MMIO_FINDCHUNK);
      if (mmResult != MMSYSERR_NOERROR) {
        throw illegal_wave_format;
      }

      WAVEFORMATEX waveFormat;
      DWORD fmSize = formatChunk.cksize;
      LONG size = mmioRead(hMmio, (HPSTR)&(waveFormat), fmSize);
      if (size != fmSize) {
        throw illegal_wave_format;
      }

      mmioAscend(hMmio, &formatChunk, 0);

      MMCKINFO dataChunk;
      dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
      mmResult = mmioDescend(hMmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK);
      if (mmResult != MMSYSERR_NOERROR) {
        throw illegal_wave_format;
      }

      DWORD totalDataSize = dataChunk.cksize;

      dSoundBuffer = createSoundBuffer(totalDataSize, &waveFormat, dSound);
      if (dSoundBuffer == NULL) {
        throw failed_to_load_wave;
      }

      char* writePoint = NULL;
      DWORD writeLength = 0;
      if (DS_OK != dSoundBuffer->Lock(0, 0, (LPVOID*)&writePoint, &writeLength, NULL, NULL, DSBLOCK_ENTIREBUFFER)) {
        throw failed_to_load_wave;
      }
      size = mmioRead(hMmio, (HPSTR)writePoint, totalDataSize);
      if (size != totalDataSize) {
        throw failed_to_load_wave;
      }
      dSoundBuffer->Unlock(writePoint, writeLength, NULL, 0);
    } catch (...) {
      safe_release(dSoundBuffer);
      mmioClose(hMmio, 0);
      throw;
    }
    mmioClose(hMmio, 0);
    return dSoundBuffer;
  }

  IDirectSoundBuffer8 * readOggVorbisFile(const std::wstring & path, IDirectSound8 * dSound) {
    auto cant_open_file = std::runtime_error("can't open ogg-vorbis file: " + toUTF8(path));
    auto illegal_vorbis_format = std::runtime_error("illegal ogg-vorbis format file: " + toUTF8(path));
    auto failed_to_load_vorbis = std::runtime_error("failed to load ogg-vorbis file: " + toUTF8(path));

    FILE* fp = _wfopen(path.c_str(), L"rb");
    if (fp == NULL) {
      throw cant_open_file;
    }
    OggVorbis_File vf;
    if (ov_open(fp, &vf, NULL, 0) != 0) {
      fclose(fp);
      throw cant_open_file;
    }

    IDirectSoundBuffer8* dSoundBuffer = NULL;

    try {
      vorbis_info *vi = ov_info(&vf, -1);
      if (vi == NULL) {
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
      if (dSoundBuffer == NULL) {
        throw failed_to_load_vorbis;
      }

      char* writePoint = NULL;
      DWORD writeLength = 0;
      if (DS_OK != dSoundBuffer->Lock(0, 0, (LPVOID*)&writePoint, &writeLength, NULL, NULL, DSBLOCK_ENTIREBUFFER)) {
        throw failed_to_load_vorbis;
      }

      int bs;
      char* writeEnd = writePoint + totalDataSize;
      while (true) {
        auto size = ov_read(&vf, writePoint, writeEnd - writePoint, 0, sizeof(WORD), 1, &bs);
        if (size < 0) {
          throw failed_to_load_vorbis;
        } else if (size == 0) {
          if (writePoint != writeEnd) {
            throw illegal_vorbis_format;
          }
          break;
        }
        writePoint += size;
      }
      dSoundBuffer->Unlock(writePoint, writeLength, NULL, 0);
    } catch (...) {
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
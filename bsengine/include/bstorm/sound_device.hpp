#pragma once

#include <windows.h>
#include <unordered_map>
#include <string>
#include <memory>

#include <bstorm/non_copyable.hpp>

struct IDirectSound8;
struct IDirectSoundBuffer8;

namespace bstorm {
  IDirectSoundBuffer8* readWaveFile(const std::wstring& path, IDirectSound8* dSound);
  IDirectSoundBuffer8* readOggVorbisFile(const std::wstring& path, IDirectSound8* dSound);

  class SoundDataLoader {
  public:
    virtual IDirectSoundBuffer8* loadSoundData(const std::wstring& path, IDirectSound8* dSound) = 0;
  };

  class SoundDataLoaderFromSoundFile : public SoundDataLoader {
  public:
    IDirectSoundBuffer8* loadSoundData(const std::wstring& path, IDirectSound8* dSound) override;
  };

  class SoundDevice;
  class SoundBuffer : private NonCopyable {
  public:
    SoundBuffer(const std::shared_ptr<SoundBuffer>& src, IDirectSound8* dSound);
    SoundBuffer(const std::wstring& path, IDirectSoundBuffer8* buf);
    ~SoundBuffer();
    void play();
    void stop();
    void setVolume(float vol); // 0 ~ 1
    void setPanRate(float pan); // - 1 ~ 1
    void seek(int sample);
    void setLoopEnable(bool enable);
    void setLoopSampleCount(DWORD start, DWORD end);
    void setLoopTime(double startSec, double endSec);
    bool isPlaying();
    float getVolume();
    const std::wstring& getPath() const;
    bool isLoopEnabled() const;
    std::wstring getLoopEndEventName() const;
    DWORD getLoopStartSampleCount() const;
    DWORD getLoopEndSampleCount() const;
  private:
    HANDLE loopEndEvent;
    DWORD loopStartSampleCnt;
    DWORD loopEndSampleCnt;
    DWORD samplePerSec;
    DWORD bytesPerSample;
    DWORD channelCnt;
    const std::wstring path;
    IDirectSoundBuffer8* dSoundBuffer;
    bool loopEnable;
    HANDLE controlThread;
  };

  struct SourcePos;
  class SoundDevice : private NonCopyable {
  public:
    SoundDevice(HWND hWnd);
    ~SoundDevice();
    std::shared_ptr<SoundBuffer> loadSound(const std::wstring& path, bool doCache, const std::shared_ptr<SourcePos>& srcPos);
    void setLoader(const std::shared_ptr<SoundDataLoader>& ld);
    void removeSoundCache(const std::wstring& path);
    void clearSoundCache();
  private:
    HWND hWnd;
    IDirectSound8* dSound;
    std::shared_ptr<SoundDataLoader> loader;
    std::unordered_map<std::wstring, std::shared_ptr<SoundBuffer>> cache;
  };
}
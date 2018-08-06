#pragma once

#include <bstorm/non_copyable.hpp>

#include <windows.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <dsound.h>

namespace bstorm
{
IDirectSoundBuffer8* ReadWaveFile(const std::wstring& path, IDirectSound8* dSound);
IDirectSoundBuffer8* ReadOggVorbisFile(const std::wstring& path, IDirectSound8* dSound);

class SoundDataLoader
{
public:
    virtual IDirectSoundBuffer8* LoadSoundData(const std::wstring& path, IDirectSound8* dSound) = 0;
};

class SoundDataLoaderFromSoundFile : public SoundDataLoader
{
public:
    IDirectSoundBuffer8 * LoadSoundData(const std::wstring& path, IDirectSound8* dSound) override;
};

class SoundDevice;
class SoundBuffer : private NonCopyable
{
public:
    SoundBuffer(const std::wstring& path, IDirectSoundBuffer8* buf);
    ~SoundBuffer();
    void Play();
    void Stop();
    void SetVolume(float vol); // 0 ~ 1
    void SetPanRate(float pan); // - 1 ~ 1
    void Seek(int sample);
    void SetLoopEnable(bool enable);
    void SetLoopSampleCount(DWORD start, DWORD end);
    void SetLoopTime(double startSec, double endSec);
    bool IsPlaying();
    float GetVolume();
    const std::wstring& GetPath() const;
    bool IsLoopEnabled() const;
    std::wstring GetLoopEndEventName() const;
    DWORD GetLoopStartSampleCount() const;
    DWORD GetLoopEndSampleCount() const;
private:
    HANDLE loopEndEvent_;
    DWORD loopStartSampleCnt_;
    DWORD loopEndSampleCnt_;
    DWORD samplePerSec_;
    DWORD bytesPerSample_;
    DWORD channelCnt_;
    const std::wstring path_;
    IDirectSoundBuffer8* dSoundBuffer_;
    bool loopEnable_;
    HANDLE controlThread_;
};

struct SourcePos;
class SoundDevice : private NonCopyable
{
public:
    SoundDevice(HWND hWnd);
    ~SoundDevice();
    std::shared_ptr<SoundBuffer> LoadSound(const std::wstring& path);
    void SetLoader(const std::shared_ptr<SoundDataLoader>& ld);
private:
    HWND hWnd_;
    IDirectSound8* dSound_;
    std::shared_ptr<SoundDataLoader> loader_;
};
}
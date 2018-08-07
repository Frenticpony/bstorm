#pragma once

#include <bstorm/non_copyable.hpp>

#include <windows.h>
#include <string>
#include <memory>
#include <dsound.h>

namespace bstorm
{
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
    DWORD GetLoopStartSampleCount() const;
    DWORD GetLoopEndSampleCount() const;
    struct SoundThreadParam
    {
        SoundBuffer* soundBuffer;
        HANDLE soundEvents[2];
    };
private:
    SoundThreadParam soundThreadParam_;
    HANDLE threadEndEvent_;
    HANDLE loopEndEvent_;
    HANDLE thread_;
    const std::wstring path_;
    IDirectSoundBuffer8* dSoundBuffer_;
    bool loopEnable_;
    DWORD loopStartSampleCnt_;
    DWORD loopEndSampleCnt_;
    DWORD samplePerSec_;
    DWORD bytesPerSample_;
    DWORD channelCnt_;
};
}

#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/com_deleter.hpp>

#include <string>
#include <memory>
#include <thread>
#include <dsound.h>
#include <concurrent_queue.h>

namespace bstorm
{
class WaveSampleStream;
class SoundDevice;

enum class SoundRequest
{
    PLAY,
    STOP,
    REWIND,
    SET_VOLUME,
    SET_PAN,
    EXIT
};

class SoundBuffer : private NonCopyable
{
public:
    SoundBuffer(std::unique_ptr<WaveSampleStream>&& stream, const std::shared_ptr<SoundDevice>& soundDevice);
    ~SoundBuffer();
    void Play();
    void Stop();
    void Rewind();
    void SetVolume(float vol); // 0 ~ 1
    void SetPanRate(float pan); // - 1 ~ 1
    void SetLoopEnable(bool enable);
    void SetLoopSampleCount(size_t start, size_t end);
    void SetLoopTime(double startSec, double endSec);
    const std::wstring& GetPath() const;
    bool IsPlaying() const;
    float GetVolume() const;
    float GetPan() const;
    bool IsLoopEnabled() const;
    size_t GetLoopStartSampleCount() const;
    size_t GetLoopEndSampleCount() const;
private:
    std::thread soundMain_;
    concurrency::concurrent_queue<SoundRequest> soundRequestQueue_;
    const std::wstring path_;
    float volume_;
    float pan_;
    const size_t sampleRate_;
    bool isPlaying_;
    bool loopEnable_;
    size_t loopStartSampleCnt_;
    size_t loopEndSampleCnt_;
};
}

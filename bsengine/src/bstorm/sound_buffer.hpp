#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/com_deleter.hpp>
#include <bstorm/wave_format.hpp>

#include <windows.h>
#include <string>
#include <memory>
#include <thread>
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#include <atomic>
#include <mutex>
#include <dsound.h>

struct IDirectSoundBuffer8;

namespace bstorm
{
class WaveSampleStream;
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
class SoundStreamBuffer
{
public:
	SoundStreamBuffer(std::unique_ptr<WaveSampleStream>&& stream, const std::shared_ptr<SoundDevice>& soundDevice);
    ~SoundStreamBuffer();
    void Play();
    void Stop();
    void Rewind();
    void SetVolume(float volume); // 0 ~ 1
    void SetPan(float pan); // -1 ~ 1
    void SetLoopEnable(bool enable);
    void SetLoopTime(size_t beginSec, size_t endSec);
    void SetLoopSampleCount(size_t begin, size_t end);
    void SetLoopRange(size_t begin, size_t end);
    bool IsPlaying() const;
    float GetVolume() const;
    size_t GetCurrentPlayCursor() const;
private:
    void SoundMain();
    enum class BufferSector
    {
        FirstHalf,
        LatterHalf
    };
    mutable std::recursive_mutex criticalSection_;
    std::unique_ptr<WaveSampleStream> istream_;
    const WaveFormat waveFormat_;
    const size_t bufSize_;
    std::unique_ptr<IDirectSoundBuffer8, com_deleter> dsBuffer_;
    std::atomic<bool> isThreadTerminated_;
    std::atomic<bool> isLoopEnabled_;
    struct LoopRange
    {
        LoopRange() : begin(0), end(0) {}
        LoopRange(size_t begin, size_t end) : begin(begin), end(end) {}
        size_t begin;
        size_t end;
        // [start, end)
    };
    std::atomic<LoopRange> loopRange_;
    std::thread soundMain_;

    // only thread internal
    void FillBufferSector(BufferSector sector);
    size_t GetHalfBufferSize() const { return bufSize_ / 2; }
    BufferSector writeSector_;
    bool isSetStopCursor_;
    size_t stopCursor_;
    DWORD playCursorOnStop_;
};
}
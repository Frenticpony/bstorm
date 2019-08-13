#pragma once

#include <bstorm/non_copyable.hpp>

#include <windows.h>
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

class SoundBuffer;
class SoundStreamBuffer;
class SoundDevice : private NonCopyable, public std::enable_shared_from_this<SoundDevice>
{
public:
    SoundDevice(HWND hWnd);
    ~SoundDevice();
    std::shared_ptr<SoundBuffer> LoadSound(const std::wstring& path);
    std::shared_ptr<SoundStreamBuffer> LoadSoundStream(const std::wstring& path);
    IDirectSound8* GetDevice() const { return dSound_; }
	void SetLoader(const std::shared_ptr<SoundDataLoader>& ld);
private:
    HWND hWnd_;
    IDirectSound8* dSound_;
	std::shared_ptr<SoundDataLoader> loader_;
};
}
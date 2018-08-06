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
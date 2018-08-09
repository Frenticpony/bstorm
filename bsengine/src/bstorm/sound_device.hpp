#pragma once

#include <bstorm/non_copyable.hpp>

#include <windows.h>
#include <string>
#include <memory>
#include <dsound.h>

namespace bstorm
{
class SoundBuffer;
class SoundDevice : private NonCopyable, public std::enable_shared_from_this<SoundDevice>
{
public:
    SoundDevice(HWND hWnd);
    ~SoundDevice();
    std::shared_ptr<SoundBuffer> LoadSound(const std::wstring& path);
    IDirectSound8* GetDevice() const { return dSound_; }
private:
    HWND hWnd_;
    IDirectSound8* dSound_;
};
}
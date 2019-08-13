﻿#pragma once

#include <bstorm/obj.hpp>

#include <windows.h>

namespace bstorm
{
class SoundBuffer;
class SoundStreamBuffer;

class ObjSound : public Obj
{
public:
    enum class SoundDivision
    {
        BGM,
        SE
    };
    ObjSound(const std::shared_ptr<Package>& package);
    ~ObjSound();
    void Update() override;
    void SetSound(const std::shared_ptr<SoundBuffer>& buf);
    void SetSoundStream(const std::shared_ptr<SoundStreamBuffer>& buf);
    void Play();
    void Stop();
    void SetVolumeRate(float vol);
    void SetPanRate(float pan);
    void SetFade(float fadeRatePerSec);
    void SetLoopEnable(bool enable);
    void SetLoopTime(double startSec, double endSec);
    void SetLoopSampleCount(DWORD startCount, DWORD endCount);
    void SetRestartEnable(bool enable);
    void SetSoundDivision(SoundDivision division);
    bool IsPlaying() const;
	bool IsStream;
    float GetVolumeRate() const;
private:
    NullableSharedPtr<SoundBuffer> soundBuffer_;
    NullableSharedPtr<SoundStreamBuffer> soundStreamBuffer_;
    bool restartEnable_;
    float fadeRatePerSec_;
    SoundDivision division_;
};
}
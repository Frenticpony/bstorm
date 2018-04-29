#pragma once

#include <bstorm/obj.hpp>

#include <windows.h>

namespace bstorm
{
class SoundDevice;
class SoundBuffer;
class ObjSound : public Obj
{
public:
    enum class SoundDivision
    {
        BGM,
        SE
    };
    ObjSound(const std::shared_ptr<GameState>& gameState);
    ~ObjSound();
    void Update() override;
    void SetSound(const std::shared_ptr<SoundBuffer>& buf);
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
    float GetVolumeRate() const;
private:
    std::shared_ptr<SoundBuffer> soundBuffer_;
    bool restartEnable_;
    float fadeRatePerSec_;
    SoundDivision division_;
};
}
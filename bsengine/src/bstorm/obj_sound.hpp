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
    void update() override;
    void setSound(const std::shared_ptr<SoundBuffer>& buf);
    void play();
    void stop();
    void setVolumeRate(float vol);
    void setPanRate(float pan);
    void setFade(float fadeRatePerSec);
    void setLoopEnable(bool enable);
    void setLoopTime(double startSec, double endSec);
    void setLoopSampleCount(DWORD startCount, DWORD endCount);
    void setRestartEnable(bool enable);
    void setSoundDivision(SoundDivision division);
    bool isPlaying() const;
    float getVolumeRate() const;
private:
    std::shared_ptr<SoundBuffer> soundBuffer;
    bool restartEnable;
    float fadeRatePerSec;
    SoundDivision division;
};
}
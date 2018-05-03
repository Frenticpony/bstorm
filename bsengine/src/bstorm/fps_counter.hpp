#pragma once

#include <array>
#include <windows.h>

namespace bstorm
{
class TimePoint
{
public:
    TimePoint();
    ~TimePoint();
    float GetElapsedMilliSec(const TimePoint& tp = TimePoint()) const;
    float GetTimeMilliSec() const;
private:
    bool isHighAccuracyMode_;
    INT64 timeMicro_; // 記録時点
    INT64 freq_;
    // 高精度モードが使えない時用の記録時点
    DWORD timeMilli_;
};

class FpsCounter
{
public:
    FpsCounter();
    ~FpsCounter();
    void Update();
    float Get() const;
    float GetStable() const;
private:
    static constexpr int SampleCnt = 64; // power of 2
    TimePoint prevFrameTime_;
    float milliSecPerFrameAccum_;
    int milliSecPerFrameIdx_;
    float fps_;
    float stableFps_; // 表示用(nフレームに1回更新するので表示したときにチラつかない)
    std::array<float, SampleCnt> milliSecPerFrameList_;
};
}
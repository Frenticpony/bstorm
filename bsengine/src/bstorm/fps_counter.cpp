#include <bstorm/fps_counter.hpp>

#include <windows.h>

static DWORD getTimeMilliSec()
{
    timeBeginPeriod(1);
    DWORD r = timeGetTime();
    timeEndPeriod(1);
    return r;
}

namespace bstorm
{
TimePoint::TimePoint() :
    isHighAccuracyMode(QueryPerformanceCounter((LARGE_INTEGER*)&time))
{
    if (isHighAccuracyMode)
    {
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    }
    timeMilli = getTimeMilliSec();
}

TimePoint::~TimePoint() {}

float TimePoint::getElapsedMilliSec(const TimePoint& tp) const
{
    if (isHighAccuracyMode && tp.isHighAccuracyMode)
    {
        return 1000.0f * (tp.time - time) / freq;
    }
    return (float)(tp.timeMilli - timeMilli);
}

FpsCounter::FpsCounter() :
    prevFrameTime(),
    milliSecPerFrameAccum(0.0f),
    milliSecPerFrameIdx(0),
    stableFps(60.0f)
{
    milliSecPerFrameList.fill(0.0f);
}

FpsCounter::~FpsCounter()
{
}

void FpsCounter::update()
{
    TimePoint now;
    float deltaTime = prevFrameTime.getElapsedMilliSec(now);
    float removedTime = milliSecPerFrameList[milliSecPerFrameIdx];
    milliSecPerFrameAccum += deltaTime - removedTime;
    milliSecPerFrameList[milliSecPerFrameIdx] = deltaTime;
    milliSecPerFrameIdx = (milliSecPerFrameIdx + 1) & (SampleCnt - 1);
    prevFrameTime = now;
    fps = 1000.0f / (milliSecPerFrameAccum / SampleCnt);
    if (milliSecPerFrameIdx == 0) { stableFps = fps; }
}

float FpsCounter::get() const
{
    return fps;
}

float FpsCounter::getStable() const
{
    return stableFps;
}
}
#include <bstorm/fps_counter.hpp>

#include <windows.h>

static DWORD GetTimeMilliSec()
{
    timeBeginPeriod(1);
    DWORD r = timeGetTime();
    timeEndPeriod(1);
    return r;
}

namespace bstorm
{
TimePoint::TimePoint() :
    isHighAccuracyMode_(QueryPerformanceCounter((LARGE_INTEGER*)&time_))
{
    if (isHighAccuracyMode_)
    {
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq_);
    }
    timeMilli_ = GetTimeMilliSec();
}

TimePoint::~TimePoint() {}

float TimePoint::GetElapsedMilliSec(const TimePoint& tp) const
{
    if (isHighAccuracyMode_ && tp.isHighAccuracyMode_)
    {
        return 1000.0f * (tp.time_ - time_) / freq_;
    }
    return (float)(tp.timeMilli_ - timeMilli_);
}

FpsCounter::FpsCounter() :
    prevFrameTime_(),
    milliSecPerFrameAccum_(0.0f),
    milliSecPerFrameIdx_(0),
    stableFps_(60.0f)
{
    milliSecPerFrameList_.fill(0.0f);
}

FpsCounter::~FpsCounter()
{
}

void FpsCounter::Update()
{
    TimePoint now;
    float deltaTime = prevFrameTime_.GetElapsedMilliSec(now);
    float removedTime = milliSecPerFrameList_[milliSecPerFrameIdx_];
    milliSecPerFrameAccum_ += deltaTime - removedTime;
    milliSecPerFrameList_[milliSecPerFrameIdx_] = deltaTime;
    milliSecPerFrameIdx_ = (milliSecPerFrameIdx_ + 1) & (SampleCnt - 1);
    prevFrameTime_ = now;
    fps_ = 1000.0f / (milliSecPerFrameAccum_ / SampleCnt);
    if (milliSecPerFrameIdx_ == 0) { stableFps_ = fps_; }
}

float FpsCounter::Get() const
{
    return fps_;
}

float FpsCounter::GetStable() const
{
    return stableFps_;
}
}
#include <bstorm/fps_counter.hpp>

#include <bstorm/time_point.hpp>

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
FpsCounter::FpsCounter() :
    prevFrameTimePoint_(std::make_shared<TimePoint>()),
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
    std::shared_ptr<TimePoint> now = std::make_shared<TimePoint>();
    float deltaTime = prevFrameTimePoint_->GetElapsedMilliSec(*now);
    float removedTime = milliSecPerFrameList_[milliSecPerFrameIdx_];
    milliSecPerFrameAccum_ += deltaTime - removedTime;
    milliSecPerFrameList_[milliSecPerFrameIdx_] = deltaTime;
    milliSecPerFrameIdx_ = (milliSecPerFrameIdx_ + 1) & (SampleCnt - 1);
    prevFrameTimePoint_ = now;
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
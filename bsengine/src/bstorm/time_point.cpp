#include <bstorm/time_point.hpp>

namespace bstorm
{
TimePoint::TimePoint() :
    isHighAccuracyMode_(QueryPerformanceCounter((LARGE_INTEGER*)&timeMicro_))
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
        return 1000.0f * (tp.timeMicro_ - timeMicro_) / freq_;
    }
    return (float)(tp.timeMilli_ - timeMilli_);
}

float TimePoint::GetTimeMilliSec() const
{
    return isHighAccuracyMode_ ? (timeMicro_ * 1000.0f) : timeMilli_;
}
}
#pragma once

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
}
#pragma once

#include <deque>
#include <windows.h>

namespace bstorm {
  class TimePoint {
  public:
    TimePoint();
    ~TimePoint();
    double getElapsedMilliSec(const TimePoint& tp = TimePoint()) const;
  private:
    bool isHighAccuracyMode;
    LARGE_INTEGER from; // 記録時点
    LARGE_INTEGER freq;
    // 高精度モードが使えない時用
    DWORD fromMilliSec;
  };

  class FpsCounter {
  public:
    FpsCounter(int sampleCount);
    ~FpsCounter();
    void update();
    double get() const;
  private:
    TimePoint prevFrameTime;
    double milliSecPerFrameAverage;
    std::deque<double> milliSecPerFrameList;
  };
}

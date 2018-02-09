#pragma once

#include <array>
#include <windows.h>

namespace bstorm {
  class TimePoint {
  public:
    TimePoint();
    ~TimePoint();
    float getElapsedMilliSec(const TimePoint& tp = TimePoint()) const;
  private:
    bool isHighAccuracyMode;
    INT64 time; // 記録時点
    INT64 freq;
    // 高精度モードが使えない時用
    DWORD timeMilli;
  };

  class FpsCounter {
  public:
    FpsCounter();
    ~FpsCounter();
    void update();
    float get() const;
  private:
    static constexpr int sampleCnt = 16; // 2のべき乗
    TimePoint prevFrameTime;
    float milliSecPerFrameAccum;
    int milliSecPerFrameIdx;
    float fps;
    std::array<float, sampleCnt> milliSecPerFrameList;
  };
}
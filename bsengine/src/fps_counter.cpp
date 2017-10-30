#include <Windows.h>
#include <bstorm/fps_counter.hpp>

static DWORD getTimeMilliSec() {
  timeBeginPeriod(1);
  DWORD r = timeGetTime();
  timeEndPeriod(1);
  return r;
}

namespace bstorm {
  TimePoint::TimePoint() :
    isHighAccuracyMode(QueryPerformanceCounter(&from))
  {
    if (isHighAccuracyMode) {
      QueryPerformanceFrequency(&freq);
    }
    fromMilliSec = getTimeMilliSec();
  }

  TimePoint::~TimePoint() { }

  double TimePoint::getElapsedMilliSec(const TimePoint& tp) const {
    if (isHighAccuracyMode && tp.isHighAccuracyMode) {
      return 1000.0 * (tp.from.QuadPart - from.QuadPart) / freq.QuadPart;
    }
    return (double)(tp.fromMilliSec - fromMilliSec);
  }

  FpsCounter::FpsCounter(int sampleCount) :
    prevFrameTime(),
    milliSecPerFrameAverage(1000.0 / 60.0),
    milliSecPerFrameList(sampleCount, 1000.0 / 60)
  {
  }

  FpsCounter::~FpsCounter() {
  }

  void FpsCounter::update() {
    TimePoint now;
    double pushedTime = prevFrameTime.getElapsedMilliSec(now);
    double popedTime = milliSecPerFrameList.front();
    prevFrameTime = now;
    milliSecPerFrameList.pop_front();
    milliSecPerFrameList.push_back(pushedTime);
    milliSecPerFrameAverage += (pushedTime - popedTime) / milliSecPerFrameList.size();
  }

  double FpsCounter::get() const {
    return 1000.0 / milliSecPerFrameAverage;
  }
}
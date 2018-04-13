#pragma once

#include <bstorm/type.hpp>

#include <memory>

namespace bstorm {
  class AutoDeleteClip {
  public:
    AutoDeleteClip(const std::shared_ptr<Rect<float>>& stgFrame, float l, float t, float r, float b) :
      stgFrame(stgFrame)
    {
      setClip(l, t, r, b);
    }
    bool outOfClip(float x, float y) const {
      float stgFrameWidth = (stgFrame->right - stgFrame->left);
      float stgFrameHeight = (stgFrame->bottom - stgFrame->top);
      return x <= -left || y <= -top || x >= right + stgFrameWidth || y >= bottom + stgFrameHeight;
    }
    void setClip(float l, float t, float r, float b) { left = l; top = t; right = r; bottom = b; }
  private:
    std::shared_ptr<Rect<float>> stgFrame;
    float left;
    float top;
    float right;
    float bottom;
  };
}

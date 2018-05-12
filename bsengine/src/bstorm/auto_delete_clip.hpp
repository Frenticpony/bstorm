#pragma once

#include <memory>

#include <bstorm/rect.hpp>

namespace bstorm
{
class AutoDeleteClip
{
public:
    AutoDeleteClip(const std::shared_ptr<Rect<float>>& stgFrame, float l, float t, float r, float b) :
        stgFrame_(stgFrame)
    {
        SetClip(l, t, r, b);
    }
    bool IsOutOfClip(float x, float y) const
    {
        float stgFrameWidth = (stgFrame_->right - stgFrame_->left);
        float stgFrameHeight = (stgFrame_->bottom - stgFrame_->top);
        return x <= -left_ || y <= -top_ || x >= right_ + stgFrameWidth || y >= bottom_ + stgFrameHeight;
    }
    void SetClip(float l, float t, float r, float b) { left_ = l; top_ = t; right_ = r; bottom_ = b; }
private:
    std::shared_ptr<Rect<float>> stgFrame_;
    float left_;
    float top_;
    float right_;
    float bottom_;
};
}

#pragma once

#include <bstorm/rect.hpp>

#include <vector>

namespace bstorm
{
class AnimationClip
{
public:
    AnimationClip(int f, int l, int t, int r, int b) : frame(f), rect(l, t, r, b) {}
    FrameCount frame;
    Rect<int> rect;
};
using AnimationData = std::vector<AnimationClip>;
}

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <d3d9.h>

namespace bstorm
{
template <typename T>
struct Rect
{
    Rect(T l, T t, T r, T b) : left(l), top(t), right(r), bottom(b) {}
    T left, top, right, bottom;
};

struct AnimationClip
{
    AnimationClip(int f, int l, int t, int r, int b) : frame(f), rect(l, t, r, b) {}
    int frame;
    Rect<int> rect;
};
using AnimationData = std::vector<AnimationClip>;

using StageIndex = int;
using RandSeed = uint32_t;
using FrameCount = int64_t;
}
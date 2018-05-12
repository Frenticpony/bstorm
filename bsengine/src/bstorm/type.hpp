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

class ColorRGB
{
public:
    ColorRGB() : r_(0xff), g_(0xff), b_(0xff) {}
    ColorRGB(int r, int g, int b) :
        r_(std::min(::std::max(r, 0), 0xff)),
        g_(std::min(::std::max(g, 0), 0xff)),
        b_(std::min(::std::max(b, 0), 0xff))
    {
    }
    bool operator==(const ColorRGB& other) const
    {
        return r_ == other.r_ && g_ == other.g_ && b_ == other.b_;
    }
    bool operator!=(const ColorRGB& other) const
    {
        return !(*this == other);
    }
    int GetR() const { return r_; }
    int GetG() const { return g_; }
    int GetB() const { return b_; }
private:
    int r_;
    int g_;
    int b_;
};

struct AnimationClip
{
    AnimationClip(int f, int l, int t, int r, int b) : frame(f), rect(l, t, r, b) {}
    int frame;
    Rect<int> rect;
};
using AnimationData = std::vector<AnimationClip>;

struct Vertex
{
    Vertex() : x(0.0f), y(0.0f), z(0.0f), color(D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff)), u(0.0f), v(0.0f) {}
    Vertex(float x, float y, float z, D3DCOLOR color, float u, float v) : x(x), y(y), z(z), color(color), u(u), v(v) {}
    float x, y, z;
    D3DCOLOR color;
    float u, v;
    static constexpr DWORD Format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
};

struct Point2D
{
    Point2D() : x(0.0f), y(0.0f) {}
    Point2D(float x, float y) : x(x), y(y) {}
    float x;
    float y;
};

using StageIndex = int;
using RandSeed = uint32_t;
using FrameCount = int64_t;
}
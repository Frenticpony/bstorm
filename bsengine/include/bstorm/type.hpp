#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <d3d9.h>

namespace bstorm {
  template <typename T>
  struct Rect {
    Rect(T l, T t, T r, T b) : left(l), top(t), right(r), bottom(b) {}
    T left, top, right, bottom;
  };

  class ColorRGB {
  public:
    ColorRGB() : r(0xff), g(0xff), b(0xff) {}
    ColorRGB(int r, int g, int b) :
      r(std::min(::std::max(r, 0), 0xff)),
      g(std::min(::std::max(g, 0), 0xff)),
      b(std::min(::std::max(b, 0), 0xff)) {}
    bool operator==(const ColorRGB& other) const {
      return r == other.r && g == other.g && b == other.b;
    }
    bool operator!=(const ColorRGB& other) const {
      return !(*this == other);
    }
    int getR() const { return r; }
    int getG() const { return g; }
    int getB() const { return b; }
  private:
    int r;
    int g;
    int b;
  };

  struct AnimationClip {
    AnimationClip(int f, int l, int t, int r, int b) : frame(f), rect(l, t, r, b) {}
    int frame;
    Rect<int> rect;
  };
  typedef std::vector<AnimationClip> AnimationData;

  struct Vertex {
    Vertex() : x(0), y(0), z(0), color(D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff)), u(0), v(0) {}
    Vertex(float x, float y, float z, D3DCOLOR color, float u, float v) : x(x), y(y), z(z), color(color), u(u), v(v) {}
    float x, y, z;
    D3DCOLOR color;
    float u, v;
    static constexpr DWORD Format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
  };

  struct Point2D {
    Point2D() : x(0.0f), y(0.0f) {}
    Point2D(float x, float y) : x(x), y(y) {}
    float x;
    float y;
  };

  typedef int KeyState;
  typedef int VirtualKey;
  typedef int Key;
  typedef int PadButton;
  typedef int MouseButton;
}
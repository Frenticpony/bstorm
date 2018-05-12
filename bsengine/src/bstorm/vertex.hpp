#pragma once

#include <d3d9.h>
#include <array>

#include <bstorm/rect.hpp>

namespace bstorm
{
struct Vertex
{
    Vertex() : x(0.0f), y(0.0f), z(0.0f), color(D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff)), u(0.0f), v(0.0f) {}
    Vertex(float x, float y, float z, D3DCOLOR color, float u, float v) : x(x), y(y), z(z), color(color), u(u), v(v) {}
    float x, y, z;
    D3DCOLOR color;
    float u, v;
    static constexpr DWORD Format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
};

std::array<Vertex, 4> GetRectVertices(D3DCOLOR color, int textureWidth, int textureHeight, const Rect<int> &rect);
}

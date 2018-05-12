#include <bstorm/vertex.hpp>

namespace bstorm
{
std::array<Vertex, 4> GetRectVertices(D3DCOLOR color, int textureWidth, int textureHeight, const Rect<int>& rect)
{
    std::array<Vertex, 4> vs;
    float ul = 1.0 * rect.left / textureWidth;
    float vt = 1.0 * rect.top / textureHeight;
    float ur = 1.0 * rect.right / textureWidth;
    float vb = 1.0 * rect.bottom / textureHeight;
    float hw = (rect.right - rect.left) / 2.0f;
    float hh = (rect.bottom - rect.top) / 2.0f;
    vs[0] = { -hw, -hh, 0, color, ul, vt };
    vs[1] = { hw, -hh, 0, color, ur, vt };
    vs[2] = { -hw, hh, 0, color, ul, vb };
    vs[3] = { hw, hh, 0, color, ur, vb };
    return vs;
}
}

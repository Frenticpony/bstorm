#include <bstorm/package.hpp>
#include <bstorm/engine_develop_options.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/obj_col.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/dx_util.hpp>
#include <bstorm/renderer.hpp>

#include <d3dx9.h>

namespace bstorm
{
void ObjCol::RenderIntersection(const std::shared_ptr<Renderer>& renderer, bool isPermitCamera) const
{
    auto package = package_.lock();
    bool renderIntersectionEnable = package && package->GetEngineDevelopOptions()->renderIntersectionEnable;

    if (renderIntersectionEnable)
    {
        for (auto& isect : GetIntersections())
        {
            isect->Render(renderer, isPermitCamera);
        }
        for (auto& isect : GetTempIntersections())
        {
            isect->Render(renderer, isPermitCamera);
        }
    }
}

bool ObjPlayer::IsForceInvincible() const
{
    if (auto package = GetPackage().lock())
    {
        return package->GetEngineDevelopOptions()->forcePlayerInvincibleEnable;
    }
    return false;
}

static std::array<Point2D, 4> LineToRect(float x1, float y1, float x2, float y2, float width)
{
    const float halfWidth = width / 2.0f;
    const float normalDir = atan2(y2 - y1, x2 - x1) + D3DX_PI / 2.0f;
    const float dx = halfWidth * cos(normalDir);
    const float dy = halfWidth * sin(normalDir);
    return std::array<Point2D, 4>{Point2D(x1 + dx, y1 + dy),
        Point2D(x1 - dx, y1 - dy),
        Point2D(x2 - dx, y2 - dy),
        Point2D(x2 + dx, y2 + dy)};
}

void Shape::Render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const
{
    const D3DCOLOR color = D3DCOLOR_ARGB(128, 255, 0, 0);
    if (type_ == Type::CIRCLE)
    {
        static constexpr int vertexNum = 66;
        static constexpr int way = vertexNum - 2;
        static bool isInitialized = false;
        static std::array<Vertex, vertexNum> vertices;
        if (!isInitialized)
        {
            float x = 0;
            float y = 1;
            float c = cos(2 * D3DX_PI / way);
            float s = sin(2 * D3DX_PI / way);
            for (int i = 0; i < vertexNum; i++)
            {
                Vertex& v = vertices[i];
                if (i == 0)
                {
                    v.x = 0;
                    v.y = 0;
                } else
                {
                    v.x = x;
                    v.y = y;
                    float nx = c * x - s * y;
                    float ny = s * x + c * y;
                    x = nx;
                    y = ny;
                }
                v.color = color;
            }
            isInitialized = true;
        }
        D3DXMATRIX world = CreateScaleRotTransMatrix(params_.Circle.x, params_.Circle.y, 0.0f, 0.0f, 0.0f, 0.0f, params_.Circle.r, params_.Circle.r, 1.0f);
        renderer->RenderPrim2D(D3DPT_TRIANGLEFAN, vertices.size(), vertices.data(), nullptr, BLEND_ALPHA, world, std::shared_ptr<Shader>(), permitCamera, false);
    } else if (type_ == Type::RECT)
    {
        static std::array<Vertex, 4> vertices;
        for (auto& v : vertices)
        {
            v.color = color;
        }
        const auto rect = LineToRect(params_.Rect.x1, params_.Rect.y1, params_.Rect.x2, params_.Rect.y2, params_.Rect.width);
        vertices[0].x = rect[0].x;
        vertices[0].y = rect[0].y;
        vertices[1].x = rect[1].x;
        vertices[1].y = rect[1].y;
        vertices[2].x = rect[3].x;
        vertices[2].y = rect[3].y;
        vertices[3].x = rect[2].x;
        vertices[3].y = rect[2].y;
        D3DXMATRIX world;
        D3DXMatrixIdentity(&world);
        renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, vertices.size(), vertices.data(), nullptr, BLEND_ALPHA, world, std::shared_ptr<Shader>(), permitCamera, false);
    }
}
}
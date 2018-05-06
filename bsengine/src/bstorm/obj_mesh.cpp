#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/obj_mesh.hpp>

#include <d3d9.h>

namespace bstorm
{
ObjMesh::ObjMesh(const std::shared_ptr<GameState>& gameState) :
    ObjRender(gameState)
{
    SetType(OBJ_MESH);
}

ObjMesh::~ObjMesh()
{
}

void ObjMesh::Update() {}

void ObjMesh::Render(const std::unique_ptr<Renderer>& renderer)
{
    if (mesh_)
    {
        D3DXMATRIX world = CreateScaleRotTransMatrix(GetX(), GetY(), GetZ(), GetAngleX(), GetAngleY(), GetAngleZ(), GetScaleX(), GetScaleY(), GetScaleZ());
        const auto& rgb = GetColor();
        D3DCOLORVALUE col = D3DCOLORVALUE{ rgb.GetR() / 255.0f, rgb.GetG() / 255.0f, rgb.GetB() / 255.0f, GetAlpha() / 255.0f };
        renderer->RenderMesh(mesh_, col, GetBlendType(), world, GetAppliedShader(), IsZWriteEnabled(), IsZTestEnabled(), IsFogEnabled());
    }
}

void ObjMesh::SetMesh(const std::shared_ptr<Mesh>& mesh)
{
    this->mesh_ = mesh;
}
}

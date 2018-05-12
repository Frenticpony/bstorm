#pragma once

#include <bstorm/type.hpp>
#include <bstorm/obj_render.hpp>

#include <memory>
#include <vector>
#include <d3d9.h>

struct IDirect3DTexture9;
namespace bstorm
{
class Texture;
class RenderTarget;
class ObjPrim : public ObjRender
{
public:
    ObjPrim(const std::shared_ptr<Package>& state);
    ~ObjPrim();
    void Update() override;
    int GetPrimitiveType() const;
    void SetPrimitiveType(int t);
    int GetVertexCount() const;
    void SetVertexCount(int n, bool doClear = true);
    const std::shared_ptr<Texture>& GetTexture() const;
    void SetTexture(const std::shared_ptr<Texture>& texture);
    const std::shared_ptr<RenderTarget>& GetRenderTarget() const;
    void SetRenderTarget(const std::shared_ptr<RenderTarget>& target);
    float GetVertexPositionX(int vIdx) const;
    float GetVertexPositionY(int vIdx) const;
    float GetVertexPositionZ(int vIdx) const;
    void SetVertexPosition(int vIdx, float x, float y, float z);
    void SetVertexUV(int vIdx, float u, float v);
    void SetVertexUVT(int vIdx, float u, float v);
    void SetVertexColor(int vIdx, int r, int g, int b);
    void SetVertexAlpha(int vIdx, int a);
    const std::vector<Vertex>& GetVertices() const;
protected:
    IDirect3DTexture9 * GetD3DTexture() const;
    _D3DPRIMITIVETYPE GetD3DPrimitiveType() const;
    std::vector<Vertex> vertices_;
private:
    _D3DPRIMITIVETYPE primType_;
    std::shared_ptr<Texture> texture_;
    std::shared_ptr<RenderTarget> renderTarget_;
};

class ObjPrim2D : public ObjPrim
{
public:
    ObjPrim2D(const std::shared_ptr<Package>& state);
    void Render(const std::shared_ptr<Renderer>& renderer) override;
};

class ObjSprite2D : public ObjPrim2D
{
public:
    ObjSprite2D(const std::shared_ptr<Package>& state);
    void SetSourceRect(float left, float top, float right, float bottom);
    void SetDestRect(float left, float top, float right, float bottom);
    void SetDestCenter();
};

class ObjSpriteList2D : public ObjPrim2D
{
public:
    ObjSpriteList2D(const std::shared_ptr<Package>& state);
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    void SetSourceRect(float left, float top, float right, float bottom);
    void SetDestRect(float left, float top, float right, float bottom);
    void SetDestCenter();
    void AddVertex();
    void CloseVertex();
    void ClearVerexCount();
private:
    bool isVertexClosed_;
    float srcRectLeft_;
    float srcRectTop_;
    float srcRectRight_;
    float srcRectBottom_;
    float dstRectLeft_;
    float dstRectTop_;
    float dstRectRight_;
    float dstRectBottom_;
};

class ObjPrim3D : public ObjPrim
{
public:
    ObjPrim3D(const std::shared_ptr<Package>& state);
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    bool IsBillboardEnabled() const;
protected:
    bool billboardEnable_;
};

class ObjSprite3D : public ObjPrim3D
{
public:
    ObjSprite3D(const std::shared_ptr<Package>& state);
    void SetSourceRect(float left, float top, float right, float bottom);
    void SetDestRect(float left, float top, float right, float bottom);
    void SetSourceDestRect(float left, float top, float right, float bottom);
    void SetBillboard(bool enable);
};
}
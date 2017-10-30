#pragma once

#include <memory>
#include <vector>
#include <d3d9.h>

#include <bstorm/type.hpp>
#include <bstorm/obj_render.hpp>

struct IDirect3DTexture9;
namespace bstorm {
  class Texture;
  class RenderTarget;
  class ObjPrim : public ObjRender {
  public:
    ObjPrim(const std::shared_ptr<GameState>& state);
    ~ObjPrim();
    void update() override;
    int getPrimitiveType() const;
    void setPrimitiveType(int t);
    int getVertexCount() const;
    void setVertexCount(int n, bool doClear = true);
    const std::shared_ptr<Texture>& getTexture() const;
    void setTexture(const std::shared_ptr<Texture>& texture);
    const std::shared_ptr<RenderTarget>& getRenderTarget() const;
    void setRenderTarget(const std::shared_ptr<RenderTarget>& target);
    float getVertexPositionX(int vIdx) const;
    float getVertexPositionY(int vIdx) const;
    float getVertexPositionZ(int vIdx) const;
    void setVertexPosition(int vIdx, float x, float y, float z);
    void setVertexUV(int vIdx, float u, float v);
    void setVertexUVT(int vIdx, float u, float v);
    void setVertexColor(int vIdx, int r, int g, int b);
    void setVertexAlpha(int vIdx, int a);
    const std::vector<Vertex>& getVertices() const;
  protected:
    IDirect3DTexture9* getD3DTexture() const;
    _D3DPRIMITIVETYPE getD3DPrimitiveType() const;
    std::vector<Vertex> vertices;
  private:
    _D3DPRIMITIVETYPE primType;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<RenderTarget> renderTarget;
  };

  class ObjPrim2D : public ObjPrim {
  public:
    ObjPrim2D(const std::shared_ptr<GameState>& state);
    void render() override;
  };

  class ObjSprite2D : public ObjPrim2D {
  public:
    ObjSprite2D(const std::shared_ptr<GameState>& state);
    void setSourceRect(float left, float top, float right, float bottom);
    void setDestRect(float left, float top, float right, float bottom);
    void setDestCenter();
  };

  class ObjSpriteList2D : public ObjPrim2D {
  public:
    ObjSpriteList2D(const std::shared_ptr<GameState>& state);
    void render() override;
    void setSourceRect(float left, float top, float right, float bottom);
    void setDestRect(float left, float top, float right, float bottom);
    void setDestCenter();
    void addVertex();
    void closeVertex();
    void clearVerexCount();
  protected:
    bool isVertexClosed;
    float srcRectLeft;
    float srcRectTop;
    float srcRectRight;
    float srcRectBottom;
    float dstRectLeft;
    float dstRectTop;
    float dstRectRight;
    float dstRectBottom;
  };

  class ObjPrim3D : public ObjPrim {
  public:
    ObjPrim3D(const std::shared_ptr<GameState>& state);
    void render() override;
    bool isBillboardEnabled() const;
  protected:
    bool billboardEnable;
  };

  class ObjSprite3D : public ObjPrim3D {
  public:
    ObjSprite3D(const std::shared_ptr<GameState>& state);
    void setSourceRect(float left, float top, float right, float bottom);
    void setDestRect(float left, float top, float right, float bottom);
    void setSourceDestRect(float left, float top, float right, float bottom);
    void setBillboard(bool enable);
  };
}
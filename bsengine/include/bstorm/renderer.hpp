#pragma once

#include <d3dx9.h>
#include <array>
#include <memory>

#include <bstorm/const.hpp>

namespace bstorm {
  struct Vertex;
  class Shader;
  class Mesh;
  class Renderer {
  public:
    Renderer(IDirect3DDevice9*);
    ~Renderer();
    // NOTE : initRenderState : 描画デバイスの初期化を行う
    // デバイスロストやライブラリによってデバイスの状態が書き換えられた場合も呼ぶ必要がある
    // 一連の描画の前に1回呼べばよい
    void initRenderState();
    void renderPrim2D(D3DPRIMITIVETYPE primType, int vertexCount, const Vertex* vertices, IDirect3DTexture9* texture, int blendType, const D3DXMATRIX& worldMatrix, const std::shared_ptr<Shader>& pixelShader, bool permitCamera, bool insertHalfPixelOffset);
    void renderPrim3D(D3DPRIMITIVETYPE primType, int vertexCount, const Vertex* vertices, IDirect3DTexture9* texture, int blendType, const D3DXMATRIX& worldMatrix, const std::shared_ptr<Shader>& pixelShader, bool zWriteEnable, bool zTestEnable, bool useFog, bool billboardEnable);
    void renderMesh(const std::shared_ptr<Mesh>& mesh, const D3DCOLORVALUE& col, int blendType, const D3DXMATRIX& worldMatrix, const std::shared_ptr<Shader>& pixelShader, bool zWriteEnable, bool zTestEnable, bool useFog);
    void setViewProjMatrix2D(const D3DXMATRIX& view, const D3DXMATRIX& proj);
    void setForbidCameraViewProjMatrix2D(int screenWidth, int screenHeight);
    void setViewProjMatrix3D(const D3DXMATRIX& view, const D3DXMATRIX& proj);
    void setViewProjMatrix3D(const D3DXMATRIX& view, const D3DXMATRIX& proj, const D3DXMATRIX& billboardMatrix);
    void setBlendType(int type);
    // NOTE : enable/disableScissorTest : デバイスロストすると設定値は消える
    void enableScissorTest(const RECT& rect);
    void disableScissorTest();
    void setFogEnable(bool enable);
    void setFogParam(float fogStart, float fogEnd, int r, int g, int b);
  private:
    IDirect3DDevice9* d3DDevice;
    IDirect3DVertexShader9* prim2DVertexShader;
    IDirect3DVertexShader9* prim3DVertexShader;
    IDirect3DVertexShader9* meshVertexShader;
    float screenWidth;
    float screenHeight;
    int currentBlendType;
    D3DXMATRIX viewProjMatrix2D;
    D3DXMATRIX viewProjMatrix3D;
    D3DXMATRIX billboardViewProjMatrix3D;
    D3DXMATRIX forbidCameraViewProjMatrix2D;
    D3DXMATRIX halfPixelOffsetMatrix;
    bool fogEnable;
    float fogStart;
    float fogEnd;
    D3DCOLOR fogColor;
  };
}
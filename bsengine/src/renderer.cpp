#include <algorithm>

#include <bstorm/type.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/camera2D.hpp>
#include <bstorm/shader.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/mesh.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/renderer.hpp>

static const char prim2DVertexShaderSrc[] =
"float4x4 worldMatrix : register(c0);"
"float4x4 viewProjMatrix : register(c4);"
"struct VS_INPUT {"
"  float4 pos : POSITION;"
"  float4 diffuse : COLOR;"
"  float2 texCoord0 : TEXCOORD0;"
"};"
"struct VS_OUTPUT {"
"  float4 pos : POSITION;"
"  float4 diffuse : COLOR;"
"  float2 texCoord0 : TEXCOORD0;"
"};"
"VS_OUTPUT main(VS_INPUT In) {"
"  VS_OUTPUT Out;"
"  Out.pos = mul(mul(In.pos, worldMatrix), viewProjMatrix);"
"  Out.pos.z = 0;"
"  Out.diffuse = In.diffuse;"
"  Out.texCoord0 = In.texCoord0;"
"  return Out;"
"}";

static const char prim3DVertexShaderSrc[] =
"float4x4 worldViewProjMatrix : register(c0);"
"float fogStart : register(c12);"
"float fogEnd : register(c13);"
"struct VS_INPUT {"
"  float4 pos : POSITION;"
"  float4 diffuse : COLOR;"
"  float2 texCoord0 : TEXCOORD0;"
"};"
"struct VS_OUTPUT {"
"  float4 pos : POSITION;"
"  float4 diffuse : COLOR;"
"  float2 texCoord0 : TEXCOORD0;"
"  float fog : FOG;"
"};"
"VS_OUTPUT main(VS_INPUT In) {"
"  VS_OUTPUT Out;"
"  Out.pos = mul(In.pos, worldViewProjMatrix);"
"  Out.diffuse = In.diffuse;"
"  Out.texCoord0 = In.texCoord0;"
"  Out.fog = (fogEnd - Out.pos.w) / (fogEnd - fogStart);"
"  return Out;"
"}";

static const char meshVertexShaderSrc[] =
"float4x4 worldViewProjMatrix : register(c0);"
"float4x4 worldMatrix : register(c4);"
"float4 materialAmbient : register(c8);"
"float4 materialDiffuse : register(c9);"
"float4 lightDir : register(c10);"
"float fogStart : register(c12);"
"float fogEnd : register(c13);"
"struct VS_INPUT {"
"  float4 pos : POSITION;"
"  float3 normal : NORMAL;"
"  float2 texCoord0 : TEXCOORD0;"
"};"
"struct VS_OUTPUT {"
"  float4 pos : POSITION;"
"  float4 diffuse : COLOR;"
"  float2 texCoord0 : TEXCOORD0;"
"  float fog : FOG;"
"};"
"VS_OUTPUT main(VS_INPUT In) {"
"  VS_OUTPUT Out;"
"  Out.pos = mul(In.pos, worldViewProjMatrix);"
"  In.normal = normalize(mul(In.normal, worldMatrix));"
"  Out.diffuse.xyz = materialAmbient.xyz + max(0.0f, dot(In.normal, lightDir.xyz)) * materialDiffuse.xyz;"
"  Out.diffuse.w = materialAmbient.w * materialDiffuse.w;"
"  Out.texCoord0 = In.texCoord0;"
"  Out.fog = (fogEnd - Out.pos.w) / (fogEnd - fogStart);"
"  return Out;"
"}";

namespace bstorm {
  Renderer::Renderer(IDirect3DDevice9* dev) :
    d3DDevice(dev),
    prim2DVertexShader(NULL),
    prim3DVertexShader(NULL),
    meshVertexShader(NULL),
    currentBlendType(BLEND_NONE),
    fogEnable(false),
    fogStart(0),
    fogEnd(0)
  {
    ID3DXBuffer* code = NULL;
    ID3DXBuffer* error = NULL;
    try {
    // create vertex shader 2D
      if (FAILED(D3DXCompileShader(prim2DVertexShaderSrc, sizeof(prim2DVertexShaderSrc) - 1, NULL, NULL, "main", "vs_1_1", D3DXSHADER_PACKMATRIX_ROWMAJOR, &code, &error, NULL))) {
        throw Log(Log::Level::LV_ERROR).setMessage((const char*)error->GetBufferPointer());
      }
      d3DDevice->CreateVertexShader((const DWORD*)code->GetBufferPointer(), &prim2DVertexShader);
      safe_release(code);
      safe_release(error);

      // create vertex shader 3D
      if (FAILED(D3DXCompileShader(prim3DVertexShaderSrc, sizeof(prim3DVertexShaderSrc) - 1, NULL, NULL, "main", "vs_1_1", D3DXSHADER_PACKMATRIX_ROWMAJOR, &code, &error, NULL))) {
        throw Log(Log::Level::LV_ERROR).setMessage((const char*)error->GetBufferPointer());
      }
      d3DDevice->CreateVertexShader((const DWORD*)code->GetBufferPointer(), &prim3DVertexShader);
      safe_release(code);
      safe_release(error);

      // create vertex shader mesh
      if (FAILED(D3DXCompileShader(meshVertexShaderSrc, sizeof(meshVertexShaderSrc) - 1, NULL, NULL, "main", "vs_1_1", D3DXSHADER_PACKMATRIX_ROWMAJOR, &code, &error, NULL))) {
        throw Log(Log::Level::LV_ERROR).setMessage((const char*)error->GetBufferPointer());
      }
      d3DDevice->CreateVertexShader((const DWORD*)code->GetBufferPointer(), &meshVertexShader);
      safe_release(code);
      safe_release(error);
    } catch (...) {
      safe_release(code);
      safe_release(error);
      throw;
    }

    D3DXMatrixTranslation(&halfPixelOffsetMatrix, -0.5f, -0.5f, 0.0f);
  }

  Renderer::~Renderer() {
    prim2DVertexShader->Release();
    prim3DVertexShader->Release();
    meshVertexShader->Release();
  }

  void Renderer::initRenderState() {
    // カリング無効化
    d3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // 固定機能パイプラインのピクセルシェーダ用
    d3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    d3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    d3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    d3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    d3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    d3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // 透明度0を描画しないようにする
    // NOTE : ObjTextが乗算合成されたときに、背景部分が真っ黒にならないようにするため
    d3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    d3DDevice->SetRenderState(D3DRS_ALPHAREF, 0);
    d3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

    currentBlendType = BLEND_ALPHA;
    d3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    d3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    // light off
    d3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
  }

  static int calcPolygonNum(D3DPRIMITIVETYPE primType, int vertexCount) {
    if (primType == D3DPT_TRIANGLELIST) {
      return vertexCount / 3;
    } else {
      return vertexCount - 2;
    }
  }

  void Renderer::renderPrim2D(D3DPRIMITIVETYPE primType, int vertexCount, const Vertex* vertices, IDirect3DTexture9* texture, int blendType, const D3DXMATRIX & worldMatrix, const std::shared_ptr<Shader>& pixelShader, bool permitCamera, bool insertHalfPixelOffset) {
    // disable z-buffer-write, z-test, fog
    d3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    d3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    d3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    // set blend type
    setBlendType(blendType);
    // set vertex shader
    d3DDevice->SetVertexShader(prim2DVertexShader);
    // set shader constant
    d3DDevice->SetVertexShaderConstantF(0, (const float*)&(insertHalfPixelOffset ? (halfPixelOffsetMatrix * worldMatrix) : worldMatrix), 4);
    d3DDevice->SetVertexShaderConstantF(4, (const float*)&(permitCamera ? viewProjMatrix2D : forbidCameraViewProjMatrix2D), 4);
    // set vertex format
    d3DDevice->SetFVF(Vertex::Format);
    // set texture
    d3DDevice->SetTexture(0, texture);
  // set pixel shader
    if (pixelShader) {
      ID3DXEffect* effect = pixelShader->getEffect();
      UINT passCnt;
      effect->Begin(&passCnt, D3DXFX_DONOTSAVESTATE);
      for (int i = 0; i < passCnt; i++) {
        effect->BeginPass(i);
        d3DDevice->DrawPrimitiveUP(primType, calcPolygonNum(primType, vertexCount), vertices, sizeof(Vertex));
        effect->EndPass();
      }
      effect->End();
    } else {
      d3DDevice->SetPixelShader(NULL);
      d3DDevice->DrawPrimitiveUP(primType, calcPolygonNum(primType, vertexCount), vertices, sizeof(Vertex));
    }
    d3DDevice->SetVertexShader(NULL);
    d3DDevice->SetPixelShader(NULL);
  }

  void Renderer::renderPrim3D(D3DPRIMITIVETYPE primType, int vertexCount, const Vertex* vertices, IDirect3DTexture9* texture, int blendType, const D3DXMATRIX & worldMatrix, const std::shared_ptr<Shader>& pixelShader, bool zWriteEnable, bool zTestEnable, bool useFog, bool billboardEnable) {
    // set z-buffer-write, z-test, fog
    d3DDevice->SetRenderState(D3DRS_ZENABLE, zTestEnable ? TRUE : FALSE);
    d3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, zWriteEnable ? TRUE : FALSE);
    d3DDevice->SetRenderState(D3DRS_FOGENABLE, useFog && fogEnable ? TRUE : FALSE);
    d3DDevice->SetRenderState(D3DRS_FOGCOLOR, fogColor);
    // set blend type
    setBlendType(blendType);
    // set vertex shader
    d3DDevice->SetVertexShader(prim3DVertexShader);
    // set shader constant
    // 3Dオブジェクトは頂点数が多いので、予め行列を全て掛けておく
    const D3DXMATRIX worldViewProjMatrix = worldMatrix * (billboardEnable ? billboardViewProjMatrix3D : viewProjMatrix3D);
    d3DDevice->SetVertexShaderConstantF(0, (const float*)&worldViewProjMatrix, 4);
    d3DDevice->SetVertexShaderConstantF(12, &fogStart, 1);
    d3DDevice->SetVertexShaderConstantF(13, &fogEnd, 1);
    // set vertex format
    d3DDevice->SetFVF(Vertex::Format);
    // set texture
    d3DDevice->SetTexture(0, texture);
  // set pixel shader
    if (pixelShader) {
      ID3DXEffect* effect = pixelShader->getEffect();
      UINT passCnt;
      effect->Begin(&passCnt, D3DXFX_DONOTSAVESTATE);
      for (int i = 0; i < passCnt; i++) {
        effect->BeginPass(i);
        d3DDevice->DrawPrimitiveUP(primType, calcPolygonNum(primType, vertexCount), vertices, sizeof(Vertex));
        effect->EndPass();
      }
      effect->End();
    } else {
      d3DDevice->SetPixelShader(NULL);
      d3DDevice->DrawPrimitiveUP(primType, calcPolygonNum(primType, vertexCount), vertices, sizeof(Vertex));
    }
    d3DDevice->SetVertexShader(NULL);
    d3DDevice->SetPixelShader(NULL);
  }

  void Renderer::renderMesh(const std::shared_ptr<Mesh>& mesh, const D3DCOLORVALUE& col, int blendType, const D3DXMATRIX & worldMatrix, const std::shared_ptr<Shader>& pixelShader, bool zWriteEnable, bool zTestEnable, bool useFog) {
    // set z-buffer-write, z-test, fog
    d3DDevice->SetRenderState(D3DRS_ZENABLE, zTestEnable ? TRUE : FALSE);
    d3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, zWriteEnable ? TRUE : FALSE);
    d3DDevice->SetRenderState(D3DRS_FOGENABLE, useFog && fogEnable ? TRUE : FALSE);
    d3DDevice->SetRenderState(D3DRS_FOGCOLOR, fogColor);
    // set blend type
    setBlendType(blendType);
    // set vertex shader
    d3DDevice->SetVertexShader(meshVertexShader);
    // set shader constant
    const D3DXMATRIX worldViewProjMatrix = worldMatrix * viewProjMatrix3D;
    d3DDevice->SetVertexShaderConstantF(0, (const float*)&worldViewProjMatrix, 4);
    d3DDevice->SetVertexShaderConstantF(4, (const float*)&worldMatrix, 4);
    d3DDevice->SetVertexShaderConstantF(12, &fogStart, 1);
    d3DDevice->SetVertexShaderConstantF(13, &fogEnd, 1);
    // set vertex format
    d3DDevice->SetFVF(MeshVertex::Format);
    for (const auto& mat : mesh->materials) {
      // set material
      D3DXVECTOR4 amb = D3DXVECTOR4{ col.r * (mat.amb + mat.emi), col.g * (mat.amb + mat.emi), col.b * (mat.amb + mat.emi), col.a };
      D3DXVECTOR4 dif = D3DXVECTOR4{ col.r * mat.dif, col.g * mat.dif, col.b * mat.dif, col.a };
      d3DDevice->SetVertexShaderConstantF(8, (const float*)&amb, 1);
      d3DDevice->SetVertexShaderConstantF(9, (const float*)&dif, 1);
      // set light dir
      D3DXVECTOR4 lightDir = { -0.5, -0.4, -0.5, 0 };
      D3DXVec4Normalize(&lightDir, &lightDir);
      d3DDevice->SetVertexShaderConstantF(10, (const float*)&lightDir, 1);
      // set texture
      d3DDevice->SetTexture(0, mat.texture ? mat.texture->getTexture() : NULL);
      // set pixel shader
      if (pixelShader) {
        ID3DXEffect* effect = pixelShader->getEffect();
        UINT passCnt;
        effect->Begin(&passCnt, D3DXFX_DONOTSAVESTATE);
        for (int i = 0; i < passCnt; i++) {
          effect->BeginPass(i);
          d3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mat.vertices.size() / 3, mat.vertices.data(), sizeof(MeshVertex));
          effect->EndPass();
        }
        effect->End();
      } else {
        d3DDevice->SetPixelShader(NULL);
        d3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, mat.vertices.size() / 3, mat.vertices.data(), sizeof(MeshVertex));
      }
    }
    d3DDevice->SetVertexShader(NULL);
    d3DDevice->SetPixelShader(NULL);
  }

  void Renderer::setViewProjMatrix2D(const D3DXMATRIX& view, const D3DXMATRIX& proj) {
    viewProjMatrix2D = view * proj;
  }

  void Renderer::setForbidCameraViewProjMatrix2D(int screenWidth, int screenHeight) {
    Camera2D camera2D;
    camera2D.reset(0, 0);
    D3DXMATRIX forbidCameraViewMatrix2D;
    D3DXMATRIX forbidCameraProjMatrix2D;
    camera2D.generateViewMatrix(forbidCameraViewMatrix2D);
    camera2D.generateProjMatrix(screenWidth, screenHeight, 0, 0, forbidCameraProjMatrix2D);
    forbidCameraViewProjMatrix2D = forbidCameraViewMatrix2D * forbidCameraProjMatrix2D;
  }

  void Renderer::setViewProjMatrix3D(const D3DXMATRIX & view, const D3DXMATRIX & proj) {
    viewProjMatrix3D = view * proj;
  }

  void Renderer::setViewProjMatrix3D(const D3DXMATRIX & view, const D3DXMATRIX & proj, const D3DXMATRIX & billboardMatrix) {
    viewProjMatrix3D = view * proj;
    billboardViewProjMatrix3D = billboardMatrix * viewProjMatrix3D;
  }

  void Renderer::setBlendType(int type) {
    if (type == currentBlendType) return;
    switch (type) {
      case BLEND_ADD_RGB:
        d3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        d3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        d3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;
      case BLEND_ADD_ARGB:
        d3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        d3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;
      case BLEND_MULTIPLY:
        d3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        d3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        d3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        break;
      case BLEND_SUBTRACT:
        d3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
        d3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;
      case BLEND_INV_DESTRGB:
        d3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        d3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
        d3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
        break;
      case BLEND_ALPHA:
      default:
        d3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        d3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        break;
    }
    currentBlendType = type;
  }

  void Renderer::enableScissorTest(const RECT& rect) {
    d3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    d3DDevice->SetScissorRect(&rect);
  }

  void Renderer::disableScissorTest() {
    d3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
  }

  void Renderer::setFogEnable(bool enable) {
    fogEnable = enable;
    if (enable) { setFogParam(0, 0, 0, 0, 0); }
  }

  void Renderer::setFogParam(float start, float end, int r, int g, int b) {
    fogEnable = true;
    fogStart = start;
    fogEnd = end;
    fogColor = toD3DCOLOR(ColorRGB(r, g, b), 0xff);
  }
}

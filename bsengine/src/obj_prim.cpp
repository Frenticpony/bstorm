#include <bstorm/obj_prim.hpp>

#include <bstorm/type.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/game_state.hpp>

#include <d3dx9.h>

static bool isValidIndex(int i, const std::vector<bstorm::Vertex>& v) {
  return i >= 0 && i < v.size();
}

namespace bstorm {
  ObjPrim::ObjPrim(const std::shared_ptr<GameState>& state) :
    ObjRender(state),
    primType(D3DPT_TRIANGLELIST)
  {
  }

  ObjPrim::~ObjPrim() {
  }

  void ObjPrim::update() {
  }

  int ObjPrim::getPrimitiveType() const {
    return static_cast<int>(primType);
  }

  void ObjPrim::setPrimitiveType(int t) {
    primType = static_cast<_D3DPRIMITIVETYPE>(t);
  }

  void ObjPrim::setVertexCount(int cnt, bool doClear) {
    if (doClear) vertices.clear();
    vertices.resize(cnt);
  }

  int ObjPrim::getVertexCount() const {
    return vertices.size();
  }

  void ObjPrim::setTexture(const std::shared_ptr<Texture> &tex) {
    renderTarget.reset();
    texture = tex;
  }

  void ObjPrim::setRenderTarget(const std::shared_ptr<RenderTarget>& target) {
    texture.reset();
    renderTarget = target;
  }

  void ObjPrim::setVertexPosition(int vIdx, float x, float y, float z) {
    if (isValidIndex(vIdx, vertices)) {
      vertices[vIdx].x = x;
      vertices[vIdx].y = y;
      vertices[vIdx].z = z;
    }
  }

  float ObjPrim::getVertexPositionX(int vIdx) const {
    if (isValidIndex(vIdx, vertices)) {
      return vertices[vIdx].x;
    }
    return 0;
  }

  float ObjPrim::getVertexPositionY(int vIdx) const {
    if (isValidIndex(vIdx, vertices)) {
      return vertices[vIdx].y;
    }
    return 0;
  }

  float ObjPrim::getVertexPositionZ(int vIdx) const {
    if (isValidIndex(vIdx, vertices)) {
      return vertices[vIdx].z;
    }
    return 0;
  }

  void ObjPrim::setVertexUV(int vIdx, float u, float v) {
    if (isValidIndex(vIdx, vertices)) {
      vertices[vIdx].u = u;
      vertices[vIdx].v = v;
    }
  }

  void ObjPrim::setVertexUVT(int vIdx, float u, float v) {
    auto d3DTex = getD3DTexture();
    if (d3DTex) {
      setVertexUV(vIdx, u / getD3DTextureWidth(d3DTex), v / getD3DTextureHeight(d3DTex));
    }
  }

  void ObjPrim::setVertexColor(int vIdx, int r, int g, int b) {
    if (isValidIndex(vIdx, vertices)) {
      ColorRGB rgb(r, g, b);
      vertices[vIdx].color = toD3DCOLOR(rgb, (vertices[vIdx].color) >> 24);
    }
  }

  void ObjPrim::setVertexAlpha(int vIdx, int a) {
    if (isValidIndex(vIdx, vertices)) {
      a = constrain(a, 0, 0xff);
      vertices[vIdx].color &= D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0);
      vertices[vIdx].color |= D3DCOLOR_RGBA(0, 0, 0, a);
    }
  }

  const std::shared_ptr<Texture>& ObjPrim::getTexture() const {
    return texture;
  }

  const std::shared_ptr<RenderTarget>& ObjPrim::getRenderTarget() const {
    return renderTarget;
  }

  const std::vector<Vertex>& ObjPrim::getVertices() const {
    return vertices;
  }

  IDirect3DTexture9 * ObjPrim::getD3DTexture() const {
    if (texture) {
      return texture->getTexture();
    } else if (renderTarget) {
      return renderTarget->getTexture();
    }
    return NULL;
  }

  _D3DPRIMITIVETYPE ObjPrim::getD3DPrimitiveType() const {
    return primType;
  }

  ObjPrim2D::ObjPrim2D(const std::shared_ptr<GameState>& state) :
    ObjPrim(state)
  {
    setType(OBJ_PRIMITIVE_2D);
  }

  void ObjPrim2D::render() {
    D3DXMATRIX world = scaleRotTrans(getX(), getY(), getZ(), getAngleX(), getAngleY(), getAngleZ(), getScaleX(), getScaleY(), getScaleZ());
    if (auto state = getGameState()) {
      state->renderer->renderPrim2D(getD3DPrimitiveType(), vertices.size(), vertices.data(), getD3DTexture(), getBlendType(), world, getAppliedShader(), isPermitCamera(), true);
    }
  }

  ObjSprite2D::ObjSprite2D(const std::shared_ptr<GameState>& state) :
    ObjPrim2D(state)
  {
    setType(OBJ_SPRITE_2D);
    setPrimitiveType(PRIMITIVE_TRIANGLESTRIP);
    setVertexCount(4);
  }

  void ObjSprite2D::setSourceRect(float left, float top, float right, float bottom) {
    setVertexUVT(0, left, top);
    setVertexUVT(1, right, top);
    setVertexUVT(2, left, bottom);
    setVertexUVT(3, right, bottom);
  }

  void ObjSprite2D::setDestRect(float left, float top, float right, float bottom) {
    setVertexPosition(0, left, top, 0);
    setVertexPosition(1, right, top, 0);
    setVertexPosition(2, left, bottom, 0);
    setVertexPosition(3, right, bottom, 0);
  }

  void ObjSprite2D::setDestCenter() {
    if (getVertexCount() < 4) return;
    auto texture = getD3DTexture();
    if (texture) {
      float hw = getD3DTextureWidth(texture) / 2.0 * (vertices[3].u - vertices[0].u);
      float hh = getD3DTextureHeight(texture) / 2.0 * (vertices[3].v - vertices[0].v);
      setDestRect(-hw, -hh, hw, hh);
    }
  }

  ObjSpriteList2D::ObjSpriteList2D(const std::shared_ptr<GameState>& state) :
    ObjPrim2D(state),
    isVertexClosed(false),
    srcRectLeft(0),
    srcRectTop(0),
    srcRectRight(0),
    srcRectBottom(0),
    dstRectLeft(0),
    dstRectTop(0),
    dstRectRight(0),
    dstRectBottom(0)
  {
    setType(OBJ_SPRITE_LIST_2D);
    setPrimitiveType(D3DPT_TRIANGLELIST);
  }

  void ObjSpriteList2D::render() {
    // 検討： インデックスバッファを使う?
    if (isVertexClosed) {
      ObjPrim2D::render();
    } else {
      // 保存
      float prevX = getX();
      float prevY = getY();
      float prevZ = getZ();
      float prevAngleX = getAngleX();
      float prevAngleY = getAngleY();
      float prevAngleZ = getAngleZ();
      float prevScaleX = getScaleX();
      float prevScaleY = getScaleY();
      float prevScaleZ = getScaleZ();
      // パラメータをリセットして描画
      setPosition(0, 0, 0);
      setAngleXYZ(0, 0, 0);
      setScaleXYZ(1, 1, 1);
      ObjPrim2D::render();
      // 復元
      setPosition(prevX, prevY, prevZ);
      setAngleXYZ(prevAngleX, prevAngleY, prevAngleZ);
      setScaleXYZ(prevScaleX, prevScaleY, prevScaleZ);
    }
  }

  void ObjSpriteList2D::setSourceRect(float left, float top, float right, float bottom) {
    srcRectLeft = left;
    srcRectTop = top;
    srcRectRight = right;
    srcRectBottom = bottom;
  }

  void ObjSpriteList2D::setDestRect(float left, float top, float right, float bottom) {
    dstRectLeft = left;
    dstRectTop = top;
    dstRectRight = right;
    dstRectBottom = bottom;
  }

  void ObjSpriteList2D::setDestCenter() {
    /* テクスチャが存在しなくても実行できるが、弾幕風の実装に合わせる */
    if (getD3DTexture()) {
      float hw = (srcRectRight - srcRectLeft) / 2.0;
      float hh = (srcRectBottom - srcRectTop) / 2.0;
      setDestRect(-hw, -hh, hw, hh);
    }
  }

  void ObjSpriteList2D::addVertex() {
    // 頂点を6つ生成する
    // 並びは
    // 0-2 3
    // |/ /|
    // 1 4-5

    // 座標変換
    D3DXMATRIX mat = scaleRotTrans(getX(), getY(), getZ(), getAngleX(), getAngleY(), getAngleZ(), getScaleX(), getScaleY(), getScaleZ());
    D3DXMATRIX vecs;
    // 1行目:左上
    // 2行目:左下
    // 3行目:右上
    // 4行目:右下
    vecs._11 = vecs._21 = dstRectLeft;
    vecs._12 = vecs._32 = dstRectTop;
    vecs._31 = vecs._41 = dstRectRight;
    vecs._22 = vecs._42 = dstRectBottom;
    vecs._13 = vecs._23 = vecs._33 = vecs._43 = 0; // z = 0
    vecs._14 = vecs._24 = vecs._34 = vecs._44 = 1; // w = 1
    auto result = vecs * mat;
    float ul = 0;
    float vt = 0;
    float ur = 0;
    float vb = 0;
    auto texture = getD3DTexture();
    if (texture) {
      int texWidth = getD3DTextureWidth(texture);
      int texHeight = getD3DTextureHeight(texture);
      ul = srcRectLeft / texWidth;
      vt = srcRectTop / texHeight;
      ur = srcRectRight / texWidth;
      vb = srcRectBottom / texHeight;
    }
    D3DCOLOR color = getD3DCOLOR();
    Vertex v0(result._11, result._12, 0, color, ul, vt);
    Vertex v1(result._21, result._22, 0, color, ul, vb);
    Vertex v2(result._31, result._32, 0, color, ur, vt);
    Vertex& v3 = v2;
    Vertex& v4 = v1;
    Vertex v5(result._41, result._42, 0, color, ur, vb);
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);
    vertices.push_back(v5);
  }

  void ObjSpriteList2D::closeVertex() {
    setPosition(0, 0, 0);
    setAngleXYZ(0, 0, 0);
    setScaleXYZ(1, 1, 1);
    isVertexClosed = true;
  }

  void ObjSpriteList2D::clearVerexCount() {
    setVertexCount(0);
    isVertexClosed = false;
  }

  ObjPrim3D::ObjPrim3D(const std::shared_ptr<GameState>& state) :
    ObjPrim(state),
    billboardEnable(false)
  {
    setType(OBJ_PRIMITIVE_3D);
  }

  void ObjPrim3D::render() {
    D3DXMATRIX world = scaleRotTrans(getX(), getY(), getZ(), getAngleX(), getAngleY(), getAngleZ(), getScaleX(), getScaleY(), getScaleZ());
    if (auto state = getGameState()) {
      state->renderer->renderPrim3D(getD3DPrimitiveType(), vertices.size(), vertices.data(), getD3DTexture(), getBlendType(), world, getAppliedShader(), isZWriteEnabled(), isZTestEnabled(), isFogEnabled(), isBillboardEnabled());
    }
  }

  bool ObjPrim3D::isBillboardEnabled() const {
    return billboardEnable;
  }

  ObjSprite3D::ObjSprite3D(const std::shared_ptr<GameState>& state) :
    ObjPrim3D(state)
  {
    setType(OBJ_SPRITE_3D);
    setPrimitiveType(PRIMITIVE_TRIANGLESTRIP);
    setVertexCount(4);
  }

  void ObjSprite3D::setSourceRect(float left, float top, float right, float bottom) {
    // NOTE : Sprite2Dとは頂点の順番が違う
    setVertexUVT(0, left, top);
    setVertexUVT(1, left, bottom);
    setVertexUVT(2, right, top);
    setVertexUVT(3, right, bottom);
  }

  void ObjSprite3D::setDestRect(float left, float top, float right, float bottom) {
    // 弾幕風の仕様で-0.5
    left -= 0.5f;
    top -= 0.5f;
    right -= 0.5f;
    bottom -= 0.5f;
    // NOTE : Sprite2Dとは頂点の順番が違う
    setVertexPosition(0, left, top, 0);
    setVertexPosition(1, left, bottom, 0);
    setVertexPosition(2, right, top, 0);
    setVertexPosition(3, right, bottom, 0);
  }

  void ObjSprite3D::setSourceDestRect(float left, float top, float right, float bottom) {
    setSourceRect(left, top, right, bottom);
    float hw = (right - left) / 2;
    float hh = (bottom - top) / 2;
    setDestRect(-hw, -hh, hw, hh);
  }

  void ObjSprite3D::setBillboard(bool enable) {
    billboardEnable = enable;
  }
}
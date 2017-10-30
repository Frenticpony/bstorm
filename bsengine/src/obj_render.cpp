#include <vector>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/shader.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/obj_shot.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/obj_render.hpp>

namespace bstorm {
  ObjRender::ObjRender(const std::shared_ptr<GameState>& gameState) :
    Obj(gameState),
    visibleFlag(true),
    priority(-1),
    x(0),
    y(0),
    z(0),
    angleX(0),
    angleY(0),
    angleZ(0),
    scaleX(1),
    scaleY(1),
    scaleZ(1),
    alpha(0xff),
    blendType(BLEND_ALPHA),
    fogEnable(true),
    zWriteEnable(true),
    zTestEnable(true),
    permitCamera(true)
  {
  }

  ObjRender::~ObjRender() {
  }

  void ObjRender::setColor(int r, int g, int b) {
    rgb = ColorRGB(r, g, b);
  }

  void ObjRender::setAlpha(int a) {
    alpha = constrain(a, 0, 0xff);
  }

  void ObjRender::setColorHSV(int h, int s, int v) {
    h %= 360;
    if (h < 0) h += 360;
    s = constrain(s, 0, 255);
    v = constrain(v, 0, 255);
    int r, g, b;
    if (s == 0) {
      r = g = b = v;
    } else {
      int hi = floor(1.0 * h / 60);
      float f = 1.0 * h / 60 - hi;
      int m = 1.0 * v * (1.0 - 1.0 * s / 255);
      int n = 1.0 * v * (1.0 - f * s / 255);
      int k = 1.0 * v * (1.0 - (1.0 - f) * s / 255);
      switch (hi) {
        case 0:
          r = v;
          g = k;
          b = m;
          break;
        case 1:
          r = n;
          g = v;
          b = m;
          break;
        case 2:
          r = m;
          g = v;
          b = k;
          break;
        case 3:
          r = m;
          g = n;
          b = v;
          break;
        case 4:
          r = k;
          g = m;
          b = v;
          break;
        case 5:
        default:
          r = v;
          g = m;
          b = n;
          break;
      }
    }
    setColor(r, g, b);
  }

  D3DCOLOR ObjRender::getD3DCOLOR() const {
    return toD3DCOLOR(rgb, alpha);
  }

  void ObjRender::setShader(const std::shared_ptr<Shader>& s) {
    shader = s;
  }

  void ObjRender::setShaderO(const std::shared_ptr<ObjRender>& obj) {
    shader = obj->shader;
  }

  void ObjRender::resetShader() {
    shader.reset();
  }

  void ObjRender::setShaderTechnique(const std::string & name) {
    if (shader) {
      shader->getEffect()->SetTechnique(name.c_str());
    }
  }

  void ObjRender::setShaderVector(const std::string & name, float x, float y, float z, float w) {
    if (shader) {
      D3DXVECTOR4 vec4(x, y, z, w);
      shader->getEffect()->SetVector(name.c_str(), &vec4);
    }
  }

  void ObjRender::setShaderFloat(const std::string & name, float f) {
    if (shader) {
      shader->getEffect()->SetFloat(name.c_str(), f);
    }
  }

  void ObjRender::setShaderFloatArray(const std::string & name, const std::vector<float>& fs) {
    if (shader) {
      shader->getEffect()->SetFloatArray(name.c_str(), &fs[0], fs.size());
    }
  }

  void ObjRender::setShaderTexture(const std::string & name, const std::shared_ptr<Texture>& texture) {
    if (shader && texture) {
      shader->setTexture(name, texture->getTexture());
    }
  }

  void ObjRender::setShaderTexture(const std::string & name, const std::shared_ptr<RenderTarget>& renderTarget) {
    if (shader && renderTarget) {
      shader->setTexture(name, renderTarget->getTexture());
    }
  }

  std::shared_ptr<Shader> ObjRender::getAppliedShader() const {
    if (shader) return shader;
    if (auto state = getGameState()) {
      return state->objLayerList->getLayerShader(priority);
    }
    return nullptr;
  }

  ObjectLayerList::ObjectLayerList() :
    shotRenderPriority(DEFAULT_SHOT_RENDER_PRIORITY),
    itemRenderPriority(DEFAULT_ITEM_RENDER_PRIORITY),
    cameraFocusPermitRenderPriority(DEFAULT_CAMERA_FOCUS_PERMIT_RENDER_PRIORITY),
    stgFrameRenderPriorityMin(DEFAULT_STG_FRAME_RENDER_PRIORITY_MIN),
    stgFrameRenderPriorityMax(DEFAULT_STG_FRAME_RENDER_PRIORITY_MAX),
    invalidRenderPriorityMin(-1),
    invalidRenderPriorityMax(-1)
  {
  }

  ObjectLayerList::~ObjectLayerList() {}

  void ObjectLayerList::remove(const std::shared_ptr<ObjRender>& obj) {
    if (obj->priority >= 0) {
      obj->posInLayer->reset();
      obj->priority = -1;
    }
  }

  void ObjectLayerList::setRenderPriority(const std::shared_ptr<ObjRender>& obj, int p) {
    if (p < 0 || p > MAX_RENDER_PRIORITY) return;
    // 現在のレイヤーから削除
    remove(obj);
    // 新しいレイヤーに追加
    auto& layer = layers.at(p);
    obj->posInLayer = layer.insert(layer.end(), obj);
    obj->priority = p;
  }

  void ObjectLayerList::renderLayer(int p, bool ignoreStgSceneObj, bool checkVisibleFlag) {
    if (p < 0 || p > MAX_RENDER_PRIORITY) return;

    auto& layer = layers.at(p);

    // バケツソートする
    std::vector<std::shared_ptr<ObjRender>> addShots;
    std::vector<std::shared_ptr<ObjRender>> mulShots;
    std::vector<std::shared_ptr<ObjRender>> subShots;
    std::vector<std::shared_ptr<ObjRender>> invShots;
    std::vector<std::shared_ptr<ObjRender>> alphaShots;
    std::vector<std::shared_ptr<ObjRender>> others;

    auto it = layer.begin();
    while (it != layer.end()) {
      auto& obj = it->lock();
      if (!obj) {
        //解放済み
        it = layer.erase(it);
        continue;
      }
      ++it;

      // 終了状態 or 非表示状態
      if (obj->isDead() || (checkVisibleFlag && !obj->isVisible())) { continue; }

      // StgSceneのオブジェクトを描画するかどうか
      if (ignoreStgSceneObj && obj->isStgSceneObject()) { continue; }

      auto shot = std::dynamic_pointer_cast<ObjShot>(obj);

      // 非Shot
      if (!shot) {
        others.push_back(obj);
        continue;
      }

      if (const auto& shotData = shot->getShotData()) {
        int blendType;
        if (!shot->isDelay()) {
          blendType = shot->getBlendType();
          if (blendType == BLEND_NONE) {
            blendType = shotData->render;
          }
        } else {
          blendType = shot->getSourceBlendType();
          if (blendType == BLEND_NONE) {
            blendType = shotData->delayRender;
          }
        }
        switch (blendType) {
          case BLEND_ALPHA:
            alphaShots.push_back(shot);
            break;
          case BLEND_ADD_RGB:
          case BLEND_ADD_ARGB:
            addShots.push_back(shot);
            break;
          case BLEND_MULTIPLY:
            mulShots.push_back(shot);
            break;
          case BLEND_SUBTRACT:
            subShots.push_back(shot);
            break;
          case BLEND_INV_DESTRGB:
            invShots.push_back(shot);
            break;
        }
      }
    }
    for (auto shot : addShots) { shot->render(); }
    for (auto shot : mulShots) { shot->render(); }
    for (auto shot : subShots) { shot->render(); }
    for (auto shot : invShots) { shot->render(); }
    for (auto shot : alphaShots) { shot->render(); }
    for (auto obj : others) { obj->render(); }
  }

  void ObjectLayerList::setLayerShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader) {
    beginPriority = std::max(0, beginPriority);
    endPriority = std::min(endPriority, MAX_RENDER_PRIORITY);
    for (int p = beginPriority; p <= endPriority; p++) {
      layerShaders[p] = shader;
    }
  }

  void ObjectLayerList::resetLayerShader(int beginPriority, int endPriority) {
    beginPriority = std::max(0, beginPriority);
    endPriority = std::min(endPriority, MAX_RENDER_PRIORITY);
    for (int p = beginPriority; p <= endPriority; p++) {
      layerShaders[p].reset();
    }
  }

  std::shared_ptr<Shader> ObjectLayerList::getLayerShader(int p) const {
    if (p < 0 || p > MAX_RENDER_PRIORITY) {
      return nullptr;
    }
    return layerShaders.at(p);
  }

  int ObjectLayerList::getShotRenderPriority() const {
    return shotRenderPriority;
  }

  void ObjectLayerList::setShotRenderPriority(int p) {
    shotRenderPriority = p;
  }

  int ObjectLayerList::getItemRenderPriority() const {
    return itemRenderPriority;
  }

  void ObjectLayerList::setItemRenderPriority(int p) {
    itemRenderPriority = p;
  }

  int ObjectLayerList::getCameraFocusPermitRenderPriority() const {
    return cameraFocusPermitRenderPriority;
  }

  int ObjectLayerList::getStgFrameRenderPriorityMin() const {
    return stgFrameRenderPriorityMin;
  }

  void ObjectLayerList::setStgFrameRenderPriorityMin(int p) {
    stgFrameRenderPriorityMin = p;
  }

  int ObjectLayerList::getStgFrameRenderPriorityMax() const {
    return stgFrameRenderPriorityMax;
  }

  void ObjectLayerList::setStgFrameRenderPriorityMax(int p) {
    stgFrameRenderPriorityMax = p;
  }

  bool ObjectLayerList::isInvalidRenderPriority(int p) const {
    return p <= invalidRenderPriorityMax && invalidRenderPriorityMin <= p;
  }

  void ObjectLayerList::setInvalidRenderPriority(int min, int max) {
    invalidRenderPriorityMin = min;
    invalidRenderPriorityMax = max;
  }

  void ObjectLayerList::clearInvalidRenderPriority() {
    invalidRenderPriorityMin = invalidRenderPriorityMax = -1;
  }

  ObjShader::ObjShader(const std::shared_ptr<GameState>& gameState) : ObjRender(gameState) {
    setType(OBJ_SHADER);
  }
  ObjShader::~ObjShader() {}
}
#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <d3d9.h>

#include <bstorm/type.hpp>
#include <bstorm/const.hpp>
#include <bstorm/obj.hpp>

namespace bstorm {
  class Shader;
  class Texture;
  class RenderTarget;
  class ObjectLayerList;
  class ObjRender : public Obj {
  public:
    ObjRender(const std::shared_ptr<GameState>& gameState);
    ~ObjRender();
    virtual void render() = 0;
    bool isVisible() const { return visibleFlag; }
    void setVisible(bool visible) { visibleFlag = visible; }
    int getRenderPriority() const { return priority; }
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    void setX(float x) { transIntersection(x - this->x, 0); this->x = x; }
    void setY(float y) { transIntersection(0, y - this->y); this->y = y; }
    void setZ(float z) { this->z = z; }
    void setPosition(float x, float y, float z) {
      transIntersection(x - this->x, y - this->y);
      this->x = x;
      this->y = y;
      this->z = z;
    }
    float getAngleX() const { return angleX; }
    float getAngleY() const { return angleY; }
    float getAngleZ() const { return angleZ; }
    void setAngleX(float angleX) { this->angleX = angleX; }
    void setAngleY(float angleY) { this->angleY = angleY; }
    void setAngleZ(float angleZ) { this->angleZ = angleZ; }
    void setAngleXYZ(float angleX, float angleY, float angleZ) {
      this->angleX = angleX;
      this->angleY = angleY;
      this->angleZ = angleZ;
    }
    float getScaleX() const { return scaleX; }
    float getScaleY() const { return scaleY; }
    float getScaleZ() const { return scaleZ; }
    void setScaleX(float scaleX) { this->scaleX = scaleX; }
    void setScaleY(float scaleY) { this->scaleY = scaleY; }
    void setScaleZ(float scaleZ) { this->scaleZ = scaleZ; }
    void setScaleXYZ(float scaleX, float scaleY, float scaleZ) {
      this->scaleX = scaleX;
      this->scaleY = scaleY;
      this->scaleZ = scaleZ;
    }
    const ColorRGB& getColor() const { return rgb; }
    int getAlpha() const { return alpha; }
    D3DCOLOR getD3DCOLOR() const;
    void setColor(int r, int g, int b);
    void setAlpha(int a);
    void setColorHSV(int h, int s, int v);
    int getBlendType() const { return blendType; }
    void setBlendType(int t) { blendType = t; }
    bool isFogEnabled() const { return fogEnable; }
    void setFogEnable(bool enable) { fogEnable = enable; }
    bool isZWriteEnabled() const { return zWriteEnable; }
    void setZWrite(bool enable) { zWriteEnable = enable; }
    bool isZTestEnabled() const { return zTestEnable; }
    void setZTest(bool enable) { zTestEnable = enable; }
    bool isPermitCamera() const { return permitCamera; }
    void setPermitCamera(bool permit) { permitCamera = permit; }
    void setShader(const std::shared_ptr<Shader>& shader);
    void setShaderO(const std::shared_ptr<ObjRender>& obj);
    const std::shared_ptr<Shader>& getShader() const { return shader; }
    void resetShader();
    void setShaderTechnique(const std::string& name);
    void setShaderVector(const std::string& name, float x, float y, float z, float w);
    void setShaderFloat(const std::string& name, float f);
    void setShaderFloatArray(const std::string& name, const std::vector<float>& fs);
    void setShaderTexture(const std::string& name, const std::shared_ptr<Texture>& texture);
    void setShaderTexture(const std::string& name, const std::shared_ptr<RenderTarget>& renderTarget);
  protected:
    std::shared_ptr<Shader> getAppliedShader() const;
    virtual void transIntersection(float dx, float dy) {}
  private:
    bool visibleFlag;
    int priority;
    float x;
    float y;
    float z;
    float angleX;
    float angleY;
    float angleZ;
    float scaleX;
    float scaleY;
    float scaleZ;
    ColorRGB rgb;
    int alpha;
    int blendType;
    bool fogEnable;
    bool zWriteEnable;
    bool zTestEnable;
    bool permitCamera;
    std::list<std::weak_ptr<ObjRender>>::iterator posInLayer;
    std::shared_ptr<Shader> shader;
    friend ObjectLayerList;
  };

  class ObjShader : public ObjRender {
  public:
    ObjShader(const std::shared_ptr<GameState>& gameState);
    ~ObjShader();
    void update() override {}
    void render() override {};
  };

  class ObjectLayerList {
  public:
    ObjectLayerList();
    ~ObjectLayerList();
    void setRenderPriority(const std::shared_ptr<ObjRender>& obj, int p);
    void renderLayer(int priority, bool ignoreStgSceneObj, bool checkVisibleFlag);
    void setLayerShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader);
    void resetLayerShader(int beginPriority, int endPriority);
    std::shared_ptr<Shader> getLayerShader(int p) const;
    int getShotRenderPriority() const;
    void setShotRenderPriority(int p);
    int getItemRenderPriority() const;
    void setItemRenderPriority(int p);
    int getCameraFocusPermitRenderPriority() const;
    int getStgFrameRenderPriorityMin() const;
    void setStgFrameRenderPriorityMin(int p);
    int getStgFrameRenderPriorityMax() const;
    void setStgFrameRenderPriorityMax(int p);
    bool isInvalidRenderPriority(int p) const;
    void setInvalidRenderPriority(int min, int max);
    void clearInvalidRenderPriority();
  private:
    void remove(const std::shared_ptr<ObjRender>& obj);
    std::array<std::list<std::weak_ptr<ObjRender>>, MAX_RENDER_PRIORITY + 1> layers;
    std::array<std::shared_ptr<Shader>, MAX_RENDER_PRIORITY + 1> layerShaders;
    int shotRenderPriority;
    int itemRenderPriority;
    int cameraFocusPermitRenderPriority;
    int stgFrameRenderPriorityMin;
    int stgFrameRenderPriorityMax;
    int invalidRenderPriorityMin;
    int invalidRenderPriorityMax;
  };
}
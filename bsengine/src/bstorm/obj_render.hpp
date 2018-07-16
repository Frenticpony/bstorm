#pragma once

#include <bstorm/color_rgb.hpp>
#include <bstorm/obj.hpp>

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <d3d9.h>

namespace bstorm
{
class Renderer;
class Shader;
class Texture;
class RenderTarget;
class ObjectLayerList;
class ObjRender : public Obj
{
public:
    ObjRender(const std::shared_ptr<Package>& package);
    ~ObjRender();
    virtual void Render(const std::shared_ptr<Renderer>& renderer) = 0;
    bool IsVisible() const { return visibleFlag_; }
    void SetVisible(bool visible) { visibleFlag_ = visible; }
    int getRenderPriority() const { return priority_; }
    float GetX() const { return x_; }
    float GetY() const { return y_; }
    float GetZ() const { return z_; }
    void SetX(float x) { OnTrans(x - this->x_, 0); this->x_ = x; }
    void SetY(float y) { OnTrans(0, y - this->y_); this->y_ = y; }
    void SetZ(float z) { this->z_ = z; }
    void SetPosition(float x, float y, float z)
    {
        OnTrans(x - this->x_, y - this->y_);
        this->x_ = x;
        this->y_ = y;
        this->z_ = z;
    }
    float GetAngleX() const { return angleX_; }
    float GetAngleY() const { return angleY_; }
    float GetAngleZ() const { return angleZ_; }
    void SetAngleX(float angleX) { this->angleX_ = angleX; }
    void SetAngleY(float angleY) { this->angleY_ = angleY; }
    void SetAngleZ(float angleZ) { this->angleZ_ = angleZ; }
    void SetAngleXYZ(float angleX, float angleY, float angleZ)
    {
        this->angleX_ = angleX;
        this->angleY_ = angleY;
        this->angleZ_ = angleZ;
    }
    float GetScaleX() const { return scaleX_; }
    float GetScaleY() const { return scaleY_; }
    float GetScaleZ() const { return scaleZ_; }
    void SetScaleX(float scaleX) { this->scaleX_ = scaleX; }
    void SetScaleY(float scaleY) { this->scaleY_ = scaleY; }
    void SetScaleZ(float scaleZ) { this->scaleZ_ = scaleZ; }
    void SetScaleXYZ(float scaleX, float scaleY, float scaleZ)
    {
        this->scaleX_ = scaleX;
        this->scaleY_ = scaleY;
        this->scaleZ_ = scaleZ;
    }
    const ColorRGB& GetColor() const { return rgb_; }
    int GetAlpha() const { return alpha_; }
    D3DCOLOR GetD3DCOLOR() const;
    void SetColor(int r, int g, int b);
    void SetAlpha(int a);
    void SetColorHSV(int h, int s, int v);
    int GetBlendType() const { return blendType_; }
    void SetBlendType(int t) { blendType_ = t; }
    bool IsFogEnabled() const { return fogEnable_; }
    void SetFogEnable(bool enable) { fogEnable_ = enable; }
    bool IsZWriteEnabled() const { return zWriteEnable_; }
    void SetZWrite(bool enable) { zWriteEnable_ = enable; }
    bool IsZTestEnabled() const { return zTestEnable_; }
    void SetZTest(bool enable) { zTestEnable_ = enable; }
    bool IsPermitCamera() const { return permitCamera_; }
    void SetPermitCamera(bool permit) { permitCamera_ = permit; }
    void SetShader(const std::shared_ptr<Shader>& shader);
    void SetShaderO(const std::shared_ptr<ObjRender>& obj);
    const std::shared_ptr<Shader>& GetShader() const { return shader_; }
    void ResetShader();
    void SetShaderTechnique(const std::string& name);
    void SetShaderVector(const std::string& name, float x, float y, float z, float w);
    void SetShaderFloat(const std::string& name, float f);
    void SetShaderFloatArray(const std::string& name, const std::vector<float>& fs);
    void SetShaderTexture(const std::string& name, const std::shared_ptr<Texture>& texture);
    void SetShaderTexture(const std::string& name, const std::shared_ptr<RenderTarget>& renderTarget);
protected:
    NullableSharedPtr<Shader> GetAppliedShader() const;
    // 移動時コールバック
    virtual void OnTrans(float dx, float dy) {}
private:
    bool visibleFlag_;
    int priority_;
    float x_;
    float y_;
    float z_;
    float angleX_;
    float angleY_;
    float angleZ_;
    float scaleX_;
    float scaleY_;
    float scaleZ_;
    ColorRGB rgb_;
    int alpha_;
    int blendType_;
    bool fogEnable_;
    bool zWriteEnable_;
    bool zTestEnable_;
    bool permitCamera_;
    std::list<std::weak_ptr<ObjRender>>::iterator posInLayer_;
    NullableSharedPtr<Shader> shader_;
    friend class ObjectLayerList;
};

class ObjShader : public ObjRender
{
public:
    ObjShader(const std::shared_ptr<Package>& package);
    ~ObjShader();
    void Update() override {}
    void Render(const std::shared_ptr<Renderer>& renderer) override {};
};

constexpr int MAX_RENDER_PRIORITY = 100;
constexpr int DEFAULT_STG_FRAME_RENDER_PRIORITY_MIN = 20;
constexpr int DEFAULT_STG_FRAME_RENDER_PRIORITY_MAX = 80;
constexpr int DEFAULT_PLAYER_RENDER_PRIORITY = 30;
constexpr int DEFAULT_ENEMY_RENDER_PRIORITY = 40;
constexpr int DEFAULT_SHOT_RENDER_PRIORITY = 50;
constexpr int DEFAULT_ITEM_RENDER_PRIORITY = 60;
constexpr int DEFAULT_CAMERA_FOCUS_PERMIT_RENDER_PRIORITY = 69;

class ObjectLayerList
{
public:
    ObjectLayerList();
    ~ObjectLayerList();
    void SetRenderPriority(const std::shared_ptr<ObjRender>& obj, int p);
    void RenderLayer(int priority, bool ignoreStgSceneObj, bool checkVisibleFlag, const std::shared_ptr<Renderer>& renderer);
    void SetLayerShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader);
    void ResetLayerShader(int beginPriority, int endPriority);
    NullableSharedPtr<Shader> GetLayerShader(int p) const;
    int GetShotRenderPriority() const;
    void SetShotRenderPriority(int p);
    int GetItemRenderPriority() const;
    void SetItemRenderPriority(int p);
    int GetCameraFocusPermitRenderPriority() const;
    int GetStgFrameRenderPriorityMin() const;
    void SetStgFrameRenderPriorityMin(int p);
    int GetStgFrameRenderPriorityMax() const;
    void SetStgFrameRenderPriorityMax(int p);
    bool IsInvalidRenderPriority(int p) const;
    void SetInvalidRenderPriority(int min, int max);
    void ClearInvalidRenderPriority();
private:
    void Remove(const std::shared_ptr<ObjRender>& obj);
    std::array<std::list<std::weak_ptr<ObjRender>>, MAX_RENDER_PRIORITY + 1> layers_;
    std::array<std::shared_ptr<Shader>, MAX_RENDER_PRIORITY + 1> layerShaders_;
    int shotRenderPriority_;
    int itemRenderPriority_;
    int cameraFocusPermitRenderPriority_;
    int stgFrameRenderPriorityMin_;
    int stgFrameRenderPriorityMax_;
    int invalidRenderPriorityMin_;
    int invalidRenderPriorityMax_;
};
}
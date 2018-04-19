#pragma once

struct D3DXMATRIX;

namespace bstorm
{
class Camera3D
{
public:
    Camera3D();
    ~Camera3D() {}
    void SetFocusX(float fx) { focusX_ = fx; }
    void SetFocusY(float fy) { focusY_ = fy; }
    void SetFocusZ(float fz) { focusZ_ = fz; }
    void SetFocusXYZ(float fx, float fy, float fz) { SetFocusX(fx); SetFocusY(fy); SetFocusZ(fz); }
    void SetRadius(float r) { radius_ = r; }
    void SetAzimuthAngle(float angle) { azimuthAngle_ = angle; }
    void SetElevationAngle(float angle) { elevationAngle_ = angle; }
    void SetYaw(float angle) { yaw_ = angle; }
    void SetPitch(float angle) { pitch_ = angle; }
    void SetRoll(float angle) { roll_ = angle; }
    void SetPerspectiveClip(float zn, float zf) { nearClip_ = zn; farClip_ = zf; }
    float GetFocusX() const { return focusX_; }
    float GetFocusY() const { return focusY_; }
    float GetFocusZ() const { return focusZ_; }
    float GetX() const;
    float GetY() const;
    float GetZ() const;
    float GetRadius() const { return radius_; }
    float GetAzimuthAngle() const { return azimuthAngle_; }
    float GetElevationAngle() const { return elevationAngle_; }
    float GetYaw() const { return yaw_; }
    float GetPitch() const { return pitch_; }
    float GetRoll() const { return roll_; }
    float GetNearPerspectiveClip() const { return nearClip_; }
    float GetFarPerspectiveClip() const { return farClip_; }
    void GenerateViewMatrix(D3DXMATRIX* view, D3DXMATRIX* billboard) const; // billboard: 物体をカメラ正面に向かせるための行列
    void GenerateProjMatrix(D3DXMATRIX* proj, float screenWidth, float screenHeight, float cameraScreenX, float cameraScreenY) const;
private:
    float focusX_;
    float focusY_;
    float focusZ_;
    float radius_;
    float azimuthAngle_;
    float elevationAngle_;
    float yaw_;
    float pitch_;
    float roll_;
    float nearClip_;
    float farClip_;
};
}
#pragma once

struct D3DXMATRIX;

namespace bstorm
{
class Camera2D
{
public:
    Camera2D();
    ~Camera2D() {};
    void SetFocusX(float fx) { focusX_ = fx; }
    void SetFocusY(float fy) { focusY_ = fy; }
    void SetAngleZ(float rotZ) { angleZ_ = rotZ; }
    void SetRatio(float r) { SetRatioX(r); SetRatioY(r); }
    void SetRatioX(float rx) { ratioX_ = rx; }
    void SetRatioY(float ry) { ratioY_ = ry; }
    float GetX() const { return focusX_; }
    float GetY() const { return focusY_; }
    float GetAngleZ() const { return angleZ_; }
    float GetRatio() const; // max(RatioX, RatioY)
    float GetRatioX() const { return ratioX_; }
    float GetRatioY() const { return ratioY_; }
    void Reset(float focusX, float focusY);
    void GenerateViewMatrix(D3DXMATRIX* view) const;
    void GenerateProjMatrix(D3DXMATRIX* proj, float screenWidth, float screenHeight, float cameraScreenX, float cameraScreenY) const;
private:
    // focus: カメラの中心に持ってきたいワールド座標
    float focusX_;
    float focusY_;
    float angleZ_;
    float ratioX_;
    float ratioY_;
};
}

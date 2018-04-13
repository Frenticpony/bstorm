#pragma once

#include <d3dx9.h>

namespace bstorm
{
class Camera2D
{
public:
    Camera2D();
    ~Camera2D() {};
    void generateViewMatrix(D3DXMATRIX& mat) const;
    // スクリーンの幅と高さ、スクリーン座標でのcameraの位置を与える
    void generateProjMatrix(float screenWidth, float screenHeight, float cameraScreenX, float cameraScreenY, D3DXMATRIX& mat) const;
    void setFocusX(float fx) { focusX = fx; }
    void setFocusY(float fy) { focusY = fy; }
    void setAngleZ(float rot) { angle = rot; }
    void setRatio(float s) { setRatioX(s); setRatioY(s); }
    void setRatioX(float sx) { scaleX = sx; }
    void setRatioY(float sy) { scaleY = sy; }
    float getX() const { return focusX; }
    float getY() const { return focusY; }
    float getAngleZ() const { return angle; }
    float getRatio()  const;
    float getRatioX() const { return scaleX; }
    float getRatioY() const { return scaleY; }
    void reset(float fx, float fy);
private:
    float focusX; // カメラの中心に持ってきたいワールド座標
    float focusY; // カメラの中心に持ってきたいワールド座標
    float angle;
    float scaleX;
    float scaleY;
};
}

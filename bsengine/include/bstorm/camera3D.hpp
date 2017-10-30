#pragma once

#include <d3dx9.h>

namespace bstorm {
  class Camera3D {
  public:
    Camera3D();
    void setFocusX(float fx) { focusX = fx; }
    void setFocusY(float fy) { focusY = fy; }
    void setFocusZ(float fz) { focusZ = fz; }
    void setFocusXYZ(float fx, float fy, float fz) { setFocusX(fx); setFocusY(fy); setFocusZ(fz); }
    void setRadius(float r) { radius = r; }
    void setAzimuthAngle(float angle) { azimuthAngle = angle; }
    void setElevationAngle(float angle) { elevationAngle = angle; }
    void setYaw(float yaw) { this->yaw = yaw; }
    void setPitch(float pitch) { this->pitch = pitch; }
    void setRoll(float roll) { this->roll = roll; }
    void setPerspectiveClip(float zn, float zf) { nearClip = zn; farClip = zf; }
    float getFocusX() const { return focusX; }
    float getFocusY() const { return focusY; }
    float getFocusZ() const { return focusZ; }
    float getX() const;
    float getY() const;
    float getZ() const;
    float getRadius() const { return radius; }
    float getAzimuthAngle() const { return azimuthAngle; }
    float getElevationAngle() const { return elevationAngle; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getRoll() const { return roll; }
    float getNearPerspectiveClip() const { return nearClip; }
    float getFarPerspectiveClip() const { return farClip; }
    void generateViewMatrix(D3DXMATRIX& viewMatrix, D3DXMATRIX& billboardMatrix) const;
    void generateProjMatrix(float screenWidth, float screenHeight, float cameraScreenX, float cameraScreenY, D3DXMATRIX& projMatrix) const;
  private:
    float focusX;
    float focusY;
    float focusZ;
    float radius;
    float azimuthAngle;
    float elevationAngle;
    float yaw;
    float pitch;
    float roll;
    float nearClip;
    float farClip;
  };
}
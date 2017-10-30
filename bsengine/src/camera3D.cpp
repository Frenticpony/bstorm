#include <bstorm/camera3D.hpp>

namespace bstorm {
  Camera3D::Camera3D() :
    focusX(0),
    focusY(0),
    focusZ(0),
    radius(0),
    azimuthAngle(0),
    elevationAngle(0),
    yaw(0),
    pitch(0),
    roll(0),
    nearClip(0),
    farClip(1000)
  {
  }

  float Camera3D::getX() const {
    return focusX + radius * cos(D3DXToRadian(elevationAngle)) * cos(D3DXToRadian(azimuthAngle));
  }

  float Camera3D::getY() const {
    return focusY + radius * sin(D3DXToRadian(elevationAngle));
  }

  float Camera3D::getZ() const {
    return focusZ + radius * cos(D3DXToRadian(elevationAngle)) * sin(D3DXToRadian(azimuthAngle));
  }

  void Camera3D::generateViewMatrix(D3DXMATRIX& view, D3DXMATRIX& billboard) const {
    D3DXMATRIX trans, rotYawPitch, rotYawPitchRoll;
    D3DXMatrixRotationYawPitchRoll(&rotYawPitch, D3DXToRadian(yaw), D3DXToRadian(pitch), 0);
    D3DXMatrixRotationYawPitchRoll(&rotYawPitchRoll, D3DXToRadian(yaw), D3DXToRadian(pitch), D3DXToRadian(roll));
    const D3DXVECTOR3 eye = D3DXVECTOR3(getX(), getY(), getZ());
    const D3DXVECTOR3 focus = D3DXVECTOR3(focusX, focusY, focusZ);
    D3DXVECTOR3 gaze = focus - eye;
    D3DXVec3TransformCoord(&gaze, &gaze, &rotYawPitch);
    const D3DXVECTOR3 at = eye + gaze;
    D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
    D3DXVec3TransformCoord(&up, &up, &rotYawPitchRoll);
    D3DXMatrixLookAtLH(&view, &eye, &at, &up);
    D3DXMatrixLookAtLH(&billboard, &D3DXVECTOR3(0, 0, 0), &gaze, &up);
    D3DXMatrixInverse(&billboard, NULL, &billboard);
  }

  void Camera3D::generateProjMatrix(float screenWidth, float screenHeight, float cameraScreenX, float cameraScreenY, D3DXMATRIX& projMatrix) const {
    D3DXMatrixPerspectiveFovLH(&projMatrix, D3DXToRadian(47.925), screenWidth / screenHeight, nearClip, farClip);
    D3DXMATRIX trans;
    D3DXMatrixTranslation(&trans, (-screenWidth / 2.0 + cameraScreenX) * 2.0 / screenWidth, (screenHeight / 2.0 - cameraScreenY) * 2.0 / screenHeight, 0);
    projMatrix = projMatrix * trans;
  }
}
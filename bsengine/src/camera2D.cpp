#include <algorithm>

#include <bstorm/camera2D.hpp>

namespace bstorm {
  Camera2D::Camera2D() : focusX(0), focusY(0), angle(0), scaleX(1), scaleY(1) {}

  void Camera2D::generateViewMatrix(D3DXMATRIX& view) const {
    D3DXMATRIX rot, scale;
    D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(focusX, focusY, 1), &D3DXVECTOR3(focusX, focusY, 0), &D3DXVECTOR3(0, -1, 0));
    D3DXMatrixRotationZ(&rot, D3DXToRadian(-angle));
    D3DXMatrixScaling(&scale, scaleX, scaleY, 1);
    view = view * scale * rot;
  }

  void Camera2D::generateProjMatrix(float screenWidth, float screenHeight, float cameraScreenX, float cameraScreenY, D3DXMATRIX& proj) const {
    D3DXMATRIX trans;
    D3DXMatrixTranslation(&trans, -screenWidth / 2.0f + cameraScreenX, screenHeight / 2.0f - cameraScreenY, 0.0f);
    D3DXMatrixScaling(&proj, 2.0f / screenWidth, 2.0f / screenHeight, 1.0f);
    proj = trans * proj;
  }

  float Camera2D::getRatio() const { return std::max(getRatioX(), getRatioY()); }

  void Camera2D::reset(float fx, float fy) {
    setFocusX(fx);
    setFocusY(fy);
    setAngleZ(0);
    setRatio(1);
  }
}

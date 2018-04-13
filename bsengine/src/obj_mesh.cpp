#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/obj_mesh.hpp>

#include <d3d9.h>

namespace bstorm {
  ObjMesh::ObjMesh(const std::shared_ptr<GameState>& gameState) :
    ObjRender(gameState)
  {
    setType(OBJ_MESH);
  }

  ObjMesh::~ObjMesh() {
  }

  void ObjMesh::update() { }

  void ObjMesh::render() {
    if (mesh) {
      if (auto gameState = getGameState()) {
        D3DXMATRIX world = scaleRotTrans(getX(), getY(), getZ(), getAngleX(), getAngleY(), getAngleZ(), getScaleX(), getScaleY(), getScaleZ());
        const auto& rgb = getColor();
        D3DCOLORVALUE col = D3DCOLORVALUE{ rgb.getR() / 255.0f, rgb.getG() / 255.0f, rgb.getB() / 255.0f, getAlpha() / 255.0f };
        gameState->renderer->renderMesh(mesh, col, getBlendType(), world, getAppliedShader(), isZWriteEnabled(), isZTestEnabled(), isFogEnabled());
      }
    }
  }

  void ObjMesh::setMesh(const std::shared_ptr<Mesh>& mesh) {
    this->mesh = mesh;
  }
}

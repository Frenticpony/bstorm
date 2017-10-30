#pragma once

#include <memory>
#include <vector>

#include <bstorm/obj_render.hpp>

namespace bstorm {
  class Mesh;
  class ObjMesh : public ObjRender {
  public:
    ObjMesh(const std::shared_ptr<GameState>& state);
    ~ObjMesh();
    void update() override;
    void render() override;
    void setMesh(const std::shared_ptr<Mesh>& mesh);
  private:
    std::shared_ptr<Mesh> mesh;
  };
}
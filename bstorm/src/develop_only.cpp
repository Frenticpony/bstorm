#include <bstorm/obj_col.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/intersection.hpp>

namespace bstorm
{
void ObjCol::RenderIntersection(const std::shared_ptr<Renderer>& renderer, bool isPermitCamera) const {}
bool ObjPlayer::IsForceInvincible() const { return false; }
void Shape::Render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const {}
}
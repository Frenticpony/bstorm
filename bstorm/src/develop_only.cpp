#include <bstorm/obj_col.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/intersection.hpp>

namespace bstorm
{
void ObjCol::RenderIntersection(const std::shared_ptr<Renderer>& renderer, bool isPermitCamera, const std::weak_ptr<Package>& package) const {}
bool ObjPlayer::IsForceInvincible(const std::shared_ptr<Package>& package) const { return false; }
void Shape::Render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const {}
}
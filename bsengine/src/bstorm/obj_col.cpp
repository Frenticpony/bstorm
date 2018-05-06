#include <bstorm/obj_col.hpp>

#include <bstorm/intersection.hpp>
#include <bstorm/game_state.hpp>

#include <iterator>

namespace bstorm
{
ObjCol::ObjCol(const std::shared_ptr<GameState>& gameState) : gameState_(gameState) {}
ObjCol::~ObjCol() {}

void ObjCol::PushBackIntersection(const std::shared_ptr<Intersection>& isect)
{
    isects_.push_back(isect);
}

void ObjCol::PopFrontIntersection()
{
    isects_.pop_front();
}

void ObjCol::AddTempIntersection(const std::shared_ptr<Intersection>& isect)
{
    tempIsects_.push_back(isect);
}

void ObjCol::TransIntersection(float dx, float dy)
{
    if (auto state = gameState_.lock())
    {
        for (auto& isect : isects_)
        {
            state->colDetector->Trans(isect, dx, dy);
        }
    }
}

void ObjCol::SetWidthIntersection(float width)
{
    if (auto state = gameState_.lock())
    {
        for (auto& isect : isects_)
        {
            state->colDetector->SetWidth(isect, width);
        }
    }
}

void ObjCol::RenderIntersection(const std::unique_ptr<Renderer>& renderer, bool isPermitCamera) const
{
    if (auto state = gameState_.lock())
    {
        if (state->renderIntersectionEnable)
        {
            for (auto& isect : GetIntersections())
            {
                isect->Render(renderer, isPermitCamera);
            }
            for (auto& isect : GetTempIntersections())
            {
                isect->Render(renderer, isPermitCamera);
            }
        }
    }
}

void ObjCol::ClearIntersection()
{
    isects_.clear();
}

void ObjCol::ClearOldTempIntersection()
{
    oldTempIsects_.clear();
    std::swap(tempIsects_, oldTempIsects_);
}

bool ObjCol::IsIntersected(const std::shared_ptr<ObjCol>& col) const
{
    for (const auto& isect1 : GetIntersections())
    {
        for (const auto& isect2 : col->GetIntersections())
        {
            if (isect1->IsIntersected(isect2)) return true;
        }

        for (const auto& isect2 : col->GetTempIntersections())
        {
            if (isect1->IsIntersected(isect2)) return true;
        }
    }
    for (const auto& isect1 : GetTempIntersections())
    {
        for (const auto& isect2 : col->GetIntersections())
        {
            if (isect1->IsIntersected(isect2)) return true;
        }

        for (const auto& isect2 : col->GetTempIntersections())
        {
            if (isect1->IsIntersected(isect2)) return true;
        }
    }
    return false;
}

bool ObjCol::IsIntersected() const
{
    return GetIntersectedCount() != 0;
}

int ObjCol::GetIntersectedCount() const
{
    int cnt = 0;
    for (const auto& isect : isects_)
    {
        cnt += isect->GetCollideIntersections().size();
    }
    for (const auto& isect : oldTempIsects_)
    {
        cnt += isect->GetCollideIntersections().size();
    }
    return cnt;
}

std::vector<std::weak_ptr<Intersection>> ObjCol::GetCollideIntersections() const
{
    // TODO: コピーのコストが勿体無いのでTempは別の関数にする
    std::vector<std::weak_ptr<Intersection>> ret;
    for (const auto& isect : GetIntersections())
    {
        const auto& tmp = isect->GetCollideIntersections();
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(ret));
    }
    for (const auto& isect : GetTempIntersections())
    {
        const auto& tmp = isect->GetCollideIntersections();
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(ret));
    }
    return ret;
}
}
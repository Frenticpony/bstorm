#pragma once

#include <memory>

namespace bstorm
{
class Stage;
class ObjStage
{
    ObjStage(const std::shared_ptr<Stage>& stage) : stage_(stage) {}
protected:
    const std::weak_ptr<Stage>& GetStage() const { return stage_; }
private:
    std::weak_ptr<Stage> stage_;
};
}

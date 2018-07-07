#pragma once

#include <list>
#include <memory>

namespace bstorm
{
class LostableGraphicResource
{
public:
    virtual ~LostableGraphicResource() {};
    virtual void OnLostDevice() = 0;
    virtual void OnResetDevice() = 0;
};

class LostableGraphicResourceManager
{
public:
    void AddResource(const std::shared_ptr<LostableGraphicResource>& resource);
    void OnLostDeviceAll();
    void OnResetDeviceAll();
    void RemoveUnusedResource();
private:
    std::list<std::weak_ptr<LostableGraphicResource>> resources_;
};
}
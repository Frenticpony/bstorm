#pragma once

#include <unordered_map>
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
    void ReleaseUnusedResource();
private:
    std::unordered_map<LostableGraphicResource*, std::weak_ptr<LostableGraphicResource>> resourceMap_;
};
}
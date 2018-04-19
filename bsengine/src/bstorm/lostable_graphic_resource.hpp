#pragma once

#include <unordered_map>
#include <memory>

namespace bstorm
{
class LostableGraphicResource
{
public:
    virtual ~LostableGraphicResource() {};
    virtual void onLostDevice() = 0;
    virtual void onResetDevice() = 0;
};

class LostableGraphicResourceManager
{
public:
    void addResource(const std::shared_ptr<LostableGraphicResource>& resource);
    void onLostDeviceAll();
    void onResetDeviceAll();
    void releaseUnusedResource();
private:
    std::unordered_map<LostableGraphicResource*, std::weak_ptr<LostableGraphicResource>> resourceMap;
};
}
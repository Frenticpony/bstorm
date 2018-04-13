#include <bstorm/lostable_graphic_resource.hpp>

namespace bstorm
{
void LostableGraphicResourceManager::addResource(const std::shared_ptr<LostableGraphicResource>& resource)
{
    resourceMap[resource.get()] = resource;
}

void LostableGraphicResourceManager::onLostDeviceAll()
{
    for (auto& entry : resourceMap)
    {
        if (auto resource = entry.second.lock())
        {
            resource->onLostDevice();
        }
    }
}

void LostableGraphicResourceManager::onResetDeviceAll()
{
    for (auto& entry : resourceMap)
    {
        if (auto resource = entry.second.lock())
        {
            resource->onResetDevice();
        }
    }
}

void LostableGraphicResourceManager::releaseUnusedResource()
{
    auto it = resourceMap.begin();
    while (it != resourceMap.end())
    {
        if (it->second.expired())
        {
            resourceMap.erase(it++);
        } else ++it;
    }
}
}
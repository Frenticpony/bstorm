#include <bstorm/lostable_graphic_resource.hpp>

namespace bstorm
{
void LostableGraphicResourceManager::AddResource(const std::shared_ptr<LostableGraphicResource>& resource)
{
    resourceMap_[resource.get()] = resource;
}

void LostableGraphicResourceManager::OnLostDeviceAll()
{
    for (auto& entry : resourceMap_)
    {
        if (auto resource = entry.second.lock())
        {
            resource->OnLostDevice();
        }
    }
}

void LostableGraphicResourceManager::OnResetDeviceAll()
{
    for (auto& entry : resourceMap_)
    {
        if (auto resource = entry.second.lock())
        {
            resource->OnResetDevice();
        }
    }
}

void LostableGraphicResourceManager::ReleaseUnusedResource()
{
    auto it = resourceMap_.begin();
    while (it != resourceMap_.end())
    {
        if (it->second.expired())
        {
            resourceMap_.erase(it++);
        } else ++it;
    }
}
}
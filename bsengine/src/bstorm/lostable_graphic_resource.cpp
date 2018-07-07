#include <bstorm/lostable_graphic_resource.hpp>

namespace bstorm
{
void LostableGraphicResourceManager::AddResource(const std::shared_ptr<LostableGraphicResource>& resource)
{
    resources_.push_back(resource);
}

void LostableGraphicResourceManager::OnLostDeviceAll()
{
    for (auto& resourcePtr : resources_)
    {
        if (auto resource = resourcePtr.lock())
        {
            resource->OnLostDevice();
        }
    }
}

void LostableGraphicResourceManager::OnResetDeviceAll()
{
    for (auto& resourcePtr : resources_)
    {
        if (auto resource = resourcePtr.lock())
        {
            resource->OnResetDevice();
        }
    }
}

void LostableGraphicResourceManager::RemoveUnusedResource()
{
    auto it = resources_.begin();
    while (it != resources_.end())
    {
        if (it->expired())
        {
            it = resources_.erase(it);
        } else ++it;
    }
}
}
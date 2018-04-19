#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/lostable_graphic_resource.hpp>

#include <string>
#include <memory>
#include <d3dx9.h>

namespace bstorm
{
class Shader : private NonCopyable, public LostableGraphicResource
{
public:
    Shader(const std::wstring& path, bool precompiled, IDirect3DDevice9* d3DDevice);
    ~Shader();
    void onLostDevice() override;
    void onResetDevice() override;
    void setTexture(const std::string& name, IDirect3DTexture9* texture);
    ID3DXEffect* getEffect() const;
private:
    ID3DXEffect * effect;
};
}
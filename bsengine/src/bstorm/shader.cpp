#include <bstorm/shader.hpp>

#include <bstorm/ptr_util.hpp>
#include <bstorm/logger.hpp>

namespace bstorm
{
Shader::Shader(const std::wstring& path, bool precompiled, IDirect3DDevice9* d3DDevice_) :
    effect_(nullptr)
{
    ID3DXBuffer* buf = nullptr;
    if (FAILED(D3DXCreateEffectFromFile(d3DDevice_, path.c_str(), nullptr, nullptr, precompiled ? D3DXSHADER_SKIPVALIDATION : 0, nullptr, &(this->effect_), &buf)))
    {
        buf->Release();
        throw Log(LogLevel::LV_ERROR)
            .Msg(((char*)(buf->GetBufferPointer())))
            .Param(LogParam(LogParam::Tag::SHADER, path));
    }
    safe_release(buf);
}

Shader::~Shader()
{
    effect_->Release();
}

void Shader::OnLostDevice()
{
    if (FAILED(effect_->OnLostDevice()))
    {
        throw Log(LogLevel::LV_ERROR).Msg("failed to release shader.");
    }
}

void Shader::OnResetDevice()
{
    if (FAILED(effect_->OnResetDevice()))
    {
        throw Log(LogLevel::LV_ERROR).Msg("failed to reset shader.");
    }
}

void Shader::SetTexture(const std::string & name, IDirect3DTexture9* texture)
{
    effect_->SetTexture(name.c_str(), texture);
}

ID3DXEffect* Shader::getEffect() const
{
    return effect_;
}
}
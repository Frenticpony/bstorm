#include <exception>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/shader.hpp>

namespace bstorm {
  Shader::Shader(const std::wstring& path, bool precompiled, IDirect3DDevice9* d3DDevice) :
    effect(NULL)
  {
    ID3DXBuffer* buf = NULL;
    if (FAILED(D3DXCreateEffectFromFile(d3DDevice, path.c_str(), NULL, NULL, precompiled ? D3DXSHADER_SKIPVALIDATION : 0, NULL, &(this->effect), &buf))) {
      buf->Release();
      throw Log(Log::Level::LV_ERROR)
        .setMessage(((char*)(buf->GetBufferPointer())))
        .setParam(Log::Param(Log::Param::Tag::SHADER, path));
    }
    safe_release(buf);
  }

  Shader::~Shader() {
    effect->Release();
  }

  void Shader::onLostDevice() {
    if (FAILED(effect->OnLostDevice())) {
      throw Log(Log::Level::LV_ERROR).setMessage("failed to release shader.");
    }
  }

  void Shader::onResetDevice() {
    if (FAILED(effect->OnResetDevice())) {
      throw Log(Log::Level::LV_ERROR).setMessage("failed to reset shader.");
    }
  }

  void Shader::setTexture(const std::string & name, IDirect3DTexture9* texture) {
    effect->SetTexture(name.c_str(), texture);
  }

  ID3DXEffect* Shader::getEffect() const {
    return effect;
  }
}
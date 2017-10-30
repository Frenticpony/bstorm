#include <exception>

#include <bstorm/util.hpp>
#include <bstorm/shader.hpp>

namespace bstorm {
  Shader::Shader(const std::wstring& path, bool precompiled, IDirect3DDevice9* d3DDevice) :
    effect(NULL)
  {
    ID3DXBuffer* buf = NULL;
    if (FAILED(D3DXCreateEffectFromFile(d3DDevice, path.c_str(), NULL, NULL, precompiled ? D3DXSHADER_SKIPVALIDATION : 0, NULL, &(this->effect), &buf))) {
      std::string errMsg((char*)(buf->GetBufferPointer()));
      buf->Release();
      throw std::runtime_error(errMsg);
    }
    safe_release(buf);
  }

  Shader::~Shader() {
    effect->Release();
  }

  void Shader::onLostDevice() {
    if (FAILED(effect->OnLostDevice())) {
      throw std::runtime_error("failed to release shader.");
    }
  }

  void Shader::onResetDevice() {
    if (FAILED(effect->OnResetDevice())) {
      throw std::runtime_error("failed to reset shader.");
    }
  }

  void Shader::setTexture(const std::string & name, IDirect3DTexture9* texture) {
    effect->SetTexture(name.c_str(), texture);
  }

  ID3DXEffect* Shader::getEffect() const {
    return effect;
  }
}
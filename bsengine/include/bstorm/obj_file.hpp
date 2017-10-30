#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <bstorm/obj.hpp>

namespace bstorm {
  class ObjFile : public Obj {
  public:
    ObjFile(const std::shared_ptr<GameState>& state);
    ~ObjFile();
    virtual bool open(const std::wstring& path) = 0;
    virtual bool openNW(const std::wstring& path) = 0;
    virtual void store() = 0;
    int getSize();
  protected:
    std::fstream file;
  };

  class ObjFileT : public ObjFile {
  public:
    ObjFileT(const std::shared_ptr<GameState>& state);
    void update() override;
    bool open(const std::wstring& path);
    bool openNW(const std::wstring& path);
    void store();
    int getLineCount() const;
    std::wstring getLineText(int lineNum) const;
    void addLine(const std::wstring& str);
    void clearLine();
    std::vector<std::wstring> splitLineText(int lineNum, const std::wstring& delimiter) const;
  protected:
    std::vector<std::wstring> lines;
  };

  class ObjFileB : public ObjFile {
  public:
    enum class Encoding {
      ACP,
      UTF8,
      UTF16LE,
      UTF16BE
    };
    ObjFileB(const std::shared_ptr<GameState>& state);
    void update() override;
    bool open(const std::wstring& path);
    bool openNW(const std::wstring& path) { return false; }
    void store() {};
    void setByteOrder(bool useBE);
    void setCharacterCode(Encoding code);
    int getPointer();
    void seek(int position);
    bool readBoolean();
    int8_t readByte();
    int16_t readShort();
    int32_t readInteger();
    int64_t readLong();
    float readFloat();
    double readDouble();
    std::wstring readString(int size);
  protected:
    Encoding code;
    bool useBE;
  };
}

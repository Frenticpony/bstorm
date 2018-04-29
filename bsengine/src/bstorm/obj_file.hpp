#pragma once

#include <bstorm/obj.hpp>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace bstorm
{
class ObjFile : public Obj
{
public:
    ObjFile(const std::shared_ptr<GameState>& state);
    ~ObjFile();
    virtual bool Open(const std::wstring& path) = 0;
    virtual bool OpenNW(const std::wstring& path) = 0;
    virtual void Store() = 0;
    int getSize();
protected:
    std::fstream file_;
};

class ObjFileT : public ObjFile
{
public:
    ObjFileT(const std::shared_ptr<GameState>& state);
    void Update() override;
    bool Open(const std::wstring& path);
    bool OpenNW(const std::wstring& path);
    void Store() override;
    int GetLineCount() const;
    std::wstring GetLineText(int lineNum) const;
    void AddLine(const std::wstring& str);
    void ClearLine();
    std::vector<std::wstring> SplitLineText(int lineNum, const std::wstring& delimiter) const;
private:
    std::vector<std::wstring> lines_;
};

class ObjFileB : public ObjFile
{
public:
    enum class Encoding
    {
        ACP,
        UTF8,
        UTF16LE,
        UTF16BE
    };
    ObjFileB(const std::shared_ptr<GameState>& state);
    void Update() override;
    bool Open(const std::wstring& path);
    bool OpenNW(const std::wstring& path) { return false; }
    void Store() override {};
    void SetByteOrder(bool useBE);
    void SetCharacterCode(Encoding code);
    int GetPointer();
    void Seek(int position);
    bool ReadBoolean();
    int8_t ReadByte();
    int16_t ReadShort();
    int32_t ReadInteger();
    int64_t ReadLong();
    float ReadFloat();
    double ReadDouble();
    std::wstring ReadString(int size);
private:
    Encoding code_;
    bool useBE_;
};
}

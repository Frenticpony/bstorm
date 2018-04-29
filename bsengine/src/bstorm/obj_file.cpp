#include <bstorm/obj_file.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>

namespace bstorm
{
ObjFile::ObjFile(const std::shared_ptr<GameState>& state) : Obj(state)
{
}

ObjFile::~ObjFile()
{
    file_.close();
}

int ObjFile::getSize()
{
    // 現在位置を保存
    auto pos = file_.tellg();
    int size = file_.seekg(0, file_.end).tellg();
    // 位置の復元
    file_.seekg(pos);
    return size;
}


ObjFileT::ObjFileT(const std::shared_ptr<GameState>& state) : ObjFile(state)
{
    SetType(OBJ_FILE_TEXT);
}

void ObjFileT::Update() {}

bool ObjFileT::Open(const std::wstring & path)
{
    file_.close();
    file_.open(path, std::ios::in);
    if (!file_.is_open()) return false;

    // 行の読み込み
    lines_.clear();
    std::streambuf* sb = file_.rdbuf();
    std::string line; line.reserve(1024);
    while (true)
    {
        int c = sb->sbumpc();
        switch (c)
        {
            case '\n':
                lines_.push_back(FromMultiByte<932>(line));
                line.clear();
                break;
            case '\r':
                if (sb->sgetc() == '\n')
                {
                    sb->sbumpc();
                    lines_.push_back(FromMultiByte<932>(line));
                    line.clear();
                } else
                {
                    line += '\r';
                }
                break;
            case EOF:
                lines_.push_back(FromMultiByte<932>(line));
                goto load_end;
            default:
                line += (char)c;
        }
    }
load_end:
    if (file_.bad() || file_.fail())
    {
        lines_.clear();
        return false;
    }
    file_.seekg(0, file_.beg);
    return true;
}

bool ObjFileT::OpenNW(const std::wstring& path)
{
    file_.close();
    MakeDirectoryP(GetParentPath(path));
    file_.open(path, std::ios::out);
    return file_.good();
}

void ObjFileT::Store()
{
    for (int i = 0; i < lines_.size(); i++)
    {
        std::string sjisLine = ToMultiByte<932>(lines_[i]);
        if (i != 0) file_.put('\n'); // CRLFになる
        file_.write(sjisLine.c_str(), sjisLine.size());
    }
}

int ObjFileT::GetLineCount() const
{
    return lines_.size();
}

std::wstring ObjFileT::GetLineText(int lineNum) const
{
    if (lineNum < 1 || lineNum > lines_.size()) return L"";
    return lines_[lineNum - 1];
}

void ObjFileT::AddLine(const std::wstring & str)
{
    lines_.push_back(str);
}

void ObjFileT::ClearLine()
{
    lines_.clear();
}

std::vector<std::wstring> ObjFileT::SplitLineText(int lineNum, const std::wstring & delimiter) const
{
    if (lineNum < 1 || lineNum > lines_.size()) return std::vector<std::wstring>();
    return Split(lines_[lineNum - 1], delimiter);
}

ObjFileB::ObjFileB(const std::shared_ptr<GameState>& state) :
    ObjFile(state),
    code_(Encoding::ACP),
    useBE_(false)
{
    SetType(OBJ_FILE_BINARY);
}

void ObjFileB::Update() {}

bool ObjFileB::Open(const std::wstring & path)
{
    file_.close();
    file_.open(path, std::ios::in | std::ios::binary);
    return file_.is_open();
}

void ObjFileB::SetByteOrder(bool useBE)
{
    this->useBE_ = useBE;
}

void ObjFileB::SetCharacterCode(Encoding code)
{
    this->code_ = code;
}

int ObjFileB::GetPointer()
{
    return (int)file_.tellg();
}

void ObjFileB::Seek(int pos)
{
    file_.seekg(pos);
}

bool ObjFileB::ReadBoolean()
{
    bool b = false;
    file_.read((char*)&b, sizeof(b));
    return b;
}

int8_t ObjFileB::ReadByte()
{
    int8_t byte = 0;
    file_.read((char*)&byte, sizeof(byte));
    return byte;
}

int16_t ObjFileB::ReadShort()
{
    uint8_t bytes[2];
    file_.read((char*)&bytes, sizeof(bytes));
    if (useBE_)
    {
        std::swap(bytes[0], bytes[1]);
    }
    return *((int16_t*)bytes);
}

int32_t ObjFileB::ReadInteger()
{
    uint8_t bytes[4];
    file_.read((char*)&bytes, sizeof(bytes));
    if (useBE_)
    {
        std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((int32_t*)bytes);
}

int64_t ObjFileB::ReadLong()
{
    uint8_t bytes[8];
    file_.read((char*)&bytes, sizeof(bytes));
    if (useBE_)
    {
        std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((int64_t*)bytes);
}

float ObjFileB::ReadFloat()
{
    uint8_t bytes[4];
    file_.read((char*)&bytes, sizeof(bytes));
    if (useBE_)
    {
        std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((float*)bytes);
}

double ObjFileB::ReadDouble()
{
    uint8_t bytes[8];
    file_.read((char*)&bytes, sizeof(bytes));
    if (useBE_)
    {
        std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((double*)bytes);
}

std::wstring ObjFileB::ReadString(int size)
{
    if (size < 0) return L"";
    int pos = file_.tellg();
    std::wstring ret;
    if (code_ == Encoding::UTF16LE || code_ == Encoding::UTF16BE)
    {
        int cnt = size / 2;
        for (int i = 0; i < cnt; i++)
        {
            uint8_t buf[2];
            file_.read((char*)&buf, sizeof(buf));
            if (code_ == Encoding::UTF16BE) std::swap(buf[0], buf[1]);
            ret += *((wchar_t*)buf);
        }
    } else
    {
        std::string buf(size, '\0');
        file_.read(&buf[0], size);
        if (code_ == Encoding::UTF8)
        {
            ret = ToUnicode(buf);
        } else
        {
            ret = FromMultiByte<CP_ACP>(buf);
        }
    }
    file_.seekg(pos + size);
    return ret;
}
}
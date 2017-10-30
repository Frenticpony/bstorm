#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/obj_file.hpp>

namespace bstorm {
  ObjFile::ObjFile(const std::shared_ptr<GameState>& state) : Obj(state)
  {
  }

  ObjFile::~ObjFile() {
    file.close();
  }

  int ObjFile::getSize() {
    // 現在位置を保存
    auto pos = file.tellg();
    int size = file.seekg(0, file.end).tellg();
    // 位置の復元
    file.seekg(pos);
    return size;
  }


  ObjFileT::ObjFileT(const std::shared_ptr<GameState>& state) : ObjFile(state) {
    setType(OBJ_FILE_TEXT);
  }

  void ObjFileT::update() { }

  bool ObjFileT::open(const std::wstring & path) {
    file.close();
    file.open(path, std::ios::in);
    if (!file.is_open()) return false;

    // 行の読み込み
    lines.clear();
    std::streambuf* sb = file.rdbuf();
    std::string line; line.reserve(1024);
    while (true) {
      int c = sb->sbumpc();
      switch (c) {
        case '\n':
          lines.push_back(fromMultiByte<932>(line));
          line.clear();
          break;
        case '\r':
          if (sb->sgetc() == '\n') {
            sb->sbumpc();
            lines.push_back(fromMultiByte<932>(line));
            line.clear();
          } else {
            line += '\r';
          }
          break;
        case EOF:
          lines.push_back(fromMultiByte<932>(line));
          goto load_end;
        default:
          line += (char)c;
      }
    }
load_end:
    if (file.bad() || file.fail()) {
      lines.clear();
      return false;
    }
    file.seekg(0, file.beg);
    return true;
  }

  bool ObjFileT::openNW(const std::wstring& path) {
    file.close();
    mkdir_p(parentPath(path));
    file.open(path, std::ios::out);
    return file.good();
  }

  void ObjFileT::store() {
    for (int i = 0; i < lines.size(); i++) {
      std::string sjisLine = toMultiByte<932>(lines[i]);
      if (i != 0) file.put('\n'); // CRLFになる
      file.write(sjisLine.c_str(), sjisLine.size());
    }
  }

  int ObjFileT::getLineCount() const {
    return lines.size();
  }

  std::wstring ObjFileT::getLineText(int lineNum) const {
    if (lineNum < 1 || lineNum > lines.size()) return L"";
    return lines[lineNum - 1];
  }

  void ObjFileT::addLine(const std::wstring & str) {
    lines.push_back(str);
  }

  void ObjFileT::clearLine() {
    lines.clear();
  }

  std::vector<std::wstring> ObjFileT::splitLineText(int lineNum, const std::wstring & delimiter) const {
    if (lineNum < 1 || lineNum > lines.size()) return std::vector<std::wstring>();
    return split(lines[lineNum - 1], delimiter);
  }

  ObjFileB::ObjFileB(const std::shared_ptr<GameState>& state) :
    ObjFile(state),
    code(Encoding::ACP),
    useBE(false)
  {
    setType(OBJ_FILE_BINARY);
  }

  void ObjFileB::update() { }

  bool ObjFileB::open(const std::wstring & path) {
    file.close();
    file.open(path, std::ios::in | std::ios::binary);
    return file.is_open();
  }

  void ObjFileB::setByteOrder(bool useBE) {
    this->useBE = useBE;
  }

  void ObjFileB::setCharacterCode(Encoding code) {
    this->code = code;
  }

  int ObjFileB::getPointer() {
    return (int)file.tellg();
  }

  void ObjFileB::seek(int pos) {
    file.seekg(pos);
  }

  bool ObjFileB::readBoolean() {
    bool b = false;
    file.read((char*)&b, sizeof(b));
    return b;
  }

  int8_t ObjFileB::readByte() {
    int8_t byte = 0;
    file.read((char*)&byte, sizeof(byte));
    return byte;
  }

  int16_t ObjFileB::readShort() {
    uint8_t bytes[2];
    file.read((char*)&bytes, sizeof(bytes));
    if (useBE) {
      std::swap(bytes[0], bytes[1]);
    }
    return *((int16_t*)bytes);
  }

  int32_t ObjFileB::readInteger() {
    uint8_t bytes[4];
    file.read((char*)&bytes, sizeof(bytes));
    if (useBE) {
      std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((int32_t*)bytes);
  }

  int64_t ObjFileB::readLong() {
    uint8_t bytes[8];
    file.read((char*)&bytes, sizeof(bytes));
    if (useBE) {
      std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((int64_t*)bytes);
  }

  float ObjFileB::readFloat() {
    uint8_t bytes[4];
    file.read((char*)&bytes, sizeof(bytes));
    if (useBE) {
      std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((float*)bytes);
  }

  double ObjFileB::readDouble() {
    uint8_t bytes[8];
    file.read((char*)&bytes, sizeof(bytes));
    if (useBE) {
      std::reverse(bytes, bytes + sizeof(bytes));
    }
    return *((double*)bytes);
  }

  std::wstring ObjFileB::readString(int size) {
    if (size < 0) return L"";
    int pos = file.tellg();
    std::wstring ret;
    if (code == Encoding::UTF16LE || code == Encoding::UTF16BE) {
      int cnt = size / 2;
      for (int i = 0; i < cnt; i++) {
        uint8_t buf[2];
        file.read((char*)&buf, sizeof(buf));
        if (code == Encoding::UTF16BE) std::swap(buf[0], buf[1]);
        ret += *((wchar_t*)buf);
      }
    } else {
      std::string buf(size, '\0');
      file.read(&buf[0], size);
      if (code == Encoding::UTF8) {
        ret = toUnicode(buf);
      } else {
        ret = fromMultiByte<CP_ACP>(buf);
      }
    }
    file.seekg(pos + size);
    return ret;
  }
}
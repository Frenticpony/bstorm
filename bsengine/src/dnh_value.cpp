#include <bstorm/util.hpp>
#include <bstorm/dnh_value.hpp>

namespace bstorm {
  std::unique_ptr<DnhValue> DnhValue::get(lua_State* L, int idx) {
    switch (lua_type(L, idx)) {
      case LUA_TNUMBER:
        return std::make_unique<DnhReal>((double)lua_tonumber(L, idx));
      case LUA_TSTRING:
      {
        std::wstring wstr = toUnicode(lua_tostring(L, idx));
        return std::make_unique<DnhChar>(wstr.empty() ? L'\0' : wstr[0]);
      }
      case LUA_TBOOLEAN:
        return std::make_unique<DnhBool>((bool)lua_toboolean(L, idx));
      case LUA_TTABLE:
      {
        size_t size = lua_objlen(L, idx);
        std::unique_ptr<DnhArray> arr = std::make_unique<DnhArray>();
        for (int i = 1; i <= size; i++) {
          lua_rawgeti(L, idx, i);
          arr->pushBack(DnhValue::get(L, -1));
          lua_pop(L, 1);
        }
        return std::move(arr);
      }
      default:
        return std::make_unique<DnhNil>();
    }
  }

  std::unique_ptr<DnhValue> DnhValue::deserialize(std::istream& in) {
    uint32_t header;
    in.read((char*)&header, sizeof(header));
    switch ((Type)header) {
      case Type::REAL:
      {
        double n;
        in.read((char*)&n, sizeof(n));
        return std::make_unique<DnhReal>(n);
      }
      case Type::CHAR:
      {
        wchar_t c;
        in.read((char*)&c, sizeof(c));
        return std::make_unique<DnhChar>(c);
      }
      case Type::BOOL:
      {
        bool b;
        in.read((char*)&b, sizeof(b));
        return std::make_unique<DnhBool>(b);
      }
      case Type::ARRAY:
      {
        auto arr = std::make_unique<DnhArray>();
        uint32_t length;
        in.read((char*)&length, sizeof(length));
        for (int i = 0; i < length; i++) {
          arr->pushBack(DnhValue::deserialize(in));
        }
        return std::move(arr);
      }
      default:
        return std::make_unique<DnhNil>();
    }
  }
  double DnhValue::toNum(lua_State* L, int idx) {
    switch (lua_type(L, idx)) {
      case LUA_TNUMBER:
        return (double)lua_tonumber(L, idx);
      case LUA_TSTRING:
      {
        std::wstring wstr = toUnicode(lua_tostring(L, idx));
        return (double)(wstr.empty() ? 0 : wstr[0]);
      }
      case LUA_TBOOLEAN:
        return (double)lua_toboolean(L, idx);
      case LUA_TTABLE:
        return _wtof(DnhValue::toString(L, idx).c_str());
      default:
        return 0;
    }
  }

  bool DnhValue::toBool(lua_State* L, int idx) {
    switch (lua_type(L, idx)) {
      case LUA_TNUMBER:
        return lua_tonumber(L, idx) != 0;
      case LUA_TSTRING:
      {
        // utf8のまま検査
        std::string str = lua_tostring(L, idx);
        return !str.empty() && str[0] != '\0';
      }
      case LUA_TBOOLEAN:
        return (bool)lua_toboolean(L, idx);
      case LUA_TTABLE:
        return lua_objlen(L, idx) != 0;
      default:
        return false;
    }
  }

  std::wstring DnhValue::toString(lua_State*L, int idx) {
    return DnhValue::get(L, idx)->toString();
  }

  std::string DnhValue::toStringU8(lua_State * L, int idx) {
    switch (lua_type(L, idx)) {
      case LUA_TNUMBER:
        return std::to_string(lua_tonumber(L, idx));
      case LUA_TSTRING:
        return lua_tostring(L, idx);
      case LUA_TBOOLEAN:
        return lua_toboolean(L, idx) ? "true" : "false";
      case LUA_TTABLE:
      {
        const size_t size = lua_objlen(L, idx);
        if (size == 0) return "";
        lua_rawgeti(L, idx, 1);
        const bool isStr = lua_type(L, -1) == LUA_TSTRING;
        lua_pop(L, 1);
        if (isStr) {
          std::string ret;
          for (int i = 1; i <= size; i++) {
            lua_rawgeti(L, idx, i);
            ret += lua_tostring(L, -1);
            lua_pop(L, 1);
          }
          return ret;
        } else {
          return toUTF8(toString(L, idx));
        }
      }
      case LUA_TNIL:
        return "(VOID)";
      default:
        return "ILLEGAL";
    }
  }

  DnhReal::DnhReal(double num) : value(num) {
    type = Type::REAL;
  }

  double DnhReal::toNum() const { return value; }
  bool DnhReal::toBool() const { return value != 0; }
  std::wstring DnhReal::toString() const { return std::to_wstring(value); }
  void DnhReal::push(lua_State * L) const { lua_pushnumber(L, value); }
  void DnhReal::serialize(std::ostream & out) const {
    uint32_t header = (uint32_t)getType();
    out.write((char*)&header, sizeof(header));
    out.write((char*)&value, sizeof(value));
  }
  std::unique_ptr<DnhValue> DnhReal::clone() const {
    return std::make_unique<DnhReal>(value);
  }

  DnhChar::DnhChar(wchar_t c) : value(c) {
    type = Type::CHAR;
  }
  double DnhChar::toNum() const { return (double)value; }
  bool DnhChar::toBool() const { return value != L'\0'; }
  std::wstring DnhChar::toString() const { return std::wstring{ value }; }
  void DnhChar::push(lua_State * L) const { lua_pushstring(L, toUTF8(std::wstring{ value }).c_str()); }
  void DnhChar::serialize(std::ostream & out) const {
    uint32_t header = (uint32_t)getType();
    out.write((char*)&header, sizeof(header));
    out.write((char*)&value, sizeof(value));
  }
  std::unique_ptr<DnhValue> DnhChar::clone() const {
    return std::make_unique<DnhChar>(value);
  }

  DnhBool::DnhBool(bool b) : value(b) {
    type = Type::BOOL;
  }
  double DnhBool::toNum() const { return (double)value; }
  bool DnhBool::toBool() const { return value; }
  std::wstring DnhBool::toString() const { return value ? L"true" : L"false"; }
  void DnhBool::push(lua_State * L) const { lua_pushboolean(L, (int)value); }
  void DnhBool::serialize(std::ostream & out) const {
    uint32_t header = (uint32_t)getType();
    out.write((char*)&header, sizeof(header));
    out.write((char*)&value, sizeof(value));
  }
  std::unique_ptr<DnhValue> DnhBool::clone() const {
    return std::make_unique<DnhBool>(value);
  }

  DnhArray::DnhArray() {
    type = Type::ARRAY;
  }
  DnhArray::DnhArray(std::vector<std::unique_ptr<DnhValue>>&& a) :
    values(std::move(a))
  {
    type = Type::ARRAY;
  }
  DnhArray::DnhArray(const std::wstring & s) {
    for (wchar_t c : s) {
      values.push_back(std::make_unique<DnhChar>(c));
    }
    type = Type::ARRAY;
  }
  DnhArray::DnhArray(const std::vector<double>& ns) {
    for (double n : ns) {
      pushBack(std::make_unique<DnhReal>(n));
    }
    type = Type::ARRAY;
  }
  DnhArray::DnhArray(const Point2D & p) {
    pushBack(std::make_unique<DnhReal>(p.x));
    pushBack(std::make_unique<DnhReal>(p.y));
    type = Type::ARRAY;
  }
  DnhArray::DnhArray(const std::vector<Point2D>& ps) {
    for (const auto& p : ps) {
      pushBack(std::make_unique<DnhArray>(p));
    }
    type = Type::ARRAY;
  }
  size_t DnhArray::getSize() const { return values.size(); }
  void DnhArray::pushBack(std::unique_ptr<DnhValue>&& v) {
    values.push_back(std::move(v));
  }
  double DnhArray::toNum() const { return _wtof(toString().c_str()); }
  bool DnhArray::toBool() const { return getSize() != 0; }
  std::wstring DnhArray::toString() const {
   // 空         =>
   // 文字列     => abc
   // それ以外   => [1, 2, 3]
    size_t size = getSize();
    if (size == 0) return L"";
    std::wstring result;
    bool notStr = values[0]->getType() != Type::CHAR;
    if (notStr) result += L"[";
    for (size_t i = 0; i < size; i++) {
      if (notStr && i != 0) result += L",";
      result += values[i]->toString();
    }
    if (notStr) result += L"]";
    return result;
  }
  std::unique_ptr<DnhValue> DnhArray::index(int idx) const {
    if (idx < 0 || idx >= getSize()) {
      return std::unique_ptr<DnhValue>();
    }
    return values[idx]->clone();
  }
  void DnhArray::push(lua_State * L) const {
    size_t size = getSize();
    lua_createtable(L, size, 0);
    for (size_t i = 0; i < size; i++) {
      values[i]->push(L);
      lua_rawseti(L, -2, i + 1);
    }
  }
  void DnhArray::serialize(std::ostream & out) const {
    uint32_t header = (uint32_t)getType();
    out.write((char*)&header, sizeof(header));
    uint32_t length = getSize();
    out.write((char*)&length, sizeof(length));
    for (auto i = 0; i < length; i++) {
      values[i]->serialize(out);
    }
  }
  std::unique_ptr<DnhValue> DnhArray::clone() const {
    std::unique_ptr<DnhArray> a = std::make_unique<DnhArray>();
    for (const auto& v : values) a->pushBack(v->clone());
    return std::move(a);
  }

  DnhNil::DnhNil() {
    type = Type::NIL;
  }

  double DnhNil::toNum() const { return 0; }
  bool DnhNil::toBool() const { return false; }
  std::wstring DnhNil::toString() const { return L"(VOID)"; }
  void DnhNil::push(lua_State * L) const { lua_pushnil(L); }
  void DnhNil::serialize(std::ostream & out) const {
    uint32_t header = (uint32_t)getType();
    out.write((char*)&header, sizeof(header));
  }
  std::unique_ptr<DnhValue> DnhNil::clone() const {
    return std::make_unique<DnhNil>();
  }
}

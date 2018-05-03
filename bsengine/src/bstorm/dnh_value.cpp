#include <bstorm/dnh_value.hpp>

#include <bstorm/util.hpp>

namespace bstorm
{
std::unique_ptr<DnhValue> DnhValue::Get(lua_State* L, int idx)
{
    switch (lua_type(L, idx))
    {
        case LUA_TNUMBER:
            return std::make_unique<DnhReal>((double)lua_tonumber(L, idx));
        case LUA_TSTRING:
        {
            std::wstring wstr = ToUnicode(lua_tostring(L, idx));
            return std::make_unique<DnhChar>(wstr.empty() ? L'\0' : wstr[0]);
        }
        case LUA_TBOOLEAN:
            return std::make_unique<DnhBool>((bool)lua_toboolean(L, idx));
        case LUA_TTABLE:
        {
            size_t size = lua_objlen(L, idx);
            std::unique_ptr<DnhArray> arr = std::make_unique<DnhArray>(size);
            for (int i = 1; i <= size; i++)
            {
                lua_rawgeti(L, idx, i);
                arr->PushBack(DnhValue::Get(L, -1));
                lua_pop(L, 1);
            }
            return std::move(arr);
        }
        default:
            return std::make_unique<DnhNil>();
    }
}

std::unique_ptr<DnhValue> DnhValue::Deserialize(std::istream& in)
{
    uint32_t header;
    in.read((char*)&header, sizeof(header));
    switch ((Type)header)
    {
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
            uint32_t length;
            in.read((char*)&length, sizeof(length));
            auto arr = std::make_unique<DnhArray>((size_t)(length));
            for (int i = 0; i < length; i++)
            {
                arr->PushBack(DnhValue::Deserialize(in));
            }
            return std::move(arr);
        }
        default:
            return std::make_unique<DnhNil>();
    }
}
double DnhValue::ToNum(lua_State* L, int idx)
{
    switch (lua_type(L, idx))
    {
        case LUA_TNUMBER:
            return (double)lua_tonumber(L, idx);
        case LUA_TSTRING:
        {
            std::wstring wstr = ToUnicode(lua_tostring(L, idx));
            return (double)(wstr.empty() ? 0 : wstr[0]);
        }
        case LUA_TBOOLEAN:
            return (double)lua_toboolean(L, idx);
        case LUA_TTABLE:
            return _wtof(DnhValue::ToString(L, idx).c_str());
        default:
            return 0;
    }
}

bool DnhValue::ToBool(lua_State* L, int idx)
{
    switch (lua_type(L, idx))
    {
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

std::wstring DnhValue::ToString(lua_State*L, int idx)
{
    return DnhValue::Get(L, idx)->ToString();
}

std::string DnhValue::ToStringU8(lua_State * L, int idx)
{
    switch (lua_type(L, idx))
    {
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
            if (isStr)
            {
                std::string ret;
                for (int i = 1; i <= size; i++)
                {
                    lua_rawgeti(L, idx, i);
                    ret += lua_tostring(L, -1);
                    lua_pop(L, 1);
                }
                return ret;
            } else
            {
                return ToUTF8(ToString(L, idx));
            }
        }
        case LUA_TNIL:
            return "(VOID)";
        default:
            return "ILLEGAL";
    }
}

const std::unique_ptr<DnhValue>& DnhValue::Nil()
{
    static std::unique_ptr<DnhValue> nil = std::make_unique<DnhNil>();
    return nil;
}

DnhReal::DnhReal(double num) :
    DnhValue(Type::REAL),
    value_(num)

{
}

double DnhReal::ToNum() const { return value_; }

bool DnhReal::ToBool() const { return value_ != 0; }

std::wstring DnhReal::ToString() const { return std::to_wstring(value_); }

void DnhReal::Push(lua_State * L) const { lua_pushnumber(L, value_); }

void DnhReal::Serialize(std::ostream & out) const
{
    uint32_t header = (uint32_t)GetType();
    out.write((char*)&header, sizeof(header));
    out.write((char*)&value_, sizeof(value_));
}

std::unique_ptr<DnhValue> DnhReal::Clone() const
{
    return std::make_unique<DnhReal>(value_);
}

DnhChar::DnhChar(wchar_t c) :
    DnhValue(Type::CHAR),
    value_(c)
{
}

double DnhChar::ToNum() const { return (double)value_; }

bool DnhChar::ToBool() const { return value_ != L'\0'; }

std::wstring DnhChar::ToString() const { return std::wstring{ value_ }; }

void DnhChar::Push(lua_State * L) const { lua_pushstring(L, ToUTF8(std::wstring{ value_ }).c_str()); }

void DnhChar::Serialize(std::ostream & out) const
{
    uint32_t header = (uint32_t)GetType();
    out.write((char*)&header, sizeof(header));
    out.write((char*)&value_, sizeof(value_));
}

std::unique_ptr<DnhValue> DnhChar::Clone() const
{
    return std::make_unique<DnhChar>(value_);
}

DnhBool::DnhBool(bool b) :
    DnhValue(Type::BOOL),
    value_(b)
{
}

double DnhBool::ToNum() const { return (double)value_; }

bool DnhBool::ToBool() const { return value_; }

std::wstring DnhBool::ToString() const { return value_ ? L"true" : L"false"; }

void DnhBool::Push(lua_State * L) const { lua_pushboolean(L, (int)value_); }

void DnhBool::Serialize(std::ostream & out) const
{
    uint32_t header = (uint32_t)GetType();
    out.write((char*)&header, sizeof(header));
    out.write((char*)&value_, sizeof(value_));
}

std::unique_ptr<DnhValue> DnhBool::Clone() const
{
    return std::make_unique<DnhBool>(value_);
}

DnhArray::DnhArray() :
    DnhValue(Type::ARRAY)
{
}

DnhArray::DnhArray(size_t reserveSize) :
    DnhValue(Type::ARRAY)
{
    Reserve(reserveSize);
}

DnhArray::DnhArray(std::vector<std::unique_ptr<DnhValue>>&& a) :
    DnhValue(Type::ARRAY),
    values_(std::move(a))
{
}

DnhArray::DnhArray(const std::wstring & s) :
    DnhValue(Type::ARRAY)
{
    Reserve(s.size());
    for (wchar_t c : s)
    {
        PushBack(std::make_unique<DnhChar>(c));
    }
}

DnhArray::DnhArray(const std::vector<double>& ns) :
    DnhValue(Type::ARRAY)
{
    Reserve(ns.size());
    for (double n : ns)
    {
        PushBack(std::make_unique<DnhReal>(n));
    }
}

DnhArray::DnhArray(const Point2D & p) :
    DnhValue(Type::ARRAY)
{
    PushBack(std::make_unique<DnhReal>(p.x));
    PushBack(std::make_unique<DnhReal>(p.y));
}

DnhArray::DnhArray(const std::vector<Point2D>& ps) :
    DnhValue(Type::ARRAY)
{
    for (const auto& p : ps)
    {
        PushBack(std::make_unique<DnhArray>(p));
    }
}

size_t DnhArray::GetSize() const { return values_.size(); }

void DnhArray::PushBack(std::unique_ptr<DnhValue>&& v)
{
    values_.push_back(std::move(v));
}

double DnhArray::ToNum() const { return _wtof(ToString().c_str()); }

bool DnhArray::ToBool() const { return GetSize() != 0; }

std::wstring DnhArray::ToString() const
{
    // 空         => 空文字列
    // 文字列     => abc
    // それ以外   => [1, 2, 3]
    size_t size = GetSize();
    if (size == 0) return L"";
    std::wstring result;
    bool notStr = values_[0]->GetType() != Type::CHAR;
    if (notStr) result += L"[";
    for (size_t i = 0; i < size; i++)
    {
        if (notStr && i != 0) result += L",";
        result += values_[i]->ToString();
    }
    if (notStr) result += L"]";
    return result;
}

const std::unique_ptr<DnhValue>& DnhArray::Index(int idx) const
{
    if (idx < 0 || idx >= GetSize())
    {
        return DnhValue::Nil();
    }
    return values_[idx];
}

void DnhArray::Push(lua_State * L) const
{
    size_t size = GetSize();
    lua_createtable(L, size, 0);
    for (size_t i = 0; i < size; i++)
    {
        values_[i]->Push(L);
        lua_rawseti(L, -2, i + 1);
    }
}

void DnhArray::Serialize(std::ostream & out) const
{
    uint32_t header = (uint32_t)GetType();
    out.write((char*)&header, sizeof(header));
    uint32_t length = GetSize();
    out.write((char*)&length, sizeof(length));
    for (auto i = 0; i < length; i++)
    {
        values_[i]->Serialize(out);
    }
}

std::unique_ptr<DnhValue> DnhArray::Clone() const
{
    std::unique_ptr<DnhArray> arr = std::make_unique<DnhArray>(GetSize());
    for (const auto& v : values_)
    {
        arr->PushBack(v->Clone());
    }
    return std::move(arr);
}

void DnhArray::Reserve(size_t size)
{
    values_.reserve(size);
}

DnhNil::DnhNil() :
    DnhValue(Type::NIL)
{
}

double DnhNil::ToNum() const { return 0; }

bool DnhNil::ToBool() const { return false; }

std::wstring DnhNil::ToString() const { return L"(VOID)"; }

void DnhNil::Push(lua_State * L) const { lua_pushnil(L); }

void DnhNil::Serialize(std::ostream & out) const
{
    uint32_t header = (uint32_t)GetType();
    out.write((char*)&header, sizeof(header));
}

std::unique_ptr<DnhValue> DnhNil::Clone() const
{
    return std::make_unique<DnhNil>();
}
}

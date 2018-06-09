#pragma once

#include <bstorm/point2D.hpp>

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>
#include <luajit/lua.hpp>

namespace bstorm
{
class DnhValue
{
public:
    enum class Type
    {
        REAL = 0,
        CHAR = 1,
        BOOL = 2,
        ARRAY = 3,
        REAL_ARRAY = 0x77,
        UINT16_ARRAY = 0x88,
        NIL = 0xaa
    };
    DnhValue(Type t) : type_(t) {}
    virtual ~DnhValue() {};
    Type GetType() const { return type_; }
    virtual double ToNum() const = 0;
    int ToInt() const { return (int)ToNum(); }
    virtual bool ToBool() const = 0;
    virtual std::wstring ToString() const = 0;
    virtual void Push(lua_State* L) const = 0;
    virtual void Serialize(std::ostream& out) const = 0;
    virtual std::unique_ptr<DnhValue> Clone() const = 0;
    static std::unique_ptr<DnhValue> Get(lua_State*L, int idx);
    static std::unique_ptr<DnhValue> Deserialize(std::istream& stream);
    static double ToNum(lua_State* L, int idx);
    static int ToInt(lua_State* L, int idx) { return (int)ToNum(L, idx); }
    static bool ToBool(lua_State* L, int idx);
    static std::wstring ToString(lua_State* L, int idx);
    static std::string ToStringU8(lua_State* L, int idx);
    static const std::unique_ptr<DnhValue>& Nil();
private:
    const Type type_;
};

class DnhReal : public DnhValue
{
public:
    DnhReal(double num);
    double ToNum() const override;
    bool ToBool() const override;
    std::wstring ToString() const override;
    void Push(lua_State* L) const override;
    void Serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> Clone() const override;
private:
    double value_;
};

class DnhChar : public DnhValue
{
public:
    DnhChar(wchar_t c);
    double ToNum() const override;
    bool ToBool() const override;
    std::wstring ToString() const override;
    void Push(lua_State* L) const override;
    void Serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> Clone() const override;
private:
    wchar_t value_;
};

class DnhBool : public DnhValue
{
public:
    DnhBool(bool b);
    double ToNum() const override;
    bool ToBool() const override;
    std::wstring ToString() const override;
    void Push(lua_State* L) const override;
    void Serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> Clone() const override;
private:
    bool value_;
};

class DnhArray : public DnhValue
{
public:
    DnhArray();
    DnhArray(size_t reserveSize);
    DnhArray(std::vector<std::unique_ptr<DnhValue>>&& a);
    DnhArray(const std::vector<double>& rs);
    DnhArray(const std::wstring& s);
    DnhArray(const Point2D& p);
    DnhArray(const std::vector<Point2D>& ps);
    size_t GetSize() const;
    void PushBack(std::unique_ptr<DnhValue>&& v);
    double ToNum() const override;
    bool ToBool() const override;
    std::wstring ToString() const override;
    const std::unique_ptr<DnhValue>& Index(int idx) const;
    void Push(lua_State* L) const override;
    void Serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> Clone() const override;
    void Reserve(size_t size);
private:
    std::vector<std::unique_ptr<DnhValue>> values_;
};

class DnhNil : public DnhValue
{
public:
    DnhNil();
    double ToNum() const override;
    bool ToBool() const override;
    std::wstring ToString() const override;
    void Push(lua_State* L) const override;
    void Serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> Clone() const override;
};

// DnhRealArray: 要素が全てrealの配列
// シリアライズ時にDnhArrayに比べてメモリ消費量が少ない（要素がrealと分かっているので要素のヘッダを作る必要がない
// 弾幕風にはないフォーマットなので互換を保ちたい部分ではシリアライズしてはいけない(Luaスタックに持っていく分には問題ない)
class DnhRealArray : public DnhValue
{
public:
    DnhRealArray();
    DnhRealArray(size_t reserveSize);
    DnhRealArray(const std::vector<double>& rs);
    DnhRealArray(std::vector<double>&& rs);
    size_t GetSize() const;
    void PushBack(double r);
    double ToNum() const override;
    bool ToBool() const override;
    std::wstring ToString() const override;
    double Index(int idx) const;
    void Push(lua_State* L) const override;
    void Serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> Clone() const override;
    void Reserve(size_t size);
private:
    std::vector<double> values_;
};

// DnhRealArrayのuint16_t版
class DnhUInt16Array : public DnhValue
{
public:
    DnhUInt16Array();
    DnhUInt16Array(size_t reserveSize);
    DnhUInt16Array(const std::vector<uint16_t>& rs);
    DnhUInt16Array(std::vector<uint16_t>&& rs);
    size_t GetSize() const;
    void PushBack(uint16_t i);
    double ToNum() const override;
    bool ToBool() const override;
    std::wstring ToString() const override;
    uint16_t Index(int idx) const;
    void Push(lua_State* L) const override;
    void Serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> Clone() const override;
    void Reserve(size_t size);
private:
    std::vector<uint16_t> values_;
};
}
#pragma once

#include <bstorm/type.hpp>

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <luajit/lua.hpp>

namespace bstorm {
  class DnhValue {
  public:
    enum class Type {
      REAL = 0,
      CHAR = 1,
      BOOL = 2,
      ARRAY = 3,
      NIL = 0xaa
    };
    virtual ~DnhValue() {};
    Type getType() const { return type; }
    virtual double toNum() const = 0;
    int toInt() const { return (int)toNum(); }
    virtual bool toBool() const = 0;
    virtual std::wstring toString() const = 0;
    virtual void push(lua_State* L) const = 0;
    virtual void serialize(std::ostream& out) const = 0;
    virtual std::unique_ptr<DnhValue> clone() const = 0;
    static std::unique_ptr<DnhValue> get(lua_State*L, int idx);
    static std::unique_ptr<DnhValue> deserialize(std::istream& stream);
    static double toNum(lua_State* L, int idx);
    static int toInt(lua_State* L, int idx) { return (int)toNum(L, idx); }
    static bool toBool(lua_State* L, int idx);
    static std::wstring toString(lua_State* L, int idx);
    static std::string toStringU8(lua_State* L, int idx);
  protected:
    Type type;
  };

  class DnhReal : public DnhValue {
  public:
    DnhReal(double num);
    double toNum() const override;
    bool toBool() const override;
    std::wstring toString() const override;
    void push(lua_State* L) const override;
    void serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> clone() const override;
  private:
    double value;
  };

  class DnhChar : public DnhValue {
  public:
    DnhChar(wchar_t c);
    double toNum() const override;
    bool toBool() const override;
    std::wstring toString() const override;
    void push(lua_State* L) const override;
    void serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> clone() const override;
  private:
    wchar_t value;
  };

  class DnhBool : public DnhValue {
  public:
    DnhBool(bool b);
    double toNum() const override;
    bool toBool() const override;
    std::wstring toString() const override;
    void push(lua_State* L) const override;
    void serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> clone() const override;
  private:
    bool value;
  };

  class DnhArray : public DnhValue {
  public:
    DnhArray();
    DnhArray(std::vector<std::unique_ptr<DnhValue>>&& a);
    DnhArray(const std::wstring& s);
    DnhArray(const std::vector<double>& ns);
    DnhArray(const Point2D& p);
    DnhArray(const std::vector<Point2D>& ps);
    size_t getSize() const;
    void pushBack(std::unique_ptr<DnhValue>&& v);
    double toNum() const override;
    bool toBool() const override;
    std::wstring toString() const override;
    std::unique_ptr<DnhValue> index(int idx) const;
    void push(lua_State* L) const override;
    void serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> clone() const override;
  private:
    std::vector<std::unique_ptr<DnhValue>> values;
  };

  class DnhNil : public DnhValue {
  public:
    DnhNil();
    double toNum() const override;
    bool toBool() const override;
    std::wstring toString() const override;
    void push(lua_State* L) const override;
    void serialize(std::ostream& out) const override;
    std::unique_ptr<DnhValue> clone() const override;
  };
}
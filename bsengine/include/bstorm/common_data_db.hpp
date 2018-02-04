#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>

// NOTE : keyListの順番が辞書順である必要があるのでunorderedじゃないmapを使う

namespace bstorm {
  class DnhValue;
  class CommonDataDB {
  public:
    typedef std::wstring DataKey;
    typedef std::wstring DataAreaName;
    typedef std::map<DataKey, std::unique_ptr<DnhValue>> CommonDataArea;
    CommonDataDB();
    ~CommonDataDB();
    void setCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& value);
    std::unique_ptr<DnhValue> getCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& defaultValue) const;
    void clearCommonData();
    void deleteCommonData(const DataKey& key);
    void setAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& value);
    std::unique_ptr<DnhValue> getAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& value) const;
    void clearAreaCommonData(const DataAreaName& areaName);
    void deleteAreaCommonData(const DataAreaName& areaName, const DataKey& key);
    void createCommonDataArea(const DataAreaName& areaName);
    bool isCommonDataAreaExists(const DataAreaName& areaName) const;
    void copyCommonDataArea(const DataAreaName& dest, const DataAreaName& src);
    std::vector<DataAreaName> getCommonDataAreaKeyList() const;
    std::vector<DataAreaName> getCommonDataValueKeyList(const DataAreaName& areaName) const;
    bool saveCommonDataArea(const DataAreaName& areaName, std::ostream& stream) const;
    bool saveCommonDataArea(const DataAreaName& areaName, const std::wstring& path) const;
    bool loadCommonDataArea(const DataAreaName& areaName, std::istream& stream);
    bool loadCommonDataArea(const DataAreaName& areaName, const std::wstring& path);
    static constexpr wchar_t* DefaultDataAreaName = L"";
    template <typename T>
    void backDoor() const {}
  private:
    std::map<DataAreaName, CommonDataArea> areaTable;
  };
}
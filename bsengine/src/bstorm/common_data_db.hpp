#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>

namespace bstorm
{
class DnhValue;
class CommonDataDB
{
public:
    using DataKey = std::wstring;
    using DataAreaName = std::wstring;
    using CommonDataArea = std::map<DataKey, std::unique_ptr<DnhValue>>;
    CommonDataDB();
    ~CommonDataDB();
    void SetCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& value);
    std::unique_ptr<DnhValue> GetCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& defaultValue) const;
    void ClearCommonData();
    void DeleteCommonData(const DataKey& key);
    void SetAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& value);
    std::unique_ptr<DnhValue> GetAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& value) const;
    void ClearAreaCommonData(const DataAreaName& areaName);
    void DeleteAreaCommonData(const DataAreaName& areaName, const DataKey& key);
    void CreateCommonDataArea(const DataAreaName& areaName);
    bool IsCommonDataAreaExists(const DataAreaName& areaName) const;
    void CopyCommonDataArea(const DataAreaName& dest, const DataAreaName& src);
    std::vector<DataAreaName> GetCommonDataAreaKeyList() const;
    std::vector<DataAreaName> GetCommonDataValueKeyList(const DataAreaName& areaName) const;
    bool SaveCommonDataArea(const DataAreaName& areaName, std::ostream& stream) const;
    bool SaveCommonDataArea(const DataAreaName& areaName, const std::wstring& path) const;
    bool LoadCommonDataArea(const DataAreaName& areaName, std::istream& stream);
    bool LoadCommonDataArea(const DataAreaName& areaName, const std::wstring& path);
    static constexpr wchar_t* DefaultDataAreaName = L"";
    const std::map<DataAreaName, CommonDataArea>& GetCommonDataAreaTable() const;
private:
    // NOTE : keyListの順番が辞書順である必要があるのでunorderedじゃないmapを使う
    std::map<DataAreaName, CommonDataArea> areaTable_;
};
}
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
    CommonDataDB(const CommonDataDB&) = default;
    CommonDataDB(CommonDataDB&&) = default;
    CommonDataDB& operator=(const CommonDataDB& other) = default;
    CommonDataDB& operator=(CommonDataDB&& other) = default;
    void SetCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& value);
    const std::unique_ptr<DnhValue>& GetCommonData(const DataKey& key, const std::unique_ptr<DnhValue>& defaultValue) const;
    void ClearCommonData();
    void DeleteCommonData(const DataKey& key);
    void SetAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& value);
    const std::unique_ptr<DnhValue>& GetAreaCommonData(const DataAreaName& areaName, const DataKey& key, const std::unique_ptr<DnhValue>& defaultValue) const;
    void ClearAreaCommonData(const DataAreaName& areaName);
    void DeleteAreaCommonData(const DataAreaName& areaName, const DataKey& key);
    void CreateCommonDataArea(const DataAreaName& areaName);
    bool IsCommonDataAreaExists(const DataAreaName& areaName) const;
    bool CopyCommonDataArea(const DataAreaName& destAreaName, const DataAreaName& srcAreaName);
    bool CopyCommonDataAreaFromOtherDB(const DataAreaName& destAreaName, const DataAreaName& srcAreaName, const CommonDataDB& srcDB);
    std::vector<DataAreaName> GetCommonDataAreaKeyList() const;
    std::vector<DataAreaName> GetCommonDataValueKeyList(const DataAreaName& areaName) const noexcept(false);
    void SaveCommonDataArea(const DataAreaName& areaName, std::ostream& stream) const noexcept(false);
    void SaveCommonDataArea(const DataAreaName& areaName, const std::wstring& path) const noexcept(false);
    void LoadCommonDataArea(const DataAreaName& areaName, std::istream& stream) noexcept(false);
    void LoadCommonDataArea(const DataAreaName& areaName, const std::wstring& path) noexcept(false);
    static constexpr wchar_t* DefaultDataAreaName = L"";
    const std::map<DataAreaName, CommonDataArea>& GetCommonDataAreaTable() const;
private:
    // NOTE : keyListの順番が辞書順である必要があるのでunorderedじゃないmapを使う
    std::map<DataAreaName, CommonDataArea> areaTable_;
};
}
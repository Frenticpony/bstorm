#include <bstorm/common_data_db.hpp>

#include <bstorm/util.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/log_error.hpp>

#include <sstream>
#include <fstream>

namespace
{
const char COMMON_DATA_HEADER[] = "RecordBufferFile";
}

namespace bstorm
{
CommonDataDB::CommonDataDB()
{
    CreateCommonDataArea(DefaultDataAreaName);
}

void CommonDataDB::SetCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& value)
{
    SetAreaCommonData(DefaultDataAreaName, key, std::move(value));
}

const std::unique_ptr<DnhValue>& CommonDataDB::GetCommonData(const DataKey& key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return GetAreaCommonData(DefaultDataAreaName, key, defaultValue);
}

void CommonDataDB::ClearCommonData()
{
    ClearAreaCommonData(DefaultDataAreaName);
}

void CommonDataDB::DeleteCommonData(const DataKey& key)
{
    DeleteAreaCommonData(DefaultDataAreaName, key);
}

void CommonDataDB::SetAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& value)
{
    // エリアがない場合は無視
    auto it = areaTable_.find(areaName);
    if (it != areaTable_.end())
    {
        CommonDataArea& area = it->second;
        area[key] = std::move(value);
    }
}

const std::unique_ptr<DnhValue>& CommonDataDB::GetAreaCommonData(const DataAreaName& areaName, const DataKey& key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    // エリアがない場合もデフォルト値を返す
    const auto itAreaTable = areaTable_.find(areaName);
    if (itAreaTable != areaTable_.end())
    {
        const CommonDataArea& area = itAreaTable->second;
        const auto itArea = area.find(key);
        if (itArea != area.end())
        {
            return itArea->second;
        }
    }
    return defaultValue;
}


void CommonDataDB::DeleteAreaCommonData(const DataAreaName& areaName, const DataKey& key)
{
    auto it = areaTable_.find(areaName);
    if (it != areaTable_.end())
    {
        CommonDataArea& area = it->second;
        area.erase(key);
    }
}

void CommonDataDB::ClearAreaCommonData(const DataAreaName& areaName)
{
    // エリア自体は消さない
    auto it = areaTable_.find(areaName);
    if (it != areaTable_.end())
    {
        CommonDataArea& area = it->second;
        area.clear();
    }
}

void CommonDataDB::CreateCommonDataArea(const DataAreaName& areaName)
{
    // 既にある場合は無視
    if (areaTable_.count(areaName) == 0)
    {
        areaTable_[areaName] = CommonDataArea();
    }
}

bool CommonDataDB::IsCommonDataAreaExists(const DataAreaName& areaName) const
{
    return areaTable_.count(areaName) != 0;
}

bool CommonDataDB::CopyCommonDataArea(const DataAreaName& destAreaName, const DataAreaName& srcAreaName)
{
    return CopyCommonDataAreaFromOtherDB(destAreaName, srcAreaName, *this);
}

bool CommonDataDB::CopyCommonDataAreaFromOtherDB(const DataAreaName& destAreaName, const DataAreaName& srcAreaName, const CommonDataDB& srcDB)
{
    auto it = srcDB.areaTable_.find(srcAreaName);
    // コピー元エリアが無ければ無視
    if (it == srcDB.areaTable_.end()) return false;
    const CommonDataArea& srcArea = it->second;
    CommonDataArea destArea;
    // 全てclone
    for (const auto& entry : srcArea)
    {
        destArea[entry.first] = entry.second->Clone();
    }
    this->areaTable_[destAreaName] = std::move(destArea);
    return true;
}

std::vector<CommonDataDB::DataAreaName> CommonDataDB::GetCommonDataAreaKeyList() const
{
    std::vector<DataAreaName> keys; keys.reserve(areaTable_.size());
    for (const auto& entry : areaTable_)
    {
        keys.push_back(entry.first);
    }
    return keys;
}

std::vector<CommonDataDB::DataKey> CommonDataDB::GetCommonDataValueKeyList(const DataAreaName& areaName) const
{
    std::vector<DataKey> keys;
    auto it = areaTable_.find(areaName);
    if (it != areaTable_.end())
    {
        const CommonDataArea& area = it->second;
        keys.reserve(area.size());
        for (const auto& entry : area)
        {
            keys.push_back(entry.first);
        }
    }
    return keys;
}

static void WriteCommonDataHeader(const CommonDataDB::CommonDataArea& area, std::ostream& out)
{
    // データ数の2倍
    uint32_t keyCntx2 = area.size() << 1;
    // 末尾NULは書かないので-1
    out.write(COMMON_DATA_HEADER, sizeof(COMMON_DATA_HEADER) - 1);
    out.write((char*)&keyCntx2, sizeof(keyCntx2));
}

static void WriteCommonDataDataSection(const CommonDataDB::DataKey& key, const std::unique_ptr<DnhValue>& value, std::ostream& out)
{
    std::string keySJIS = ToMultiByte<932>(key);
    uint32_t keySize = keySJIS.size();
    std::ostringstream elemStream;
    value->Serialize(elemStream);
    std::string elem = elemStream.str();
    uint32_t elemSize = elem.size();
    uint32_t keyName_sizeSize = keySize + 5;
    uint32_t footer = 0x04;

    out.put('\xff');
    out.write((char*)&keySize, sizeof(keySize));
    out.write(&keySJIS[0], keySize);
    out.write((char*)&elemSize, sizeof(elemSize));
    out.write(&elem[0], elemSize);
    out.put('\x02');
    out.write((char*)&keyName_sizeSize, sizeof(keyName_sizeSize));
    out.write(&keySJIS[0], keySize);
    out.write("_size", 5);
    out.write((char*)&footer, sizeof(footer));
    out.write((char*)&elemSize, sizeof(elemSize));
}

void CommonDataDB::SaveCommonDataArea(const DataAreaName& areaName, std::ostream& out) const noexcept(false)
{
    auto it = areaTable_.find(areaName);
    if (it == areaTable_.end())
    {
        throw common_data_area_not_exist(areaName);
    }
    const CommonDataArea& area = it->second;
    WriteCommonDataHeader(area, out);
    for (const auto& entry : area)
    {
        WriteCommonDataDataSection(entry.first, entry.second, out);
    }
    if (!out.good())
    {
        throw failed_to_save_common_data_area();
    }
}

void CommonDataDB::SaveCommonDataArea(const DataAreaName& areaName, const std::wstring& path) const noexcept(false)
{
    if (areaTable_.count(areaName) == 0)
    {
        throw common_data_area_not_exist(areaName);
    }
    std::ofstream fstream;
    MakeDirectoryP(GetParentPath(path));
    fstream.open(path, std::ios::out | std::ios::binary);
    if (!fstream.good())
    {
        throw failed_to_save_common_data_area()
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    }
    try
    {
        SaveCommonDataArea(areaName, fstream);
    } catch (Log& log)
    {
        log.SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        throw log;
    }
}

static uint32_t ReadCommonDataHeader(std::istream& in)
{
    constexpr size_t headerSize = sizeof(COMMON_DATA_HEADER) - 1;
    std::string header(headerSize, '\0');
    in.read(&header[0], headerSize);
    if (header != std::string(COMMON_DATA_HEADER))
    {
        throw illegal_common_data_format();
    }
    uint32_t keyCntx2 = 0;
    in.read((char*)&keyCntx2, sizeof(keyCntx2));
    return keyCntx2;
}

static void ReadCommonDataDataSection(CommonDataDB::CommonDataArea& area, std::istream& in)
{
    uint8_t sectionType = 0x02;
    in.read((char*)&sectionType, sizeof(sectionType)); // 0xff or 0x02
    if (sectionType == 0xff)
    {
        // read element
        uint32_t keySize = 0;
        in.read((char*)&keySize, sizeof(keySize));
        std::string keyName(keySize, '\0');
        in.read(&keyName[0], keySize);
        // skip element size
        in.ignore(sizeof(uint32_t));
        auto value = DnhValue::Deserialize(in);
        area[FromMultiByte<932>(keyName)] = std::move(value);
    } else
    {
        // read element size
        uint32_t keySizeSize = 0;
        in.read((char*)&keySizeSize, sizeof(keySizeSize));
        // skip keyName_size, 0x00000004, element size
        in.ignore(keySizeSize + sizeof(uint32_t) + sizeof(uint32_t));
    }
}

void CommonDataDB::LoadCommonDataArea(const DataAreaName& areaName, std::istream& in) noexcept(false)
{
    // 新しいエリアで上書きする
    CommonDataArea area;
    const auto keyCntx2 = ReadCommonDataHeader(in);
    for (uint32_t i = 0; i < keyCntx2; i++)
    {
        ReadCommonDataDataSection(area, in);
    }
    if (area.size() != (keyCntx2 >> 1))
    {
        throw illegal_common_data_format();
    }
    areaTable_[areaName] = std::move(area);
}

void CommonDataDB::LoadCommonDataArea(const DataAreaName& areaName, const std::wstring& path) noexcept(false)
{
    std::ifstream stream;
    stream.open(path, std::ios::in | std::ios::binary);
    if (!stream.is_open())
    {
        throw cant_open_common_data_file(path);
    }
    try
    {
        LoadCommonDataArea(areaName, stream);
    } catch (Log& log)
    {
        log.SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        throw log;
    }
}

const std::map<CommonDataDB::DataAreaName, CommonDataDB::CommonDataArea>& CommonDataDB::GetCommonDataAreaTable() const
{
    return areaTable_;
}
}
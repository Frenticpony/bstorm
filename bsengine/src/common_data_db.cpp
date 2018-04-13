#include <bstorm/common_data_db.hpp>

#include <bstorm/util.hpp>
#include <bstorm/dnh_value.hpp>

#include <cstdint>
#include <sstream>
#include <fstream>

namespace bstorm {
  CommonDataDB::CommonDataDB() {
    createCommonDataArea(DefaultDataAreaName);
  }

  CommonDataDB::~CommonDataDB() {}

  void CommonDataDB::setCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& value) {
    setAreaCommonData(DefaultDataAreaName, key, std::move(value));
  }

  std::unique_ptr<DnhValue> CommonDataDB::getCommonData(const DataKey& key, std::unique_ptr<DnhValue>&& defaultValue) const {
    return getAreaCommonData(DefaultDataAreaName, key, std::move(defaultValue));
  }

  void CommonDataDB::clearCommonData() {
    clearAreaCommonData(DefaultDataAreaName);
  }

  void CommonDataDB::deleteCommonData(const DataKey& key) {
    deleteAreaCommonData(DefaultDataAreaName, key);
  }

  void CommonDataDB::setAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& value) {
    // エリアがない場合は無視
    auto it = areaTable.find(areaName);
    if (it != areaTable.end()) {
      CommonDataArea& area = it->second;
      area[key] = std::move(value);
    }
  }

  std::unique_ptr<DnhValue> CommonDataDB::getAreaCommonData(const DataAreaName& areaName, const DataKey& key, std::unique_ptr<DnhValue>&& defaultValue) const {
    // エリアがない場合もデフォルト値を返す
    const auto itAreaTable = areaTable.find(areaName);
    if (itAreaTable != areaTable.end()) {
      const CommonDataArea& area = itAreaTable->second;
      const auto itArea = area.find(key);
      if (itArea != area.end()) {
        return itArea->second->clone();
      }
    }
    return std::move(defaultValue);
  }


  void CommonDataDB::deleteAreaCommonData(const DataAreaName& areaName, const DataKey& key) {
    auto it = areaTable.find(areaName);
    if (it != areaTable.end()) {
      CommonDataArea& area = it->second;
      area.erase(key);
    }
  }

  void CommonDataDB::clearAreaCommonData(const DataAreaName& areaName) {
    // エリア自体は消さない
    auto it = areaTable.find(areaName);
    if (it != areaTable.end()) {
      CommonDataArea& area = it->second;
      area.clear();
    }
  }

  void CommonDataDB::createCommonDataArea(const DataAreaName& areaName) {
    // 既にある場合は無視
    if (areaTable.count(areaName) == 0) {
      areaTable[areaName] = CommonDataArea();
    }
  }

  bool CommonDataDB::isCommonDataAreaExists(const DataAreaName& areaName) const {
    return areaTable.count(areaName) != 0;
  }

  void CommonDataDB::copyCommonDataArea(const DataAreaName& dest, const DataAreaName& src) {
    // コピー元エリアが無ければ無視
    auto it = areaTable.find(src);
    if (it != areaTable.end()) {
      CommonDataArea& srcArea = it->second;
      CommonDataArea destArea;
      // 全てclone
      for (const auto& entry : srcArea) {
        destArea[entry.first] = entry.second->clone();
      }
      areaTable[dest] = std::move(destArea);
    }
  }

  std::vector<CommonDataDB::DataAreaName> CommonDataDB::getCommonDataAreaKeyList() const {
    std::vector<DataAreaName> keys; keys.reserve(areaTable.size());
    for (const auto& entry : areaTable) {
      keys.push_back(entry.first);
    }
    return keys;
  }

  std::vector<CommonDataDB::DataKey> CommonDataDB::getCommonDataValueKeyList(const DataAreaName& areaName) const {
    std::vector<DataKey> keys;
    auto it = areaTable.find(areaName);
    if (it != areaTable.end()) {
      const CommonDataArea& area = it->second;
      keys.reserve(area.size());
      for (const auto& entry : area) {
        keys.push_back(entry.first);
      }
    }
    return keys;
  }

  static const char commonDataHeader[] = "RecordBufferFile";

  static void writeCommonDataHeader(const CommonDataDB::CommonDataArea& area, std::ostream& out) {
    // データ数の2倍
    uint32_t keyCntx2 = area.size() << 1;
    // 末尾NULは書かないので-1
    out.write(commonDataHeader, sizeof(commonDataHeader) - 1);
    out.write((char*)&keyCntx2, sizeof(keyCntx2));
  }

  static void writeCommonDataDataSection(const CommonDataDB::DataKey& key, const std::unique_ptr<DnhValue>& value, std::ostream& out) {
    std::string keySJIS = toMultiByte<932>(key);
    uint32_t keySize = keySJIS.size();
    std::ostringstream elemStream;
    value->serialize(elemStream);
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

  bool CommonDataDB::saveCommonDataArea(const DataAreaName& areaName, std::ostream& out) const {
    // 存在しないエリアなら何もしない
    auto it = areaTable.find(areaName);
    if (it != areaTable.end()) {
      const CommonDataArea& area = it->second;
      writeCommonDataHeader(area, out);
      for (const auto& entry : area) {
        writeCommonDataDataSection(entry.first, entry.second, out);
      }
      if (out.good()) return true;
    }
    return false;
  }

  bool CommonDataDB::saveCommonDataArea(const DataAreaName& areaName, const std::wstring& path) const {
    // 存在しないエリアなら何もしない
    if (areaTable.count(areaName) == 0) return false;
    std::ofstream fstream;
    mkdir_p(parentPath(path));
    fstream.open(path, std::ios::out | std::ios::binary);
    if (!fstream.good()) return false;
    return saveCommonDataArea(areaName, fstream);
  }

  static uint32_t readCommonDataHeader(std::istream& in) {
    in.ignore(sizeof(commonDataHeader) - 1);
    uint32_t keyCntx2 = 0;
    in.read((char*)&keyCntx2, sizeof(keyCntx2));
    return keyCntx2;
  }

  static void readCommonDataDataSection(CommonDataDB::CommonDataArea& area, std::istream& in) {
    uint8_t sectionType = 0x02;
    in.read((char*)&sectionType, sizeof(sectionType)); // 0xff or 0x02
    if (sectionType == 0xff) {
      // read element
      uint32_t keySize = 0;
      in.read((char*)&keySize, sizeof(keySize));
      std::string keyName(keySize, '\0');
      in.read(&keyName[0], keySize);
      // skip element size
      in.ignore(sizeof(uint32_t));
      auto value = DnhValue::deserialize(in);
      area[fromMultiByte<932>(keyName)] = std::move(value);
    } else {
      // read element size
      uint32_t keySizeSize = 0;
      in.read((char*)&keySizeSize, sizeof(keySizeSize));
      // skip keyName_size, 0x00000004, element size
      in.ignore(keySizeSize + sizeof(uint32_t) + sizeof(uint32_t));
    }
  }

  bool CommonDataDB::loadCommonDataArea(const DataAreaName& areaName, std::istream& in) {
    // 新しいエリアで上書きする
    CommonDataArea area;
    const auto keyCntx2 = readCommonDataHeader(in);
    for (uint32_t i = 0; i < keyCntx2; i++) {
      readCommonDataDataSection(area, in);
    }
    areaTable[areaName] = std::move(area);
    return true;
  }


  bool CommonDataDB::loadCommonDataArea(const DataAreaName& areaName, const std::wstring& path) {
    std::ifstream stream;
    stream.open(path, std::ios::in | std::ios::binary);
    if (!stream.is_open()) return false;
    return loadCommonDataArea(areaName, stream);
  }
}
#pragma once

#include <bstorm/logger.hpp>

namespace bstorm
{
Log LogError(const std::wstring& msg);
Log LogError(const std::string& msg);
Log LogError(std::string&& msg);

// CommonData
Log cant_open_common_data_file(const std::wstring& filePath);
Log failed_to_save_common_data_area();
Log illegal_common_data_format();
Log common_data_area_not_exist(const std::wstring& areaName);

// Replay
Log cant_open_replay_file(const std::wstring& filePath);
Log failed_to_save_replay_file(const std::wstring& filePath);
Log illegal_replay_format(const std::wstring& filePath);
}

#include <bstorm/log_error.hpp>

namespace bstorm
{
Log cant_open_common_data_file(const std::wstring& filePath)
{
    return LogError("can't open common data file.")
        .Param(LogParam(LogParam::Tag::TEXT, filePath));
}
Log failed_to_save_common_data_area()
{
    return LogError("failed to save common area.");
}
Log illegal_common_data_format()
{
    return LogError("can't load illegal format common data.");
}
Log common_data_area_not_exist(const std::wstring & areaName)
{
    return LogError("common data area does not exist.")
        .Param(LogParam(LogParam::Tag::TEXT, areaName));
}
Log cant_open_replay_file(const std::wstring& filePath)
{
    return LogError("can't open replay file.")
        .Param(LogParam(LogParam::Tag::TEXT, filePath));
}
Log failed_to_save_replay_file(const std::wstring & filePath)
{
    return LogError("failed to save replay file.")
        .Param(LogParam(LogParam::Tag::TEXT, filePath));
}
Log illegal_replay_format(const std::wstring & filePath)
{
    return LogError("can't load illegal format replay file.")
        .Param(LogParam(LogParam::Tag::TEXT, filePath));
}
}
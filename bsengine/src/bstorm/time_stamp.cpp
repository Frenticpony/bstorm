#include <bstorm/time_stamp.hpp>

#include <windows.h>

namespace bstorm
{
TimeStamp GetFileLastUpdateTime(const std::wstring & file) noexcept(false)
{
    HANDLE fileHandle = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return TIME_STAMP_NONE;
    }

    FILETIME lastUpdateTime;
    if (GetFileTime(fileHandle, NULL, NULL, &lastUpdateTime))
    {
        uint64_t high = static_cast<uint64_t>(lastUpdateTime.dwHighDateTime);
        uint64_t low = static_cast<uint64_t>(lastUpdateTime.dwLowDateTime);
        return (high << 32u) | low;
    }
    return TIME_STAMP_NONE;
}
}

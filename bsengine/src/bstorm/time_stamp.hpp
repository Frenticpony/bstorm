#pragma once

#include <cstdint>
#include <string>

namespace bstorm
{
using TimeStamp = uint64_t;
constexpr TimeStamp TIME_STAMP_NONE = 0u;
TimeStamp GetFileLastUpdateTime(const std::wstring& file) noexcept(false);
}

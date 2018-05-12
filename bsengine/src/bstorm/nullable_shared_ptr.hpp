#pragma once

#include <memory>

namespace bstorm
{
template <class T>
using NullableSharedPtr = std::shared_ptr<T>;
}

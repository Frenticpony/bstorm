#pragma once

#include <algorithm>

namespace bstorm
{
template <typename T>
T constrain(const T& v, const T& min, const T& max)
{
    return std::min(std::max<T>(v, min), max);
}

inline int NextPow2(int x)
{
    if (x < 0) return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}
}
#pragma once

namespace bstorm
{
template <typename T>
struct Rect
{
    Rect(T l, T t, T r, T b) : left(l), top(t), right(r), bottom(b) {}
    T left, top, right, bottom;
};
}

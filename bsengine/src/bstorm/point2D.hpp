#pragma once

namespace bstorm
{
class Point2D
{
public:
    Point2D() : x(0.0f), y(0.0f) {}
    Point2D(float x, float y) : x(x), y(y) {}
    float x;
    float y;
};
}
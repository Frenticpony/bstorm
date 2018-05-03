#pragma once

#include <bstorm/type.hpp>

#include <random>

namespace bstorm
{
class RandGenerator
{
public:
    static constexpr RandValue min() { return 0u; }
    static constexpr RandValue max() { return 0xffffffffu; }
    RandGenerator(RandValue seed) : s(seed) {}
    RandValue operator()() { return RandUInt(); }
    RandValue RandUInt()
    {
        s = s ^ (s << 13); s = s ^ (s >> 17);
        return s = s ^ (s << 5);
    }
    double RandDouble(double min, double max)
    {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(*this);
    }
    RandValue GetSeed() const { return s; }
    void SetSeed(RandValue seed) { s = seed; }
private:
    RandValue s;
};
}
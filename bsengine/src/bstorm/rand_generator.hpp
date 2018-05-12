#pragma once

#include <random>

namespace bstorm
{
using RandSeed = uint32_t;

class RandGenerator
{
public:
    static constexpr RandSeed min() { return 0u; }
    static constexpr RandSeed max() { return 0xffffffffu; }
    RandGenerator(RandSeed seed) : s(seed) {}
    RandSeed operator()() { return RandUInt(); }
    RandSeed RandUInt()
    {
        s = s ^ (s << 13); s = s ^ (s >> 17);
        return s = s ^ (s << 5);
    }
    double RandDouble(double min, double max)
    {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(*this);
    }
    RandSeed GetSeed() const { return s; }
    void SetSeed(RandSeed seed) { s = seed; }
private:
    RandSeed s;
};
}
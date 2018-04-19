#pragma once

#include <stdint.h>
#include <random>

namespace bstorm
{
class RandGenerator
{
public:
    static constexpr uint32_t min() { return 0u; }
    static constexpr uint32_t max() { return 0xffffffffu; }
    RandGenerator(uint32_t seed) : s(seed) {}
    uint32_t operator()() { return randUInt(); }
    uint32_t randUInt()
    {
        s = s ^ (s << 13); s = s ^ (s >> 17);
        return s = s ^ (s << 5);
    }
    double randDouble(double min, double max)
    {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(*this);
    }
    uint32_t getSeed() const { return s; }
    void setSeed(uint32_t seed) { s = seed; }
private:
    uint32_t s;
};
}
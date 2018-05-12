#pragma once
#include <stdint.h>

namespace bstorm
{
using PlayerLife = double;
using PlayerSpell = double;
using PlayerPower = double;
using PlayerScore = int64_t;
using PlayerGraze = int64_t;
using PlayerPoint = int64_t;

class StageCommonPlayerParams
{
public:
    StageCommonPlayerParams() { Reset(); }
    void Reset()
    {
        life = 2.0;
        spell = 3.0;
        power = 1.0;
        score = graze = point = 0;
    }
    PlayerLife life;
    PlayerSpell spell;
    PlayerPower power;
    PlayerScore score;
    PlayerGraze graze;
    PlayerPoint point;
};
}
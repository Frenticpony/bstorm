#pragma once
namespace bstorm
{
class ShotCounter
{
public:
    ShotCounter() : playerShotCount(0), enemyShotCount(0) {}
    int playerShotCount;
    int enemyShotCount;
};
}

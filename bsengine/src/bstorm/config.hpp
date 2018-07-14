#pragma once

#include <bstorm/version.hpp>
#include <bstorm/dnh_const.hpp>

#include <string>
#include <vector>

#undef VK_LEFT
#undef VK_RIGHT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

namespace bstorm
{
namespace conf
{
struct KeyMap
{
    std::string actionName;
    int64_t vkey;
    int64_t key;
    int64_t pad;
};

struct KeyConfig
{
    std::vector<struct KeyMap> keyMaps{
        { u8"Left (←)", VK_LEFT, KEY_LEFT, 0 },
    { u8"Right (→)", VK_RIGHT, KEY_RIGHT, 1 },
    { u8"Up (↑)", VK_UP, KEY_UP, 2 },
    { u8"Down (↓)", VK_DOWN, KEY_DOWN, 3 },
    { u8"Decide (決定)", VK_OK, KEY_Z, 5 },
    { u8"Cancel (キャンセル)", VK_CANCEL, KEY_X, 6 },
    { u8"Shot (ショット)", VK_SHOT, KEY_Z, 5 },
    { u8"Bomb (ボム)", VK_BOMB, KEY_X, 6 },
    { u8"SlowMove (低速移動)", VK_SLOWMOVE, KEY_LSHIFT, 7 },
    { u8"User1 (ユーザ定義1)", VK_USER1, KEY_C, 8 },
    { u8"User2 (ユーザ定義2)", VK_USER2, KEY_V, 9 },
    { u8"Pause (一時停止)", VK_PAUSE, KEY_ESCAPE, 10 }
    };
};

struct Options
{
    bool hideMouseCursor = false;
    bool saveLogFile = false;
};

struct WindowConfig
{
    bool fullScreen = false;
    int64_t windowWidth = 640;
    int64_t windowHeight = 480;
};

struct BstormConfig
{
    std::string generatedBy = BSTORM_VERSION;
    struct KeyConfig keyConfig;
    struct Options options;
    struct WindowConfig windowConfig;
};
}

conf::BstormConfig LoadBstormConfig(const std::string& path, bool isBinaryFormat);
bool SaveBstormConfig(const std::string& path, bool isBinaryFormat, conf::BstormConfig config);
}

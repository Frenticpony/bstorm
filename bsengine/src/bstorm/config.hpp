#pragma once

#include <bstorm/version.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/logger.hpp>

#include "../../json/single_include/nlohmann/json.hpp"

#include <string>

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

using nlohmann::json;

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

template <typename T>
void read_value(const char* name, T& dst, const json& j, bool rethrowException = false)
{
    try
    {
        dst = j.at(name).get<T>();
    } catch (const std::exception& e)
    {
        if (rethrowException)
        {
            throw;
        }
        Logger::WriteLog(Log::Level::LV_WARN, "config property '" + std::string(name) + "' can't load from file, set a default value.");
    }
}

inline json get_untyped(const json& j, const char *property)
{
    if (j.find(property) != j.end())
    {
        return j.at(property).get<json>();
    }
    return json();
}

inline void from_json(const json& _j, struct KeyMap& _x)
{
    read_value<std::string>("action_name", _x.actionName, _j, true);
    read_value<int64_t>("vkey", _x.vkey, _j, true);
    read_value<int64_t>("key", _x.key, _j, true);
    read_value<int64_t>("pad", _x.pad, _j, true);
}

inline void to_json(json& _j, const struct KeyMap& _x)
{
    _j = json::object();
    _j["action_name"] = _x.actionName;
    _j["vkey"] = _x.vkey;
    _j["key"] = _x.key;
    _j["pad"] = _x.pad;
}

inline void from_json(const json& _j, struct KeyConfig& _x)
{
    read_value<std::vector<struct KeyMap>>("key_maps", _x.keyMaps, _j);
}

inline void to_json(json& _j, const struct KeyConfig& _x)
{
    _j = json::object();
    _j["key_maps"] = _x.keyMaps;
}

inline void from_json(const json& _j, struct Options& _x)
{
    read_value<bool>("hide_mouse_cursor", _x.hideMouseCursor, _j);
    read_value<bool>("save_log_file", _x.saveLogFile, _j);
}

inline void to_json(json& _j, const struct Options& _x)
{
    _j = json::object();
    _j["hide_mouse_cursor"] = _x.hideMouseCursor;
    _j["save_log_file"] = _x.saveLogFile;
}

inline void from_json(const json& _j, struct WindowConfig& _x)
{
    read_value<bool>("full_screen", _x.fullScreen, _j);
    read_value<int64_t>("window_width", _x.windowWidth, _j);
    read_value<int64_t>("window_height", _x.windowHeight, _j);
}

inline void to_json(json& _j, const struct WindowConfig& _x)
{
    _j = json::object();
    _j["full_screen"] = _x.fullScreen;
    _j["window_width"] = _x.windowWidth;
    _j["window_height"] = _x.windowHeight;
}

inline void from_json(const json& _j, struct BstormConfig& _x)
{
    read_value<std::string>("generated_by", _x.generatedBy, _j);
    read_value<struct KeyConfig>("key_config", _x.keyConfig, _j);
    read_value<struct Options>("options", _x.options, _j);
    read_value<struct WindowConfig>("window_config", _x.windowConfig, _j);
}

inline void to_json(json& _j, const struct BstormConfig& _x)
{
    _j = json::object();
    _j["generated_by"] = _x.generatedBy;
    _j["key_config"] = _x.keyConfig;
    _j["options"] = _x.options;
    _j["window_config"] = _x.windowConfig;
}
}

conf::BstormConfig LoadBstormConfig(const std::string& path, bool isBinaryFormat);
bool SaveBstormConfig(const std::string& path, bool isBinaryFormat, conf::BstormConfig config);
}

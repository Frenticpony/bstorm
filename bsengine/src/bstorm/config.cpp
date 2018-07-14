#include <bstorm/config.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/version.hpp>

#include "../../json/single_include/nlohmann/json.hpp"

#include <fstream>
#include <cstdio>
#include <windows.h>

namespace bstorm
{
namespace conf
{
using nlohmann::json;

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

conf::BstormConfig LoadBstormConfig(const std::string & path, bool isBinaryFormat)
{
    conf::BstormConfig config;
    try
    {
        if (FILE* fp = fopen(path.c_str(), "r"))
        {
            fclose(fp);
            if (isBinaryFormat)
            {
                std::ifstream configFileIn(path, std::ios::binary);
                config = conf::json::from_msgpack(configFileIn);
                configFileIn.close();
            } else
            {
                std::ifstream configFileIn(path);
                conf::json j;
                configFileIn >> j;
                config = j;
                configFileIn.close();
            }
        } else
        {
            Logger::WriteLog(Log::Level::LV_WARN, "can't open config file, load default settings.");
        }
    } catch (...)
    {
        Logger::WriteLog(Log::Level::LV_WARN, "failed to load config file, load default settings.");
    }
    return config;
}

bool SaveBstormConfig(const std::string & path, bool isBinaryFormat, conf::BstormConfig config)
{
    config.generatedBy = BSTORM_VERSION;
    if (isBinaryFormat)
    {
        std::ofstream configFileOut(path, std::ios::binary);
        if (!configFileOut.good()) return false;
        auto data = conf::json::to_msgpack(config);
        configFileOut.write((char*)data.data(), data.size());
    } else
    {
        std::ofstream configFileOut(path);
        if (!configFileOut.good()) return false;
        configFileOut << std::setw(4) << conf::json(config) << std::endl;
    }
    return true;
}
}

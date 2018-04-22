#include <bstorm/config.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/version.hpp>

#include <fstream>
#include <cstdio>
#include <windows.h>

namespace bstorm
{
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

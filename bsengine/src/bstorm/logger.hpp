#pragma once

#include <bstorm/source_map.hpp>

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <mutex>
#include <windows.h>

namespace bstorm
{
class Log
{
public:
    enum class Level
    {
        LV_INFO,
        LV_WARN,
        LV_ERROR,
        LV_SUCCESS,
        LV_DETAIL,
        LV_DEBUG,
        LV_USER
    };
    static const char* getLevelName(Level level);
    class Param
    {
    public:
        enum class Tag
        {
            TEXT,
            TEXTURE,
            SOUND,
            MESH,
            PLAYER_SHOT_DATA,
            ENEMY_SHOT_DATA,
            ITEM_DATA,
            SCRIPT,
            RENDER_TARGET,
            SHADER
        };
        Param(Tag tag, const std::string& text);
        Param(Tag tag, const std::wstring& text);
        Param(Tag tag, std::string&& text);
        Tag getTag() const { return tag; }
        const std::string& getText() const { return text; }
    private:
        Tag tag;
        std::string text; // utf8
    };
    Log();
    Log(Level level);
    Log& setMessage(const std::string& text);
    Log& setMessage(const std::wstring& text);
    Log& setMessage(std::string&& text);
    Log& setParam(const Param& param);
    Log& setParam(Param&& param);
    Log& addSourcePos(const std::shared_ptr<SourcePos>& srcPos);
    Log& setLevel(Level level);
    std::string ToString() const;
    Level getLevel() const { return level; }
    const std::string& getMessage() const { return msg; }
    const std::shared_ptr<Param>& getParam() const { return param; }
    const std::vector<SourcePos>& getSourcePosStack() const { return srcPosStack; }
private:
    Level level;
    std::string msg; // utf8
    std::shared_ptr<Param> param; // Nullable
    std::vector<SourcePos> srcPosStack; // add = push_back
};


class Logger
{
public:
    static void Init(const std::shared_ptr<Logger>& logger);
    static void WriteLog(Log& lg);
    static void WriteLog(Log&& lg);
    static void WriteLog(Log::Level level, const std::string& text);
    static void WriteLog(Log::Level level, const std::wstring& text);
    static void WriteLog(Log::Level level, std::string&& text);
    static void Shutdown();
    static void SetEnable(bool enable);
    virtual ~Logger() {}
    virtual void log(Log& lg) = 0;
    virtual void log(Log&& lg) = 0;
    void log(Log::Level level, const std::string& text);
    void log(Log::Level level, const std::wstring& text);
    void log(Log::Level level, std::string&& text);
private:
    static std::shared_ptr<Logger> logger;
    static std::mutex mutex;
    static bool logEnabled;
};

class DummyLogger : public Logger
{
public:
    DummyLogger() {}
private:
    void log(Log& lg) override
    {
        log(std::move(Log(lg)));
    };
    void log(Log&& lg) override
    {
        OutputDebugStringA(lg.ToString().c_str());
        OutputDebugStringA("\n");
    };
};

class FileLogger : public Logger
{
public:
    FileLogger(const std::wstring& filePath, const std::shared_ptr<Logger>& cc);
    ~FileLogger() override;
    void log(Log& lg) override;
    void log(Log&& lg) override;
private:
    std::ofstream file;
    std::shared_ptr<Logger> cc;
};
}
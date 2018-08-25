#pragma once

#include <bstorm/source_map.hpp>

#include <string>
#include <memory>
#include <vector>
#include <mutex>

namespace bstorm
{
class LogParam
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
    LogParam(Tag tag, const std::string& text);
    LogParam(Tag tag, const std::wstring& text);
    LogParam(Tag tag, std::string&& text);
    Tag GetTag() const { return tag_; }
    const std::string& GetText() const { return text_; }
private:
    Tag tag_;
    std::string text_; // utf8
};

enum class LogLevel
{
    LV_INFO,
    LV_WARN,
    LV_ERROR,
    LV_SUCCESS,
    LV_DEBUG,
    LV_USER
};

class Log
{
public:
    static const char* GetLevelName(LogLevel level);
    Log();
    Log(LogLevel level);
    Log& Msg(const std::string& text);
    Log& Msg(const std::wstring& text);
    Log& Msg(std::string&& text);
    const std::string& Msg() const { return msg_; }
    Log& Param(const LogParam& param);
    Log& Param(LogParam&& param);
    const std::shared_ptr<LogParam>& Param() const { return param_; }
    Log& Level(LogLevel level);
    LogLevel Level() const { return level_; }
    Log& AddSourcePos(const std::shared_ptr<SourcePos>& srcPos);
    const std::vector<SourcePos>& GetSourcePosStack() const { return srcPosStack_; }
    std::string ToString() const;
    Log&& move() { return std::move(*this); }
private:
    LogLevel level_;
    std::string msg_; // utf8
    std::shared_ptr<LogParam> param_; // Nullable
    std::vector<SourcePos> srcPosStack_; // add = push_back
};

Log LogInfo(const std::wstring& msg);
Log LogInfo(const std::string& msg);
Log LogInfo(std::string&& msg);

Log LogWarn(const std::wstring& msg);
Log LogWarn(const std::string& msg);
Log LogWarn(std::string&& msg);

Log LogError(const std::wstring& msg);
Log LogError(const std::string& msg);
Log LogError(std::string&& msg);

Log LogDebug(const std::wstring& msg);
Log LogDebug(const std::string& msg);
Log LogDebug(std::string&& msg);

class Logger
{
public:
    static void Init(const std::shared_ptr<Logger>& logger);
    static void Write(Log& lg);
    static void Write(Log&& lg);
    static void Write(LogLevel level, const std::string& text);
    static void Write(LogLevel level, const std::wstring& text);
    static void Write(LogLevel level, std::string&& text);
    static void Shutdown();
    virtual ~Logger() {}
    virtual void log(Log& lg) noexcept(false) = 0;
    virtual void log(Log&& lg) noexcept(false) = 0;
    void log(LogLevel level, const std::string& text);
    void log(LogLevel level, const std::wstring& text);
    void log(LogLevel level, std::string&& text);
private:
    static std::shared_ptr<Logger> logger;
    static std::mutex mutex;
};
}
#pragma once

#include <bstorm/source_map.hpp>

#include <string>
#include <memory>
#include <vector>
#include <mutex>

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
        LV_DEBUG,
        LV_USER
    };
    static const char* GetLevelName(Level level);
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
        Tag GetTag() const { return tag_; }
        const std::string& GetText() const { return text_; }
    private:
        Tag tag_;
        std::string text_; // utf8
    };
    Log();
    Log(Level level);
    Log& SetMessage(const std::string& text);
    Log& SetMessage(const std::wstring& text);
    Log& SetMessage(std::string&& text);
    Log& SetParam(const Param& param);
    Log& SetParam(Param&& param);
    Log& AddSourcePos(const std::shared_ptr<SourcePos>& srcPos);
    Log& SetLevel(Level level);
    std::string ToString() const;
    Level GetLevel() const { return level_; }
    const std::string& GetMessage() const { return msg_; }
    const std::shared_ptr<Param>& GetParam() const { return param_; }
    const std::vector<SourcePos>& GetSourcePosStack() const { return srcPosStack_; }
private:
    Level level_;
    std::string msg_; // utf8
    std::shared_ptr<Param> param_; // Nullable
    std::vector<SourcePos> srcPosStack_; // add = push_back
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
    virtual void log(Log& lg) noexcept(false) = 0;
    virtual void log(Log&& lg) noexcept(false) = 0;
    void log(Log::Level level, const std::string& text);
    void log(Log::Level level, const std::wstring& text);
    void log(Log::Level level, std::string&& text);
private:
    static std::shared_ptr<Logger> logger;
    static std::mutex mutex;
    static bool logEnabled;
};
}
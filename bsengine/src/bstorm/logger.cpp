#include <bstorm/logger.hpp>

#include <bstorm/string_util.hpp>
#include <bstorm/dummy_logger.hpp>

namespace bstorm
{
void Logger::Write(Log& lg)
{
    std::lock_guard<std::mutex> lock(mutex);
    logger->log(lg);
}

void Logger::Write(Log&& lg)
{
    std::lock_guard<std::mutex> lock(mutex);
    logger->log(std::move(lg));
}

void Logger::Write(LogLevel level, const std::string & text)
{
    std::lock_guard<std::mutex> lock(mutex);
    logger->log(level, text);
}

void Logger::Write(LogLevel level, const std::wstring & text)
{
    std::lock_guard<std::mutex> lock(mutex);
    logger->log(level, ToUTF8(text));
}

void Logger::Write(LogLevel level, std::string && text)
{
    std::lock_guard<std::mutex> lock(mutex);
    logger->log(level, std::move(text));
}

void Logger::Init(const std::shared_ptr<Logger>& l)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (l == nullptr)
    {
        logger = std::make_shared<DummyLogger>();
    } else
    {
        logger = l;
    }
}

void Logger::Shutdown()
{
    std::lock_guard<std::mutex> lock(mutex);
    logger.reset();
}

void Logger::log(LogLevel level, const std::string & text)
{
    log(std::move(Log(level).Msg(text)));
}

void Logger::log(LogLevel level, const std::wstring & text)
{
    log(level, ToUTF8(text));
}

void Logger::log(LogLevel level, std::string && text)
{
    log(std::move(Log(level).Msg(std::move(text))));
}

std::shared_ptr<Logger> Logger::logger = std::make_shared<DummyLogger>();
std::mutex Logger::mutex;

LogParam::LogParam(Tag tag, const std::string & text) :
    tag_(tag),
    text_(text)
{
}

LogParam::LogParam(Tag tag, const std::wstring & text) :
    tag_(tag),
    text_(ToUTF8(text))
{
}

LogParam::LogParam(Tag tag, std::string && text) :
    tag_(tag),
    text_(std::move(text))
{
}

const char * Log::GetLevelName(LogLevel level)
{
    switch (level)
    {
        case LogLevel::LV_INFO: return "info";
        case LogLevel::LV_WARN: return "warn";
        case LogLevel::LV_ERROR: return "error";
        case LogLevel::LV_SUCCESS: return "success";
        case LogLevel::LV_DEBUG: return "debug";
        case LogLevel::LV_USER: return "user";
    }
    return "???";
}

Log::Log() :
    level_(LogLevel::LV_INFO)
{
}

Log::Log(LogLevel level) : level_(level)
{
}

Log & Log::Msg(const std::string & text)
{
    msg_ = text;
    return *this;
}

Log & Log::Msg(const std::wstring & text)
{
    return Msg(ToUTF8(text));
}

Log & Log::Msg(std::string && text)
{
    msg_ = std::move(text);
    return *this;
}

Log & Log::Param(const LogParam & param)
{
    this->param_ = std::make_shared<LogParam>(param);
    return *this;
}

Log & Log::Param(LogParam && param)
{
    this->param_ = std::make_shared<LogParam>(std::move(param));
    return *this;
}

Log & Log::AddSourcePos(const std::shared_ptr<SourcePos>& srcPos)
{
    if (srcPos) srcPosStack_.push_back(*srcPos);
    return *this;
}

Log & Log::Level(LogLevel level)
{
    this->level_ = level;
    return *this;
}

std::string Log::ToString() const
{
    std::string s;
    s += msg_;
    if (param_)
    {
        s += " [" + param_->GetText() + "]";
    }
    if (!srcPosStack_.empty())
    {
        s += " @ " + srcPosStack_[0].ToString();
    }
    return s;
}

Log LogInfo(const std::wstring& msg)
{
    return Log(LogLevel::LV_INFO)
        .Msg(msg);
}
Log LogInfo(const std::string& msg)
{
    return Log(LogLevel::LV_INFO)
        .Msg(msg);
}
Log LogInfo(std::string&& msg)
{
    return Log(LogLevel::LV_INFO)
        .Msg(std::move(msg));
}

Log LogWarn(const std::wstring& msg)
{
    return Log(LogLevel::LV_WARN)
        .Msg(msg);
}
Log LogWarn(const std::string& msg)
{
    return Log(LogLevel::LV_WARN)
        .Msg(msg);
}
Log LogWarn(std::string&& msg)
{
    return Log(LogLevel::LV_WARN)
        .Msg(std::move(msg));
}

Log LogError(const std::wstring& msg)
{
    return Log(LogLevel::LV_ERROR)
        .Msg(msg);
}
Log LogError(const std::string& msg)
{
    return Log(LogLevel::LV_ERROR)
        .Msg(msg);
}
Log LogError(std::string&& msg)
{
    return Log(LogLevel::LV_ERROR)
        .Msg(std::move(msg));
}

Log LogDebug(const std::wstring& msg)
{
    return Log(LogLevel::LV_DEBUG)
        .Msg(msg);
}
Log LogDebug(const std::string& msg)
{
    return Log(LogLevel::LV_DEBUG)
        .Msg(msg);
}
Log LogDebug(std::string&& msg)
{
    return Log(LogLevel::LV_DEBUG)
        .Msg(std::move(msg));
}
}
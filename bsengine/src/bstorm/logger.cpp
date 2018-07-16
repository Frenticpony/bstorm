#include <bstorm/logger.hpp>

#include <bstorm/string_util.hpp>

namespace bstorm
{
void Logger::WriteLog(Log& lg)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(lg);
}

void Logger::WriteLog(Log&& lg)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(std::move(lg));
}

void Logger::WriteLog(Log::Level level, const std::string & text)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(level, text);
}

void Logger::WriteLog(Log::Level level, const std::wstring & text)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(level, ToUTF8(text));
}

void Logger::WriteLog(Log::Level level, std::string && text)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(level, std::move(text));
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

void Logger::SetEnable(bool enable)
{
    std::lock_guard<std::mutex> lock(mutex);
    logEnabled = enable;
}

void Logger::log(Log::Level level, const std::string & text)
{
    log(std::move(Log(level).SetMessage(text)));
}

void Logger::log(Log::Level level, const std::wstring & text)
{
    log(level, ToUTF8(text));
}

void Logger::log(Log::Level level, std::string && text)
{
    log(std::move(Log(level).SetMessage(std::move(text))));
}

std::shared_ptr<Logger> Logger::logger = std::make_shared<DummyLogger>();
std::mutex Logger::mutex;
bool Logger::logEnabled = true;

Log::Param::Param(Tag tag, const std::string & text) :
    tag_(tag),
    text_(text)
{
}

Log::Param::Param(Tag tag, const std::wstring & text) :
    tag_(tag),
    text_(ToUTF8(text))
{
}

Log::Param::Param(Tag tag, std::string && text) :
    tag_(tag),
    text_(std::move(text))
{
}

const char * Log::GetLevelName(Level level)
{
    switch (level)
    {
        case Log::Level::LV_INFO: return "info";
        case Log::Level::LV_WARN: return "warn";
        case Log::Level::LV_ERROR: return "error";
        case Log::Level::LV_SUCCESS: return "success";
        case Log::Level::LV_DETAIL: return "detail";
        case Log::Level::LV_DEBUG: return "debug";
        case Log::Level::LV_USER: return "user";
    }
    return "???";
}

Log::Log() :
    level_(Level::LV_INFO)
{
}

Log::Log(Level level) : level_(level)
{
}

Log & Log::SetMessage(const std::string & text)
{
    msg_ = text;
    return *this;
}

Log & Log::SetMessage(const std::wstring & text)
{
    return SetMessage(ToUTF8(text));
}

Log & Log::SetMessage(std::string && text)
{
    msg_ = std::move(text);
    return *this;
}

Log & Log::SetParam(const Param & param)
{
    this->param_ = std::make_shared<Param>(param);
    return *this;
}

Log & Log::SetParam(Param && param)
{
    this->param_ = std::make_shared<Param>(std::move(param));
    return *this;
}

Log & Log::AddSourcePos(const std::shared_ptr<SourcePos>& srcPos)
{
    if (srcPos) srcPosStack_.push_back(*srcPos);
    return *this;
}

Log & Log::SetLevel(Level level)
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

FileLogger::FileLogger(const std::wstring & filePath, const std::shared_ptr<Logger>& cc) :
    cc_(cc)
{
    file_.open(filePath, std::ios::app);
    if (!file_.good())
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("can't open log file.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, filePath));
    }
}

FileLogger::~FileLogger()
{
    file_.close();
}

static void logToFile(const Log& lg, std::ofstream& file)
{
    if (lg.GetLevel() == Log::Level::LV_USER) return;
    {
        // date
        std::string date(23, '\0');
        SYSTEMTIME st;
        GetLocalTime(&st);
        sprintf(&date[0], "%04d/%02d/%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        file << date + " ";
    }
    {
        // level tag
        file << "[" << Log::GetLevelName(lg.GetLevel()) << "] ";
    }
    file << lg.ToString() << std::endl;
}

void FileLogger::log(Log& lg) noexcept(false)
{
    logToFile(lg, file_);
    if (cc_) cc_->log(lg);
}

void FileLogger::log(Log&& lg) noexcept(false)
{
    logToFile(lg, file_);
    if (cc_) cc_->log(std::move(lg));
}
}
#include <bstorm/file_logger.hpp>

#include <windows.h>

namespace bstorm
{
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
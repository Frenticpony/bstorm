#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>

namespace bstorm {
  void Logger::WriteLog(Log& lg) {
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(lg);
  }

  void Logger::WriteLog(Log&& lg) {
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(std::move(lg));
  }

  void Logger::WriteLog(Log::Level level, const std::string & text) {
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(level, text);
  }

  void Logger::WriteLog(Log::Level level, const std::wstring & text) {
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(level, toUTF8(text));
  }

  void Logger::WriteLog(Log::Level level, std::string && text) {
    std::lock_guard<std::mutex> lock(mutex);
    if (logEnabled) logger->log(level, std::move(text));
  }

  void Logger::Init(const std::shared_ptr<Logger>& l) {
    std::lock_guard<std::mutex> lock(mutex);
    logger = l;
  }

  void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex);
    logger.reset();
  }

  void Logger::SetEnable(bool enable) {
    std::lock_guard<std::mutex> lock(mutex);
    logEnabled = enable;
  }

  void Logger::log(Log::Level level, const std::string & text) {
    log(std::move(Log(level).setMessage(text)));
  }

  void Logger::log(Log::Level level, const std::wstring & text) {
    log(level, toUTF8(text));
  }

  void Logger::log(Log::Level level, std::string && text) {
    log(std::move(Log(level).setMessage(std::move(text))));
  }

  std::shared_ptr<Logger> Logger::logger;
  std::mutex Logger::mutex;
  bool Logger::logEnabled = true;

  Log::Param::Param(Tag tag, const std::string & text) :
    tag(tag),
    text(text)
  {
  }

  Log::Param::Param(Tag tag, const std::wstring & text) :
    tag(tag),
    text(toUTF8(text))
  {
  }

  Log::Param::Param(Tag tag, std::string && text) :
    tag(tag),
    text(std::move(text))
  {
  }

  const char * Log::getLevelName(Level level) {
    switch (level) {
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
    level(Level::LV_INFO)
  {
  }

  Log::Log(Level level) : level(level) {
  }

  Log & Log::setMessage(const std::string & text) {
    msg = text;
    return *this;
  }

  Log & Log::setMessage(const std::wstring & text) {
    return setMessage(toUTF8(text));
  }

  Log & Log::setMessage(std::string && text) {
    msg = std::move(text);
    return *this;
  }

  Log & Log::setParam(const Param & param) {
    this->param = std::make_shared<Param>(param);
    return *this;
  }

  Log & Log::setParam(Param && param) {
    this->param = std::make_shared<Param>(std::move(param));
    return *this;
  }

  Log & Log::addSourcePos(const std::shared_ptr<SourcePos>& srcPos) {
    if (srcPos) srcPosStack.push_back(*srcPos);
    return *this;
  }

  Log & Log::setLevel(Level level) {
    this->level = level;
    return *this;
  }

  std::string Log::toString() const {
    std::string s;
    s += msg;
    if (param) {
      s += " [" + param->getText() + "]";
    }
    if (!srcPosStack.empty()) {
      s += " @ " + srcPosStack[0].toString();
    }
    return s;
  }

  FileLogger::FileLogger(const std::wstring & filePath, const std::shared_ptr<Logger>& cc) :
    cc(cc)
  {
    file.open(filePath, std::ios::app);
    if (!file.good()) {
      throw Log(Log::Level::LV_ERROR)
        .setMessage("can't open log file.")
        .setParam(Log::Param(Log::Param::Tag::TEXT, filePath));
    }
  }

  FileLogger::~FileLogger() {
    file.close();
  }

  static void logToFile(const Log& lg, std::ofstream& file) {
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
      file << "[" << Log::getLevelName(lg.getLevel()) << "] ";
    }
    file << lg.toString() << std::endl;
  }

  void FileLogger::log(Log& lg) {
    logToFile(lg, file);
    if (cc) cc->log(lg);
  }

  void FileLogger::log(Log&& lg) {
    logToFile(lg, file);
    if (cc) cc->log(std::move(lg));
  }
}
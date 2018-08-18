#pragma once

#include <bstorm/logger.hpp>

#include <fstream>

namespace bstorm
{
class FileLogger : public Logger
{
public:
    FileLogger(const std::wstring& filePath, const std::shared_ptr<Logger>& cc);
    ~FileLogger() override;
    void log(Log& lg) noexcept(false) override;
    void log(Log&& lg) noexcept(false) override;
private:
    std::ofstream file_;
    std::shared_ptr<Logger> cc_;
};
}

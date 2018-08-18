#pragma once

#include <bstorm/logger.hpp>

#include <windows.h>

namespace bstorm
{
class DummyLogger : public Logger
{
public:
    DummyLogger() {}
private:
    void log(Log& lg) noexcept(false) override
    {
        log(std::move(Log(lg)));
    };
    void log(Log&& lg) noexcept(false) override
    {
        OutputDebugStringA(lg.ToString().c_str());
        OutputDebugStringA("\n");
    };
};
}
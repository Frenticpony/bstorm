#pragma once

#include <future>
#include <vector>

namespace bstorm
{
template <class Fn>
void ParallelTimes(int loopCnt, Fn&& func)
{
    auto coreCnt = std::max((int)std::thread::hardware_concurrency(), 1);
    std::vector<std::future<void>> workers;
    workers.reserve(coreCnt);
    for (int coreId = 0; coreId < coreCnt; ++coreId)
    {
        workers.emplace_back(std::async([&](int id)
        {
            const int begin = loopCnt / coreCnt * id + std::min(loopCnt % coreCnt, id);
            const int end = loopCnt / coreCnt * (id + 1) + std::min(loopCnt % coreCnt, id + 1);
            for (int i = begin; i < end; ++i) { func(i); }
            return;
        }, coreId));
    }
    for (const auto& worker : workers) { worker.wait(); }
}
}
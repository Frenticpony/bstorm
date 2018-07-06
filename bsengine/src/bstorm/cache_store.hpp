#pragma once

#include <memory>
#include <future>
#include <unordered_map>

namespace bstorm
{
enum class CacheMode
{
    REF_COUNT,
    PERSISTENT
};

// SharedCache: flyweight pattern, 非同期ロード機能付き
// NOTE: メインスレッド終了前に破棄しなければならない

// K: require operator==, copyable, moveable
// V: require GetCacheMode, SetCacheMode
template <class K, class V>
class CacheStore
{
public:
    template <class... Args>
    std::shared_ptr<V> Get(const K& key)
    {
        auto it = cacheMap_.find(key);
        if (it != cacheMap_.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    template <class... Args>
    std::shared_ptr<V> Load(const K& key, CacheMode mode, Args&&... args)
    {
        return LoadAsync(key, mode, std::forward<Args>(args)...).get();
    }

    template <class... Args>
    std::shared_future<std::shared_ptr<V>> LoadAsync(const K& key, CacheMode mode, Args&&... args)
    {
        auto it = cacheMap_.find(key);
        if (it != cacheMap_.end())
        {
            auto& cache = it->second.get();
            if (mode == CacheMode::PERSISTENT)
            {
                cache->SetCacheMode(mode);
            }
            return it->second;
        }

        return cacheMap_[key] = std::async(std::launch::async | std::launch::deferred, [key, mode, args...]()
        {
            return std::make_shared<V>(key, mode, args...);
        }).share();
    }

    void Remove(const K& key)
    {
        cacheMap_.erase(key);
    }

    void RemoveAll()
    {
        // 中身を空にしてメモリを解放
        std::unordered_map<K, std::shared_future<std::shared_ptr<V>>>().swap(cacheMap_);
    }

    void RemoveUnused()
    {
        auto it = cacheMap_.begin();
        while (it != cacheMap_.end())
        {
            try
            {
                auto& cache = it->second.get();
                if (cache->GetCacheMode() == CacheMode::REF_COUNT && cache.use_count() <= 1)
                {
                    cacheMap_.erase(it++);
                } else ++it;
            } catch (...)
            {
                ++it;
            }
        }
    }

    const std::unordered_map<K, std::shared_future<std::shared_ptr<V>>>& GetCacheMap() { return cacheMap_; }
private:
    std::unordered_map<K, std::shared_future<std::shared_ptr<V>>> cacheMap_;
};
};
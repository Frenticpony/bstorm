#pragma once

#include <vector>
#include <string>

namespace bstorm
{
class WaveSampleStream
{
public:
    virtual ~WaveSampleStream() {};

    virtual size_t GetTotalBytes() const = 0;
    virtual size_t GetSampleRate() const = 0;
    virtual size_t GetBitsPerSample() const = 0;
    virtual size_t GetChannelCount() const = 0;
    virtual size_t GetFormatTag() const = 0;
    virtual size_t GetByteRate() const = 0;
    virtual size_t GetBlockAlign() const = 0;
    size_t GetBytesPerSample() const
    {
        return GetBitsPerSample() / 8;
    }
    size_t GetTotalSampleCount() const
    {
        return GetTotalBytes() / GetBytesPerSample() / GetChannelCount();
    }
    virtual const std::wstring& GetPath() const = 0;

    virtual void SeekBytes(size_t pos) = 0;
    void SeekSamples(size_t samplePos)
    {
        SeekBytes(samplePos * GetBytesPerSample() * GetChannelCount());
    }
    virtual size_t ReadBytes(size_t byte, char* dst) = 0;
    const size_t ReadSamples(size_t sampleCnt, char* dst)
    {
        return ReadBytes(sampleCnt * GetBytesPerSample() * GetChannelCount(), dst) / GetBytesPerSample() / GetChannelCount();
    }
    virtual size_t Tell() = 0;
    virtual bool IsEnd() = 0;
    virtual bool IsClosed() const = 0;
};
}

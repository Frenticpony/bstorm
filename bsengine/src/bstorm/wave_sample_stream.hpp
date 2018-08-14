#pragma once

#include <bstorm/wave_format.hpp>

#include <vector>
#include <string>

namespace bstorm
{
class WaveSampleStream
{
public:
    virtual ~WaveSampleStream() {};

    size_t GetTotalBytes() const { return waveFormat_.totalBytes; }
    size_t GetSampleRate() const { return waveFormat_.sampleRate; }
    size_t GetBitsPerSample() const { return waveFormat_.bitsPerSample; }
    size_t GetChannelCount() const { return waveFormat_.channelCount; }
    size_t GetFormatTag() const { return waveFormat_.formatTag; }
    size_t GetByteRate() const { return waveFormat_.byteRate; }
    size_t GetBlockAlign() const { return waveFormat_.blockAlign; }
    const WaveFormat& GetWaveFormat() const { return waveFormat_; }

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
protected:
    WaveFormat waveFormat_;
};
}

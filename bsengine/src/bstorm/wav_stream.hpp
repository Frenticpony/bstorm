#pragma once

#include <bstorm/wave_sample_stream.hpp>

#include <memory>
#include <string>
#include <fstream>

namespace bstorm
{
class WavStream : public WaveSampleStream
{
public:
    WavStream(const std::wstring& path);
    ~WavStream();

    const std::wstring& GetPath() const override;

    void SeekBytes(size_t pos) override;
    size_t ReadBytes(size_t byte, char* dst) override;
    size_t Tell() override;
    bool IsClosed() const override;
private:
    struct WavRiffHeader
    {
        char chunkID[4];
        int chunkSize;
        char format[4];
    } riffHeader_;

    struct WavFormatChunk
    {
        char chunkID[4];
        int chunkSize;
        short formatTag;
        short channelCnt;
        int samplePerSec;
        int bytePerSec;
        short blockAlign;
        short bitsPerSample;
    } formatChunk_;

    struct WavDataChunk
    {
        char chunkID[4];
        int chunkSize;
    } dataChunk_;

    size_t headerSize_;
    const std::wstring path_;
    std::ifstream fileStream_;
};
}
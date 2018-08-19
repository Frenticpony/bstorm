#include <bstorm/wav_stream.hpp>
#include <bstorm/logger.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdio>

namespace bstorm
{
WavStream::WavStream(const std::wstring & path) :
    headerSize_(0),
    path_(path)
{
    fileStream_.open(path, std::ios::binary | std::ios::in);
    if (!fileStream_.good())
    {
        throw Log(LogLevel::LV_ERROR)
            .Msg("can't open file")
            .Param(LogParam(LogParam::Tag::TEXT, path));
    }

    auto illegal_wave_format = Log(LogLevel::LV_ERROR)
        .Msg("illegal wave format file.")
        .Param(LogParam(LogParam::Tag::TEXT, path));

    // read riff header
    fileStream_.read((char*)&riffHeader_, sizeof(riffHeader_));
    if (!fileStream_.good() || strncmp(riffHeader_.chunkID, "RIFF", 4) || strncmp(riffHeader_.format, "WAVE", 4))
    {
        throw illegal_wave_format;
    }

    // read format chunk
    fileStream_.read((char*)&formatChunk_, sizeof(formatChunk_));
    if (!fileStream_.good() || strncmp(formatChunk_.chunkID, "fmt ", 4))
    {
        throw illegal_wave_format;
    }

    size_t extFormatInfoSize = formatChunk_.chunkSize + sizeof(formatChunk_.chunkID) + sizeof(formatChunk_.chunkSize) - sizeof(formatChunk_);
    fileStream_.ignore(extFormatInfoSize);

    headerSize_ = sizeof(riffHeader_) + sizeof(formatChunk_) + extFormatInfoSize;

    // skip until data chunk is found
    while (true)
    {
        fileStream_.read((char*)&dataChunk_, sizeof(dataChunk_));
        if (!fileStream_.good()) throw illegal_wave_format;
        headerSize_ += sizeof(dataChunk_);
        if (strncmp(dataChunk_.chunkID, "data", 4) == 0)
        {
            break;
        }
        fileStream_.seekg(dataChunk_.chunkSize, std::ios_base::cur);
        headerSize_ += dataChunk_.chunkSize;
    }

    waveFormat_.totalBytes = dataChunk_.chunkSize;
    waveFormat_.sampleRate = formatChunk_.samplePerSec;
    waveFormat_.bitsPerSample = formatChunk_.bitsPerSample;
    waveFormat_.channelCount = formatChunk_.channelCnt;
    waveFormat_.formatTag = formatChunk_.formatTag;
    waveFormat_.byteRate = formatChunk_.bytePerSec;
    waveFormat_.blockAlign = formatChunk_.blockAlign;
}

WavStream::~WavStream()
{
    fileStream_.close();
}

const std::wstring & WavStream::GetPath() const
{
    return path_;
}

void WavStream::SeekBytes(size_t pos)
{
    fileStream_.seekg(headerSize_ + pos, std::ios::beg);
}

size_t WavStream::ReadBytes(size_t byte, char * dst)
{
    auto readSize = std::min(byte, GetTotalBytes() - Tell());
    if (readSize == 0)
    {
        return 0;
    }
    fileStream_.read(dst, readSize);
    return fileStream_.gcount();
}

size_t WavStream::Tell()
{
    return (size_t)fileStream_.tellg() - headerSize_;
}

bool WavStream::IsClosed() const
{
    return fileStream_.fail();
}
}
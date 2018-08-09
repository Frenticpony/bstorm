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
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("can't open file")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
    }

    auto illegal_wave_format = Log(Log::Level::LV_ERROR)
        .SetMessage("illegal wave format file.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, path));

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
}

WavStream::~WavStream()
{
    fileStream_.close();
}

size_t WavStream::GetTotalBytes() const
{
    return dataChunk_.chunkSize;
}
size_t WavStream::GetSampleRate() const
{
    return formatChunk_.samplePerSec;
}
size_t WavStream::GetBitsPerSample() const
{
    return formatChunk_.bitsPerSample;
}
size_t WavStream::GetChannelCount() const
{
    return formatChunk_.channelCnt;
}

size_t WavStream::GetFormatTag() const
{
    return formatChunk_.formatTag;
}

size_t WavStream::GetByteRate() const
{
    return formatChunk_.bytePerSec;
}

size_t WavStream::GetBlockAlign() const
{
    return formatChunk_.blockAlign;
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
    return readSize;
}

size_t WavStream::Tell()
{
    return (size_t)fileStream_.tellg() - headerSize_;
}
bool WavStream::IsEnd()
{
    return Tell() >= GetTotalBytes();
}
bool WavStream::IsClosed() const
{
    return fileStream_.fail();
}
}
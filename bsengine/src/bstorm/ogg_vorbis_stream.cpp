#include <bstorm/ogg_vorbis_stream.hpp>

#include <bstorm/logger.hpp>

#include <windows.h>
#include <mmreg.h>
#include <algorithm>

namespace bstorm
{
static size_t is_read(void* dst, size_t size, size_t nmemb, void* userData)
{
    auto& stream = *(std::istream*)(userData);
    stream.read((char*)dst, size * nmemb);
    return (size_t)stream.gcount();
}

static int is_seek(void* userData, ogg_int64_t offset, int whence)
{
    auto& stream = *(std::istream*)(userData);
    int way = std::ios::beg;
    if (whence == SEEK_SET)
    {
        way = std::ios::beg;
    } else if (whence == SEEK_CUR)
    {
        way = std::ios::cur;
    } else if (whence == SEEK_END)
    {
        way = std::ios::end;
    }
    stream.seekg((size_t)offset, way);
    return 0;
}

static long is_tell(void* userData)
{
    auto& stream = *(std::istream*)(userData);
    return  (long)stream.tellg();
}

OggVorbisStream::OggVorbisStream(const std::wstring & path) :
    path_(path)
{
    fileStream_.open(path, std::ios::binary | std::ios::in);
    if (!fileStream_.good())
    {
        throw Log(LogLevel::LV_ERROR)
            .Msg("Unable to open file.")
            .Param(LogParam(LogParam::Tag::TEXT, path));
    }

    ov_callbacks cbs{ is_read, is_seek, nullptr, is_tell };
    if (ov_open_callbacks(&fileStream_, &ovFile_, 0, 0, cbs) != 0)
    {
        throw Log(LogLevel::LV_ERROR)
            .Msg("Illegal ogg vorbis format file.")
            .Param(LogParam(LogParam::Tag::TEXT, path));
    }

    info_ = ov_info(&ovFile_, -1);
    waveFormat_.totalBytes = (size_t)(ov_pcm_total(&ovFile_, -1)) * info_->channels * 2;
    waveFormat_.sampleRate = info_->rate;
    waveFormat_.bitsPerSample = 16;
    waveFormat_.channelCount = info_->channels;
    waveFormat_.formatTag = WAVE_FORMAT_PCM;
    waveFormat_.byteRate = info_->rate * info_->channels * 2;
    waveFormat_.blockAlign = info_->channels * 2;
}

OggVorbisStream::~OggVorbisStream()
{
    ov_clear(&ovFile_);
    fileStream_.close();
}

const std::wstring & OggVorbisStream::GetPath() const
{
    return path_;
}

void OggVorbisStream::SeekBytes(size_t pos)
{
    ov_pcm_seek(&ovFile_, pos / 2 / GetChannelCount());
}

size_t OggVorbisStream::ReadBytes(size_t requestSize, char * dst)
{
    int bitStream = 0;
    size_t total = 0;
    while (total < requestSize)
    {
        auto size = ov_read(&ovFile_, dst + total, requestSize - total, 0, 2, 1, &bitStream);
        if (size <= 0)
        {
            break;
        }
        total += size;
    }
    return total;
}

size_t OggVorbisStream::Tell()
{
    return (size_t)(ov_pcm_tell(&ovFile_)) * GetChannelCount() * 2;
}

bool OggVorbisStream::IsClosed() const
{
    return fileStream_.fail();
}
}
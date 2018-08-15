#pragma once

#include <bstorm/wave_sample_stream.hpp>

#include <vorbis/vorbisfile.h>

#include <memory>
#include <string>
#include <fstream>

namespace bstorm
{
class OggVorbisStream : public WaveSampleStream
{
public:
    OggVorbisStream(const std::wstring& path);
    ~OggVorbisStream();

    const std::wstring& GetPath() const override;

    void SeekBytes(size_t pos) override;
    size_t ReadBytes(size_t byte, char* dst) override;
    size_t Tell() override;
    bool IsClosed() const override;
private:
    const std::wstring path_;
    std::ifstream fileStream_;
    OggVorbis_File ovFile_;
    vorbis_info* info_;
};
}
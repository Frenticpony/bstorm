#include <bstorm/sound_device.hpp>

#include <bstorm/file_util.hpp>
#include <bstorm/ptr_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/sound_buffer.hpp>
#include <bstorm/wav_stream.hpp>
#include <bstorm/ogg_vorbis_stream.hpp>

namespace bstorm
{
SoundDevice::SoundDevice(HWND hWnd) :
    hWnd_(hWnd),
    dSound_(nullptr)
{
    if (DS_OK != DirectSoundCreate8(nullptr, &dSound_, nullptr))
    {
        throw Log(LogLevel::LV_ERROR)
            .Msg("Failed to initialize sound device.");
    }
    dSound_->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
}

SoundDevice::~SoundDevice()
{
    safe_release(dSound_);
}

std::shared_ptr<SoundBuffer> SoundDevice::LoadSound(const std::wstring & path)
{
    auto uniqPath = GetCanonicalPath(path);
    auto ext = GetLowerExt(uniqPath);
    if (ext == L".wav" || ext == L".wave")
    {
        return std::make_shared<SoundBuffer>(std::make_unique<WavStream>(uniqPath), shared_from_this());
    } else if (ext == L".ogg")
    {
        return std::make_shared<SoundBuffer>(std::make_unique<OggVorbisStream>(uniqPath), shared_from_this());
    }
    throw Log(LogLevel::LV_ERROR)
        .Msg("Unsupported sound file format.")
        .Param(LogParam(LogParam::Tag::TEXT, path));
}
}
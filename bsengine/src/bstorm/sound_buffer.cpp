#include <bstorm/sound_buffer.hpp>

#include <bstorm/logger.hpp>

#include <bstorm/math_util.hpp>
#include <bstorm/ptr_util.hpp>

namespace bstorm
{
static DWORD WINAPI SoundControlThread(LPVOID arg)
{
    SoundBuffer* soundBuffer = (SoundBuffer*)arg;
    while (true)
    {
        HANDLE loopEndEvent = soundBuffer->GetLoopEndEvent();
        auto ret = WaitForSingleObject(loopEndEvent, INFINITE);
        if (ret == WAIT_OBJECT_0)
        {
            if (soundBuffer->IsLoopEnabled())
            {
                soundBuffer->Seek(soundBuffer->GetLoopStartSampleCount());
            }
        } else
        {
            // unexpected sound control error occured.
            return 1;
        }
    }
    return 0;
}

SoundBuffer::SoundBuffer(const std::wstring& path, IDirectSoundBuffer8* buf) :
    path_(path),
    dSoundBuffer_(buf),
    loopEnable_(false),
    loopStartSampleCnt_(0),
    loopEndSampleCnt_(DSBPN_OFFSETSTOP),
    controlThread_(nullptr),
    loopEndEvent_(nullptr)
{
    try
    {
        dSoundBuffer_->SetVolume(DSBVOLUME_MAX);
        dSoundBuffer_->SetPan(DSBPAN_CENTER);

        WAVEFORMATEX waveFormat;
        if (FAILED(dSoundBuffer_->GetFormat(&waveFormat, sizeof(waveFormat), nullptr)))
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to get sound buffer format.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        }
        samplePerSec_ = waveFormat.nSamplesPerSec;
        bytesPerSample_ = waveFormat.wBitsPerSample / 8;
        channelCnt_ = waveFormat.nChannels;

        loopEndEvent_ = CreateEvent(nullptr, TRUE, FALSE, L"LOOP_END");
        if (loopEndEvent_ == nullptr)
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to create loop end event.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        }

        controlThread_ = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SoundControlThread, this, 0, nullptr);
        if (controlThread_ == nullptr)
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("failed to create sound control thread.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
        }
    } catch (...)
    {
        if (loopEndEvent_)
        {
            CloseHandle(loopEndEvent_);
        }
        if (controlThread_)
        {
            TerminateThread(controlThread_, 1);
            CloseHandle(controlThread_);
        }
        safe_release(dSoundBuffer_);
        throw;
    }
}

void SoundBuffer::Play()
{
    dSoundBuffer_->Play(0, 0, loopEnable_ ? DSBPLAY_LOOPING : 0);
}

void SoundBuffer::Stop()
{
    dSoundBuffer_->Stop();
}

void SoundBuffer::SetVolume(float vol)
{
    vol = constrain(vol, 0.0f, 1.0f);
    vol = vol == 0.0f ? DSBVOLUME_MIN : (1000.0f * log10f(vol));
    vol = constrain(vol, 1.0f*DSBVOLUME_MIN, 1.0f*DSBVOLUME_MAX);
    dSoundBuffer_->SetVolume(vol);
}

void SoundBuffer::SetPanRate(float pan)
{
    pan = constrain(pan, -1.0f, 1.0f);
    const bool rightPan = pan > 0;
    pan = 1 - abs(pan); // ’†‰›‚ð1(”{)‚É‚·‚é
    pan = pan == 0 ? DSBPAN_LEFT : (1000.0f * log10f(pan));
    if (rightPan) pan *= -1;
    pan = constrain(pan, 1.0f*DSBPAN_LEFT, 1.0f*DSBPAN_RIGHT);
    dSoundBuffer_->SetPan(pan);
}

void SoundBuffer::Seek(int sample)
{
    dSoundBuffer_->SetCurrentPosition(sample * bytesPerSample_ * channelCnt_);
}

void SoundBuffer::SetLoopEnable(bool enable)
{
    loopEnable_ = enable;
}

void SoundBuffer::SetLoopSampleCount(DWORD start, DWORD end)
{
    loopStartSampleCnt_ = start;
    loopEndSampleCnt_ = end;
    IDirectSoundNotify8* soundNotify = nullptr;
    dSoundBuffer_->QueryInterface(IID_IDirectSoundNotify8, (void**)&soundNotify);
    DSBPOSITIONNOTIFY notifyPos;
    notifyPos.dwOffset = loopEndSampleCnt_ * bytesPerSample_ * channelCnt_;
    notifyPos.hEventNotify = loopEndEvent_;
    soundNotify->SetNotificationPositions(1, &notifyPos);
    safe_release(soundNotify);
}

void SoundBuffer::SetLoopTime(double startSec, double endSec)
{
    SetLoopSampleCount((DWORD)(samplePerSec_ * startSec), (DWORD)(samplePerSec_ * endSec));
}

bool SoundBuffer::IsPlaying()
{
    DWORD status;
    dSoundBuffer_->GetStatus(&status);
    return (status & DSBSTATUS_PLAYING) != 0;
}

float SoundBuffer::GetVolume()
{
    LONG vol;
    dSoundBuffer_->GetVolume(&vol);
    return vol == DSBVOLUME_MIN ? 0.0f : powf(10.0f, vol / 3000.0f);
}

const std::wstring & SoundBuffer::GetPath() const
{
    return path_;
}

bool SoundBuffer::IsLoopEnabled() const
{
    return loopEnable_;
}

HANDLE SoundBuffer::GetLoopEndEvent() const
{
    return loopEndEvent_;
}

DWORD SoundBuffer::GetLoopStartSampleCount() const
{
    return loopStartSampleCnt_;
}

DWORD SoundBuffer::GetLoopEndSampleCount() const
{
    return loopEndSampleCnt_;
}

SoundBuffer::~SoundBuffer()
{
    TerminateThread(controlThread_, 0);
    CloseHandle(controlThread_);
    CloseHandle(loopEndEvent_);
    safe_release(dSoundBuffer_);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("release sound.")
        .SetParam(Log::Param(Log::Param::Tag::SOUND, path_))));
}

}
#include <bstorm/obj_sound.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/sound_device.hpp>
#include <bstorm/game_state.hpp>

namespace bstorm
{
ObjSound::ObjSound(const std::shared_ptr<GameState>& gameState) :
    Obj(gameState),
    soundBuffer_(NULL),
    restartEnable_(false),
    division_(SoundDivision::BGM),
    fadeRatePerSec_(0)
{
    SetType(OBJ_SOUND);
}

ObjSound::~ObjSound()
{
}

void ObjSound::Update()
{
    if (fadeRatePerSec_ != 0)
    {
        if (IsPlaying())
        {
            SetVolumeRate(GetVolumeRate() + fadeRatePerSec_ / 60.0f);
        }
    }
}

void ObjSound::SetSound(const std::shared_ptr<SoundBuffer>& buf)
{
    restartEnable_ = false;
    soundBuffer_ = buf;
}

void ObjSound::Play()
{
    if (soundBuffer_)
    {
        if (IsPlaying() || !restartEnable_)
        {
            soundBuffer_->Seek(0);
        }
        soundBuffer_->Play();
    }
}

void ObjSound::Stop()
{
    if (soundBuffer_)
    {
        soundBuffer_->Stop();
    }
}

void ObjSound::SetVolumeRate(float vol)
{
    if (soundBuffer_)
    {
        soundBuffer_->SetVolume(vol / 100.0f);
    }
}

void ObjSound::SetPanRate(float pan)
{
    if (soundBuffer_)
    {
        soundBuffer_->SetPanRate(pan / 100.0f);
    }
}

void ObjSound::SetFade(float fadeRatePerSec)
{
    this->fadeRatePerSec_ = fadeRatePerSec;
}

void ObjSound::SetLoopEnable(bool enable)
{
    if (soundBuffer_)
    {
        soundBuffer_->SetLoopEnable(enable);
    }
}

void ObjSound::SetLoopTime(double startSec, double endSec)
{
    if (soundBuffer_)
    {
        soundBuffer_->SetLoopTime(startSec, endSec);
    }
}

void ObjSound::SetLoopSampleCount(DWORD startCount, DWORD endCount)
{
    if (soundBuffer_)
    {
        soundBuffer_->SetLoopSampleCount(startCount, endCount);
    }
}

void ObjSound::SetRestartEnable(bool enable)
{
    if (soundBuffer_)
    {
        restartEnable_ = enable;
    }
}

void ObjSound::SetSoundDivision(SoundDivision div)
{
    division_ = div;
}

bool ObjSound::IsPlaying() const
{
    return soundBuffer_ && soundBuffer_->IsPlaying();
}

float ObjSound::GetVolumeRate() const
{
    if (soundBuffer_)
    {
        return 100.0f * soundBuffer_->GetVolume();
    } else
    {
        return 0.0f;
    }
}
}
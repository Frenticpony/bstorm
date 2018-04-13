#include <bstorm/obj_sound.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/sound_device.hpp>
#include <bstorm/game_state.hpp>

namespace bstorm {
  ObjSound::ObjSound(const std::shared_ptr<GameState>& gameState) :
    Obj(gameState),
    soundBuffer(NULL),
    restartEnable(false),
    division(SoundDivision::BGM),
    fadeRatePerSec(0)
  {
    setType(OBJ_SOUND);
  }

  ObjSound::~ObjSound() {
  }

  void ObjSound::update() {
    if (fadeRatePerSec != 0) {
      if (isPlaying()) {
        setVolumeRate(getVolumeRate() + fadeRatePerSec / 60.0f);
      }
    }
  }

  void ObjSound::setSound(const std::shared_ptr<SoundBuffer>& buf) {
    restartEnable = false;
    soundBuffer = buf;
  }

  void ObjSound::play() {
    if (soundBuffer) {
      if (isPlaying() || !restartEnable) {
        soundBuffer->seek(0);
      }
      soundBuffer->play();
    }
  }

  void ObjSound::stop() {
    if (soundBuffer) {
      soundBuffer->stop();
    }
  }

  void ObjSound::setVolumeRate(float vol) {
    if (soundBuffer) {
      soundBuffer->setVolume(vol / 100.0f);
    }
  }

  void ObjSound::setPanRate(float pan) {
    if (soundBuffer) {
      soundBuffer->setPanRate(pan / 100.0f);
    }
  }

  void ObjSound::setFade(float fadeRatePerSec) {
    this->fadeRatePerSec = fadeRatePerSec;
  }

  void ObjSound::setLoopEnable(bool enable) {
    if (soundBuffer) {
      soundBuffer->setLoopEnable(enable);
    }
  }

  void ObjSound::setLoopTime(double startSec, double endSec) {
    if (soundBuffer) {
      soundBuffer->setLoopTime(startSec, endSec);
    }
  }

  void ObjSound::setLoopSampleCount(DWORD startCount, DWORD endCount) {
    if (soundBuffer) {
      soundBuffer->setLoopSampleCount(startCount, endCount);
    }
  }

  void ObjSound::setRestartEnable(bool enable) {
    if (soundBuffer) {
      restartEnable = enable;
    }
  }

  void ObjSound::setSoundDivision(SoundDivision div) {
    division = div;
  }

  bool ObjSound::isPlaying() const {
    return soundBuffer && soundBuffer->isPlaying();
  }

  float ObjSound::getVolumeRate() const {
    if (soundBuffer) {
      return 100.0f * soundBuffer->getVolume();
    } else {
      return 0.0f;
    }
  }
}
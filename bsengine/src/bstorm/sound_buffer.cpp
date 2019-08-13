#include <bstorm/sound_buffer.hpp>

#include <bstorm/ptr_util.hpp>
#include <bstorm/math_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/wave_sample_stream.hpp>
#include <bstorm/sound_device.hpp>

#include <dsound.h>
#include <cassert>

namespace bstorm
{
static inline float TodB(float r)
{
    return 20.0f * log10f(r);
}

static inline float FromdB(float db)
{
    return powf(10.0f, db / 20.0f);
}

constexpr DWORD THREAD_END_EVENT = 0;
constexpr DWORD LOOP_END_EVENT = 1;
// sound thread
static DWORD WINAPI SoundMain(LPVOID p)
{
	SoundBuffer::SoundThreadParam* param = (SoundBuffer::SoundThreadParam*)p;
	while (true)
	{
		auto ret = WaitForMultipleObjects(2, param->soundEvents, FALSE, INFINITE);
		if (ret == WAIT_OBJECT_0 + THREAD_END_EVENT)
		{
			break;
		}
		else if (ret == WAIT_OBJECT_0 + LOOP_END_EVENT)
		{
			if (param->soundBuffer->IsLoopEnabled())
			{
				param->soundBuffer->Seek(param->soundBuffer->GetLoopStartSampleCount());
			}
		}
		else
		{
			// unexpected sound control error occured.
			return 1;
		}
	}
	return 0;
}

static void CloseThread(HANDLE thread, HANDLE threadEndEvent)
{
	SetEvent(threadEndEvent);
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
}

SoundBuffer::SoundBuffer(const std::wstring& path, IDirectSoundBuffer8* buf) :
	threadEndEvent_(nullptr),
	loopEndEvent_(nullptr),
	thread_(nullptr),
	path_(path),
	dSoundBuffer_(buf),
	loopEnable_(false),
	loopStartSampleCnt_(0),
	loopEndSampleCnt_(DSBPN_OFFSETSTOP)
{
	try
	{
		dSoundBuffer_->SetVolume(DSBVOLUME_MAX);
		dSoundBuffer_->SetPan(DSBPAN_CENTER);

		WAVEFORMATEX waveFormat;
		if (FAILED(dSoundBuffer_->GetFormat(&waveFormat, sizeof(waveFormat), NULL)))
		{
			throw Log(LogLevel::LV_ERROR)
				.Msg("failed to get sound buffer format.")
				.Param(LogParam(LogParam::Tag::TEXT, path));
		}
		samplePerSec_ = waveFormat.nSamplesPerSec;
		bytesPerSample_ = waveFormat.wBitsPerSample / 8;
		channelCnt_ = waveFormat.nChannels;

		threadEndEvent_ = CreateEvent(nullptr, TRUE, FALSE, NULL);
		if (threadEndEvent_ == nullptr)
		{
			throw Log(LogLevel::LV_ERROR)
				.Msg("failed to create sound event.")
				.Param(LogParam(LogParam::Tag::TEXT, path));
		}

		loopEndEvent_ = CreateEvent(nullptr, TRUE, FALSE, NULL);
		if (loopEndEvent_ == nullptr)
		{
			throw Log(LogLevel::LV_ERROR)
				.Msg("failed to create sound event.")
				.Param(LogParam(LogParam::Tag::TEXT, path));
		}

		soundThreadParam_.soundBuffer = this;
		soundThreadParam_.soundEvents[THREAD_END_EVENT] = threadEndEvent_;
		soundThreadParam_.soundEvents[LOOP_END_EVENT] = loopEndEvent_;

		thread_ = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SoundMain, &soundThreadParam_, 0, NULL);
		if (thread_ == nullptr)
		{
			throw Log(LogLevel::LV_ERROR)
				.Msg("failed to create sound thread.")
				.Param(LogParam(LogParam::Tag::TEXT, path));
		}
	}
	catch (...)
	{
		if (thread_)
		{
			CloseThread(thread_, threadEndEvent_);
		}
		if (loopEndEvent_)
		{
			CloseHandle(loopEndEvent_);
		}
		if (threadEndEvent_)
		{
			CloseHandle(threadEndEvent_);
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
	pan = 1 - abs(pan); // 中央を1(倍)にする
	pan = pan == 0 ? DSBPAN_LEFT : (1000.0f * log10f(pan));
	if (rightPan) pan *= -1;
	pan = constrain(pan, 1.0f*DSBPAN_LEFT, 1.0f*DSBPAN_RIGHT);
	dSoundBuffer_->SetPan(pan);
}

void SoundBuffer::Seek(int sample)
{
	Log(LogLevel::LV_USER)
		.Msg("reading from memory")
		.Param(LogParam(LogParam::Tag::TEXT, "reading from memory"));

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
	CloseThread(thread_, threadEndEvent_);
	CloseHandle(loopEndEvent_);
	CloseHandle(threadEndEvent_);
	safe_release(dSoundBuffer_);

	//throw Log(LogLevel::LV_ERROR)
	//	.Msg("release sound.")
	//	.Param(LogParam(LogParam::Tag::TEXT, path_));
}

//stream

static IDirectSoundBuffer8* CreateSoundStreamBuffer(size_t bufSize, const std::unique_ptr<WaveSampleStream>& stream, const std::shared_ptr<SoundDevice>& soundDevice)
{
    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = stream->GetFormatTag();
    waveFormat.nChannels = stream->GetChannelCount();
    waveFormat.nSamplesPerSec = stream->GetSampleRate();
    waveFormat.nAvgBytesPerSec = stream->GetByteRate();
    waveFormat.nBlockAlign = stream->GetBlockAlign();
    waveFormat.wBitsPerSample = stream->GetBitsPerSample();
    waveFormat.cbSize = sizeof(WAVEFORMAT);

    DSBUFFERDESC dsBufferDesc;

    dsBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    dsBufferDesc.dwBufferBytes = bufSize;
    dsBufferDesc.dwReserved = 0;
    dsBufferDesc.dwFlags = DSBCAPS_LOCDEFER
        | DSBCAPS_GLOBALFOCUS
        | DSBCAPS_GETCURRENTPOSITION2
        | DSBCAPS_CTRLVOLUME
        | DSBCAPS_CTRLFREQUENCY
        | DSBCAPS_CTRLPAN; // 3D機能は使えなくなる
    dsBufferDesc.lpwfxFormat = &waveFormat;
    dsBufferDesc.guid3DAlgorithm = GUID_NULL;

    IDirectSoundBuffer* dsTempBuffer = nullptr;
    IDirectSoundBuffer8* dsBuffer = nullptr;

    if (soundDevice->GetDevice()->CreateSoundBuffer(&dsBufferDesc, &dsTempBuffer, nullptr) != DS_OK)
    {
        goto failed_to_create;
    }

    if (dsTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&(dsBuffer)) != DS_OK)
    {
        goto failed_to_create;
    }

    return dsBuffer;
failed_to_create:
    safe_release(dsTempBuffer);
    safe_release(dsBuffer);
    throw Log(LogLevel::LV_ERROR)
        .Msg("Failed to create sound buffer.")
        .Param(LogParam(LogParam::Tag::TEXT, stream->GetPath()));

}

static size_t CalcBufferSize(const std::unique_ptr<WaveSampleStream>& stream)
{
    constexpr float bufSec = 1.0f;
    size_t bufSamples = (size_t)(bufSec * stream->GetSampleRate());
    size_t bytesPerSample = stream->GetBytesPerSample();
    return std::min(bufSamples * bytesPerSample * stream->GetChannelCount(), stream->GetTotalBytes());
}

SoundStreamBuffer::SoundStreamBuffer(std::unique_ptr<WaveSampleStream>&& stream, const std::shared_ptr<SoundDevice>& soundDevice) :
    istream_(std::move(stream)),
    waveFormat_(istream_->GetWaveFormat()),
    bufSize_(CalcBufferSize(istream_)),
    dsBuffer_(CreateSoundStreamBuffer(bufSize_, istream_, soundDevice), com_deleter()),
    isThreadTerminated_(false),
    isLoopEnabled_(false),
    loopRange_(LoopRange(0, istream_->GetTotalBytes())),
    soundMain_(&SoundStreamBuffer::SoundMain, this)
{
}

SoundStreamBuffer::~SoundStreamBuffer()
{
    isThreadTerminated_ = true;
    soundMain_.join();
    Stop(); // NOTE: セカンダリバッファ解放前に止めないとプライマリバッファに音が残ることがある
}

#define CRITICAL_SECTION std::lock_guard<std::recursive_mutex> lock(criticalSection_)

void SoundStreamBuffer::Play()
{
    CRITICAL_SECTION;
    dsBuffer_->Play(0, 0, DSBPLAY_LOOPING);
}

void SoundStreamBuffer::Stop()
{
    CRITICAL_SECTION;
    dsBuffer_->Stop();
}

void SoundStreamBuffer::Rewind()
{
    // TODO: 短い間隔で行うと音がパラついて聞こえるので調査
    CRITICAL_SECTION;
    dsBuffer_->Stop(); // 再生停止
    dsBuffer_->SetCurrentPosition(0); // 再生カーソルを先頭に戻す
    istream_->SeekBytes(0); // 入力を先頭に戻す
    isSetStopCursor_ = false; // ストップカーソルを消す
    FillBufferSector(BufferSector::FirstHalf); // バッファの前半を埋める
    writeSector_ = BufferSector::LatterHalf; // バッファの後半を書き込み対象に
}

void SoundStreamBuffer::SetVolume(float volume)
{
    CRITICAL_SECTION;
    // 1/100dBに換算
    volume = constrain(volume, 0.0f, 1.0f);
    volume = volume == 0.0f ? DSBVOLUME_MIN : (100 * TodB(volume));
    volume = constrain(volume, (float)DSBVOLUME_MIN, (float)DSBVOLUME_MAX);
    dsBuffer_->SetVolume(volume);
}

void SoundStreamBuffer::SetPan(float pan)
{
    CRITICAL_SECTION;
    pan = constrain(pan, -1.0f, 1.0f);
    const bool rightPan = pan > 0;
    pan = 1 - abs(pan); // 0 .. 1
    pan = pan == 0.0f ? DSBPAN_LEFT : (100 * TodB(pan)); // -inf .. 0
    if (rightPan) pan *= -1; // -inf .. 0 .. inf
    pan = constrain(pan, 1.0f*DSBPAN_LEFT, 1.0f*DSBPAN_RIGHT); // -10000 .. 0 .. 10000
    dsBuffer_->SetPan(pan);
}

bool SoundStreamBuffer::IsPlaying() const
{
    CRITICAL_SECTION;
    DWORD status;
    dsBuffer_->GetStatus(&status);
    return (status & DSBSTATUS_PLAYING) != 0;
}

float SoundStreamBuffer::GetVolume() const
{
    CRITICAL_SECTION;
    LONG vol;
    dsBuffer_->GetVolume(&vol);
    return vol == DSBVOLUME_MIN ? 0.0f : FromdB(vol / 100.0f);
}

size_t SoundStreamBuffer::GetCurrentPlayCursor() const
{
    CRITICAL_SECTION;
    DWORD playCursor;
    dsBuffer_->GetCurrentPosition(&playCursor, nullptr);
    return playCursor;
}

void SoundStreamBuffer::SetLoopEnable(bool enable)
{
    isLoopEnabled_ = enable;
}

void SoundStreamBuffer::SetLoopTime(size_t beginSec, size_t endSec)
{
    SetLoopSampleCount(beginSec * waveFormat_.sampleRate, endSec * waveFormat_.sampleRate);
}

void SoundStreamBuffer::SetLoopSampleCount(size_t begin, size_t end)
{
    size_t conv = waveFormat_.bitsPerSample / 8 * waveFormat_.channelCount;
    SetLoopRange(begin * conv, end * conv);
}

void SoundStreamBuffer::SetLoopRange(size_t begin, size_t end)
{
    loopRange_ = LoopRange(begin, end);
}

void SoundStreamBuffer::SoundMain()
{
    Rewind();
    while (!isThreadTerminated_)
    {
        {
            CRITICAL_SECTION;
            if (istream_->IsClosed()) break;

            // fill buffer.
            {
                size_t playCursor = GetCurrentPlayCursor();
                if (writeSector_ == BufferSector::FirstHalf && playCursor >= GetHalfBufferSize() ||
                    writeSector_ == BufferSector::LatterHalf && playCursor < GetHalfBufferSize())
                {
                    FillBufferSector(writeSector_);
                    writeSector_ = writeSector_ == BufferSector::FirstHalf ? BufferSector::LatterHalf : BufferSector::FirstHalf;
                }
            }

            // stop if reach end of stream.
            if (isSetStopCursor_)
            {
                if (IsPlaying())
                {
                    size_t playCursor = GetCurrentPlayCursor();
                    if (stopCursor_ < GetHalfBufferSize())
                    {
                        if (stopCursor_ <= playCursor && playCursor < playCursorOnStop_)
                        {
                            Stop();
                        }
                    } else
                    {
                        if (playCursor < playCursorOnStop_ || stopCursor_ <= playCursor)
                        {
                            Stop();
                        }
                    }
                }
            }
        }
        // NOTE: スリープが長いとゼロ埋め部分のせい?でノイズが出ることがある
        Sleep(1);
    }
}

void SoundStreamBuffer::FillBufferSector(BufferSector sector)
{
    CRITICAL_SECTION;

    const size_t sectorOffset = sector == BufferSector::FirstHalf ? 0 : GetHalfBufferSize();
    const size_t sectorSize = sector == BufferSector::FirstHalf ? GetHalfBufferSize() : (bufSize_ - GetHalfBufferSize());

    char *writePtr, *unusedWritePtr;
    DWORD writableBytes, unusedWritableBytes;
    dsBuffer_->Lock(sectorOffset, sectorSize, (LPVOID*)&writePtr, &writableBytes, (LPVOID*)(&unusedWritePtr), &unusedWritableBytes, NULL);

    // never cyclic allocation
    assert(unusedWritePtr == NULL && unusedWritableBytes == 0);
    assert(sectorSize == writableBytes);

    const auto loopRange = loopRange_.load();

    size_t writtenSize = 0;
    while (writtenSize < sectorSize)
    {
        const size_t streamReadPos = istream_->Tell();

        if (isLoopEnabled_ &&
            streamReadPos < waveFormat_.totalBytes && // not stream end
            loopRange.begin < waveFormat_.totalBytes && loopRange.end <= waveFormat_.totalBytes && // valid loop range
            streamReadPos < loopRange.end && loopRange.end <= streamReadPos + (sectorSize - writtenSize) // over loop end
            )
        {
            // loop
            writtenSize += istream_->ReadBytes(loopRange.end - streamReadPos, writePtr + writtenSize);
            istream_->SeekBytes(loopRange.begin);
        } else
        {
            writtenSize += istream_->ReadBytes(sectorSize - writtenSize, writePtr + writtenSize);
        }

        if (istream_->IsEnd() || istream_->IsClosed())
        {
            // セクタの残りを無音で埋める
            FillMemory(writePtr + writtenSize, sectorSize - writtenSize, waveFormat_.bitsPerSample == 8 ? 0x80 : 0);

            // 最初にストリーム終端に到達したときだけ, 終了位置を記録
            if (!isSetStopCursor_)
            {
                isSetStopCursor_ = true;
                stopCursor_ = sectorOffset + writtenSize;
                playCursorOnStop_ = GetCurrentPlayCursor();
            }
            break;
        }
    }

    dsBuffer_->Unlock(writePtr, writableBytes, unusedWritePtr, unusedWritableBytes);
}
}

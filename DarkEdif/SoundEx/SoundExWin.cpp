#include "SoundEx.hpp"
#include "Common.hpp"

#include <Windows.h>
#include <timeapi.h>

#include <xaudio2.h>
#include <xapofx.h>
#include <xaudio2fx.h>

// DEFINES:
// -----
#define SafeRelease(i) if (i) { i->Release(); i = nullptr; }
#define SafeDestroyVoice(i) if (i) { ((IXAudio2Voice*)i)->DestroyVoice(); i = nullptr; }
#define SafeDeleteCallback(i) if (i) { delete (VoiceCallback*)i; i = nullptr; }

#define AS_VOICE(v) ((IXAudio2SourceVoice*)(v))
#define AS_PVOICE(v) ((IXAudio2SourceVoice**)(&v))
#define AS_CALLBACK(c) ((VoiceCallback*)(c))

// The maximum number of sound channels that can be queued at once
#define MAX_QUEUE_CHANNELS (MAXIMUM_WAIT_OBJECTS - 1)
// -----

// Internal data
namespace SoundEx
{
	IXAudio2* xAudio2 = nullptr;
	IXAudio2MasteringVoice* mVoice = nullptr;

	HANDLE audioThread = INVALID_HANDLE_VALUE;
	DWORD audioThreadID = NULL;

	HANDLE queueThread = INVALID_HANDLE_VALUE;
	DWORD queueThreadID = NULL;

	// Signaled to wake up a thread
	HANDLE wakeEvent = INVALID_HANDLE_VALUE;
	// Signaled when a thread should be closed
	HANDLE closeEvent = INVALID_HANDLE_VALUE;
	// Signaled when the audio engine must be restarted
	HANDLE restartEvent = INVALID_HANDLE_VALUE;
	// Signaled when the main thread is trying to get the position of a channel, which may cause issues if the audio thread is also trying to access/modify the sound position
	HANDLE getPositionEvent = INVALID_HANDLE_VALUE;
	HANDLE channelTimers[MAXIMUM_WAIT_OBJECTS];

	struct FileHandle
	{
		CInputBufFile pFile;
		HFILE hFile;
	};

	struct QueuedChannel
	{
		wchar name[SOUNDEX_MAX_WAVE_NAME_LENGTH + 1];
		int channel;
		bool loop;
		float volume;
		float frequency;
		float pan;
		bool active;
	} channelQueue[MAX_QUEUE_CHANNELS];

	CRITICAL_SECTION audioCS;
	CRITICAL_SECTION callbackCS;
	CRITICAL_SECTION queueCS;

	XAUDIO2_VOICE_DETAILS mDetails;

	// Engine callback to handle XAudio2 device loss
	class XAudioCallback : public IXAudio2EngineCallback
	{
	public:
		void WINAPI OnCriticalError(HRESULT Error) override
		{
			SetEvent(restartEvent);
			SetEvent(wakeEvent);
		}
		void WINAPI OnProcessingPassEnd() override {}
		void WINAPI OnProcessingPassStart() override {}
	} xAudioCallback;

	// Source voice callback to handle various things
	// (a bit ugly, sound processing is annoying)
	class VoiceCallback : public IXAudio2VoiceCallback
	{
	public:
		VoiceCallback(Channel* channel) { this->channel = channel; }

		Channel* channel;
		void WINAPI OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
		void WINAPI OnVoiceProcessingPassEnd() override {}
		void WINAPI OnStreamEnd() override
		{
			if (channel->wave->type != AudioType_Ogg)
				goto finished;

			if (!TryLockCallbacks())
			{
				if (WaitForSingleObject(getPositionEvent, 0) == WAIT_OBJECT_0)
					LockCallbacks();
				else
					goto finished;
			}
			channel->sampleLooped = channel->sampleStart = 0;
			UnlockCallbacks();
		finished:
			SetEvent(wakeEvent);
		}
		void WINAPI OnBufferStart(void* pBufferContext) override
		{
			SetEvent(wakeEvent);
		}
		void WINAPI OnBufferEnd(void* pBufferContext) override
		{
			SetEvent(wakeEvent);
		}
		void WINAPI OnLoopEnd(void* pBufferContext) override
		{
			if (!TryLockCallbacks())
			{
				if (WaitForSingleObject(getPositionEvent, 0) == WAIT_OBJECT_0)
					LockCallbacks();
				else
					goto finished;
			}
			channel->sampleLooped += channel->numSamples;
			UnlockCallbacks();
		finished:
			SetEvent(wakeEvent);
		}
		void WINAPI OnVoiceError(void* pBufferContext, HRESULT Error) override {}
	};
}

using namespace SoundEx;

/*

---------- UTILITY FUNCTIONS ----------

*/

void SetPan(IXAudio2SourceVoice* voice, uint32 numChannels, float pan)
{
	float left, right;
	if (pan < 0.0f)
	{
		left = 1.0f;
		right = std::max(1.0f + pan, 0.0f);
	}
	else
	{
		right = 1.0f;
		left = std::max(1.0f - pan, 0.0f);
	}

	float panMatrix[8];
	std::memset(panMatrix, 0x0, sizeof(panMatrix));
	switch (numChannels)
	{
	case 1:
	{
		panMatrix[0] = left;
		panMatrix[1] = right;

		voice->SetOutputMatrix(NULL, numChannels, mDetails.InputChannels, panMatrix);
		break;
	}
	case 2:
	{
		panMatrix[0] = left;
		panMatrix[3] = right;

		voice->SetOutputMatrix(NULL, numChannels, mDetails.InputChannels, panMatrix);
		break;
	}
	}
}

IXAudio2SourceVoice* CreateVoice(Channel& channel, const std::shared_ptr<WaveData> wave)
{
	WAVEFORMATEX wfm;
	wfm.cbSize = 0;
	wfm.wFormatTag = wave->floatPCM ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	wfm.nChannels = wave->numChannels;
	wfm.nSamplesPerSec = wave->sampleRate;
	wfm.wBitsPerSample = wave->bitRate;
	wfm.nBlockAlign = (wfm.wBitsPerSample / CHAR_BIT) * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	auto newVoice = (IXAudio2SourceVoice*)nullptr;
	auto newCallback = new VoiceCallback(&channel);
	if (FAILED(xAudio2->CreateSourceVoice(&newVoice, &wfm, 0u, 100.0f, newCallback)))
	{
		delete newCallback;
		return nullptr;
	}

	newVoice->SetVolume(channel.volume);
	newVoice->SetFrequencyRatio(channel.frequency);
	SetPan(newVoice, wave->numChannels, channel.pan);

	SafeDestroyVoice(channel.pAudio);
	SafeDeleteCallback(channel.pCallback);
	channel.pAudio = newVoice;
	channel.pCallback = newCallback;

	switch (channel.effect)
	{
	case AudioEffect_FXReverb:
		PlatformSetFXReverb(channel, channel.fxReverb, true);
		break;
	case AudioEffect_Princeton:
		PlatformSetFXPrinceton(channel, channel.princeton, true);
		break;
	}
	return newVoice;
}

DWORD WINAPI AudioMain(LPVOID)
{
	while (true)
	{
		WaitForSingleObject(wakeEvent, INFINITE); // sleep until we're called on :)
		if (WaitForSingleObject(closeEvent, 0) == WAIT_OBJECT_0)
			break;

		LockAudio();
		if (WaitForSingleObject(restartEvent, 0) == WAIT_OBJECT_0)
		{
			for (auto& channel : audioCtx.channels)
			{
				if (!channel.inUse)
					continue;

				channel.sampleStart = channel.wave->type == AudioType_WAV ? PlatformGetChannelPosition(channel) : 0;
				channel.sampleLooped = 0;
				PlatformStopChannel(channel);
			}
			if (PlatformInit(true))
			{
				for (auto& channel : audioCtx.channels)
				{
					if (!channel.inUse)
						continue;

					auto voice = CreateVoice(channel, channel.wave);
					channel.sampleStart = PlatformUpdateChannel(channel);
					channel.sampleLooped = 0;
					if (voice && channel.playing)
						voice->Start();
				}
			}
		}
		Update();

		UnlockAudio();
	}

	return 0;
}

DWORD WINAPI AudioQueueMain(LPVOID)
{
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(tc));

	timeBeginPeriod(tc.wPeriodMin);
	while (true)
	{
		auto ret = WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, channelTimers, FALSE, INFINITE);
		if (ret < WAIT_OBJECT_0 || ret >= WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS)
			continue;

		auto index = ret - WAIT_OBJECT_0;
		if (index == MAXIMUM_WAIT_OBJECTS - 1) // close event
			break;

		EnterCriticalSection(&queueCS);
		auto& pChannel = channelQueue[index];
		if (pChannel.active)
		{
			StartChannel(pChannel.name, pChannel.channel, pChannel.loop, pChannel.volume, pChannel.frequency, pChannel.pan, 0);
			pChannel.active = false;
		}
		LeaveCriticalSection(&queueCS);
	}
	timeEndPeriod(tc.wPeriodMin);

	return 0;
}

/*

---------- MAIN FUNCTIONS ----------

*/

bool SoundEx::PlatformInit(bool recovering)
{
	auto ret = true;

	// XAudio2 requires manual COM initialization:
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		ret = false;
		goto finished;
	}

	SafeDestroyVoice(mVoice);
	SafeRelease(xAudio2);
	if (FAILED(XAudio2Create(&xAudio2)))
	{
		ret = false;
		goto finished;
	}
	xAudio2->RegisterForCallbacks(&xAudioCallback);

	if (FAILED(xAudio2->CreateMasteringVoice(&mVoice, 2)))
	{
		SafeRelease(xAudio2);

		ret = false;
		goto finished;
	}

	mVoice->GetVoiceDetails(&mDetails);
	mVoice->SetVolume(audioCtx.masterVolume);

finished:
	if (!recovering)
	{
		InitializeCriticalSection(&audioCS);
		InitializeCriticalSection(&callbackCS);
		InitializeCriticalSection(&queueCS);

		wakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		closeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		restartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		getPositionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		std::memset(channelTimers, 0xFF, sizeof(channelTimers));
		std::memset(channelQueue, 0x0, sizeof(channelQueue));
		for (auto& channelTimer : channelTimers)
		{
			auto accessRights = TIMER_MODIFY_STATE | SYNCHRONIZE;
			auto timer = CreateWaitableTimerEx(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, accessRights);
			if (timer == INVALID_HANDLE_VALUE)
				timer = CreateWaitableTimer(NULL, FALSE, NULL);

			channelTimer = timer;
		}
		channelTimers[MAXIMUM_WAIT_OBJECTS - 1] = closeEvent;

		audioThread = CreateThread(NULL, NULL, AudioMain, NULL, NULL, &audioThreadID);
		queueThread = CreateThread(NULL, NULL, AudioQueueMain, NULL, NULL, &queueThreadID);
	}
	return ret;
}

void SoundEx::PlatformRelease()
{
	if (audioThread != INVALID_HANDLE_VALUE || queueThread != INVALID_HANDLE_VALUE)
	{
		SetEvent(closeEvent);
		SetEvent(wakeEvent);
		// Wait for the threads to close now that we signaled the events for that.
		// If for some reason it ends up taking too long we'll forcibly terminate the threads:
		if (WaitForSingleObject(audioThread, 500) == WAIT_TIMEOUT)
			TerminateThread(audioThread, 1);
		if (WaitForSingleObject(queueThread, 500) == WAIT_TIMEOUT)
			TerminateThread(queueThread, 1);

		audioThread = queueThread = INVALID_HANDLE_VALUE;
	}

	SafeDestroyVoice(mVoice);
	SafeRelease(xAudio2);
	CoUninitialize();

	CloseHandle(wakeEvent);
	wakeEvent = INVALID_HANDLE_VALUE;

	CloseHandle(closeEvent);
	closeEvent = INVALID_HANDLE_VALUE;

	CloseHandle(restartEvent);
	restartEvent = INVALID_HANDLE_VALUE;

	CloseHandle(getPositionEvent);
	getPositionEvent = INVALID_HANDLE_VALUE;

	DeleteCriticalSection(&audioCS);
	DeleteCriticalSection(&callbackCS);
	DeleteCriticalSection(&queueCS);
}


void SoundEx::LockAudio()
{
	EnterCriticalSection(&audioCS);
}

void SoundEx::UnlockAudio()
{
	LeaveCriticalSection(&audioCS);
}


void SoundEx::LockCallbacks()
{
	EnterCriticalSection(&callbackCS);
}

void SoundEx::UnlockCallbacks()
{
	LeaveCriticalSection(&callbackCS);
}

bool SoundEx::TryLockCallbacks()
{
	return TryEnterCriticalSection(&callbackCS);
}


void SoundEx::PlatformSetMasterVolume(float volume)
{
	if (!mVoice)
		return;

	mVoice->SetVolume(volume);
}


void SoundEx::PlatformCreateChannel(Channel& channel)
{
	channel.pAudio = nullptr;
	channel.pCallback = nullptr;
	if (!mVoice)
		return;

	auto wave = channel.wave;
	if (!CreateVoice(channel, wave))
		return;
}

void SoundEx::PlatformQueueChannel(const wchar* name, int channel, bool loop, float volume, float frequency, float pan, int ms)
{
	EnterCriticalSection(&queueCS);

	auto index = uint32(-1);
	for (uint32 i = 0; i < MAX_QUEUE_CHANNELS; ++i)
	{
		auto& pChannel = channelQueue[i];
		if (!pChannel.active)
		{
			index = i;
			break;
		}
	}
	if (index == uint32(-1))
		index = 0;

	auto& pChannel = channelQueue[index];
	wcsncpy(pChannel.name, name, SOUNDEX_MAX_WAVE_NAME_LENGTH);
	pChannel.name[SOUNDEX_MAX_WAVE_NAME_LENGTH] = L'\0';
	pChannel.channel = channel;
	pChannel.loop = loop;
	pChannel.volume = volume;
	pChannel.frequency = frequency;
	pChannel.pan = pan;
	pChannel.active = true;

	LARGE_INTEGER time;
	time.QuadPart = (LONGLONG)ms * -10000;
	auto ret = SetWaitableTimer(channelTimers[index], &time, NULL, NULL, NULL, FALSE);

	LeaveCriticalSection(&queueCS);
}

void SoundEx::PlatformStopChannel(Channel& channel)
{
	SafeDestroyVoice(channel.pAudio);
	SafeDeleteCallback(channel.pCallback);
}

void SoundEx::PlatformClearQueue()
{
	EnterCriticalSection(&queueCS);
	for (uint32 i = 0; i < MAX_QUEUE_CHANNELS; ++i)
	{
		CancelWaitableTimer(channelTimers[i]);
		channelQueue[i].active = false;
	}
	LeaveCriticalSection(&queueCS);
}

void SoundEx::PlatformPauseChannel(Channel& channel)
{
	if (!channel.pAudio)
		return;

	AS_VOICE(channel.pAudio)->Stop(channel.reverbTails ? XAUDIO2_PLAY_TAILS : NULL);
}

void SoundEx::PlatformResumeChannel(Channel& channel)
{
	if (!channel.pAudio)
		return;

	AS_VOICE(channel.pAudio)->Start();
}


void SoundEx::PlatformSetChannelVolume(Channel& channel, float volume)
{
	if (!channel.pAudio)
		return;

	AS_VOICE(channel.pAudio)->SetVolume(volume);
}

void SoundEx::PlatformSetChannelFrequency(Channel& channel, float frequency)
{
	if (!channel.pAudio)
		return;

	AS_VOICE(channel.pAudio)->SetFrequencyRatio(frequency);
}

void SoundEx::PlatformSetChannelPan(Channel& channel, float pan)
{
	if (!channel.pAudio)
		return;

	SetPan(AS_VOICE(channel.pAudio), channel.wave->numChannels, pan);
}

void SoundEx::PlatformSetChannelPosition(Channel& channel, int sample)
{
	if (!channel.pAudio)
		return;

	auto voice = AS_VOICE(channel.pAudio);
	const auto wave = channel.wave;
	switch (wave->type)
	{
	case AudioType_WAV:
	{
		if (sample < 0 || (uint32)sample >= wave->numSamples)
			goto failure;

		break;
	}
	case AudioType_Ogg:
	{
		if (!stb_vorbis_seek(channel.vorbis, (uint32)sample))
			goto failure;

		break;
	}
	}

	LockCallbacks();

	{
		voice->Stop();
		voice->FlushSourceBuffers();
		XAUDIO2_VOICE_STATE vState;
		// (waiting for all the buffers to actually be flushed to MAKE SURE the callback functions don't get executed here)
		while (true)
		{
			voice->GetState(&vState, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			if (!vState.BuffersQueued)
				break;
		}

		channel.nextVorbisBuffer = 0;
		channel.sampleStart = sample;
		voice->GetState(&vState);
		channel.sampleLooped = vState.SamplesPlayed;

		auto sampleStart = PlatformUpdateChannel(channel);
		if (sampleStart != 0)
			channel.sampleStart = sampleStart;

		if (channel.playing)
			voice->Start();
	}

	UnlockCallbacks();

failure:
	audioCtx.lastError = AudioError_SeekFailure;
}


void SoundEx::PlatformSetFXReverb(Channel& channel, const FXReverbParams& params, bool forceCreateFX)
{
	if (!channel.pAudio)
		return;

	auto voice = AS_VOICE(channel.pAudio);
	const auto wave = channel.wave;
	if (forceCreateFX || channel.effect != AudioEffect_FXReverb)
	{
		IUnknown* effect;
		if (SUCCEEDED(CreateFX(__uuidof(FXReverb), &effect)))
		{
			XAUDIO2_EFFECT_DESCRIPTOR effectDesc;
			effectDesc.InitialState = TRUE;
			effectDesc.OutputChannels = wave->numChannels;
			effectDesc.pEffect = effect;

			XAUDIO2_EFFECT_CHAIN effectChain;
			effectChain.EffectCount = 1;
			effectChain.pEffectDescriptors = &effectDesc;

			voice->SetEffectChain(&effectChain);

			effect->Release();
		}
	}

	voice->SetEffectParameters(0, &params, sizeof(params));
}

void SoundEx::PlatformSetFXPrinceton(Channel& channel, const PrincetonReverbParams& params, bool forceCreateFX)
{
	if (!channel.pAudio)
		return;

	auto voice = AS_VOICE(channel.pAudio);
	const auto wave = channel.wave;
	if (forceCreateFX || channel.effect != AudioEffect_Princeton)
	{
		IUnknown* effect;
		if (SUCCEEDED(XAudio2CreateReverb(&effect)))
		{
			XAUDIO2_EFFECT_DESCRIPTOR effectDesc;
			effectDesc.InitialState = TRUE;
			effectDesc.OutputChannels = wave->numChannels;
			effectDesc.pEffect = effect;

			XAUDIO2_EFFECT_CHAIN effectChain;
			effectChain.EffectCount = 1;
			effectChain.pEffectDescriptors = &effectDesc;

			voice->SetEffectChain(&effectChain);

			effect->Release();
		}
	}

	voice->SetEffectParameters(0, &params, sizeof(params));
}

void SoundEx::PlatformClearFX(Channel& channel)
{
	if (!channel.pAudio)
		return;

	AS_VOICE(channel.pAudio)->SetEffectChain(NULL);
}


uint64 SoundEx::PlatformUpdateChannel(Channel& channel)
{
	if (!channel.pAudio)
		return 0;

	XAUDIO2_BUFFER buffer = {};

	auto voice = AS_VOICE(channel.pAudio);
	auto wave = channel.wave;
	switch (wave->type)
	{
	case AudioType_WAV:
	{
		buffer.AudioBytes = wave->sampleSize;
		buffer.pAudioData = wave->samples.get();
		buffer.PlayBegin = (uint32)channel.sampleStart;
		buffer.PlayLength = buffer.PlayBegin < wave->numSamples ? wave->numSamples - buffer.PlayBegin : 0;
		buffer.LoopCount = channel.loop ? XAUDIO2_LOOP_INFINITE : 0;
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		voice->SubmitSourceBuffer(&buffer);
		return channel.sampleStart;
	}
	case AudioType_Ogg:
	{
		auto sampleOffset = stb_vorbis_get_sample_offset(channel.vorbis);
		auto currentSample = sampleOffset != -1 ? (uint64)sampleOffset : 0;
		auto destBuffer = channel.vorbisSamples.get() + channel.nextVorbisBuffer * channel.vorbisSamplesCapacity;

		channel.vorbisSamplesSize = stb_vorbis_get_samples_short_interleaved(channel.vorbis, wave->numChannels, destBuffer, channel.vorbisSamplesCapacity);
		if (!channel.vorbisSamplesSize)
		{
			voice->Discontinuity();
			if (!channel.loop)
			{
				channel.streamEnding = true;
				return currentSample;
			}
		}

		buffer.AudioBytes = channel.vorbisSamplesSize * wave->numChannels * sizeof(int16);
		buffer.pAudioData = (BYTE*)destBuffer;
		if (channel.vorbisSamplesSize < SOUNDEX_STREAM_BUFFER_SAMPLES)
		{
			channel.streamEnding = true;
			buffer.Flags = XAUDIO2_END_OF_STREAM;
			if (channel.loop)
				stb_vorbis_seek_start(channel.vorbis);
		}
		else
		{
			channel.streamEnding = false;
		}
		voice->SubmitSourceBuffer(&buffer);

		channel.nextVorbisBuffer++;
		channel.nextVorbisBuffer %= SOUNDEX_NUM_STREAM_BUFFERS;
		return currentSample;
	}
	}

	return 0;
}


int SoundEx::PlatformGetChannelPosition(const Channel& channel, bool sampleStart)
{
	if (!channel.pAudio)
		return 0;

	SetEvent(getPositionEvent);
	LockCallbacks();

	XAUDIO2_VOICE_STATE state;
	AS_VOICE(channel.pAudio)->GetState(&state);
	auto ret = sampleStart ?
		(int)((state.SamplesPlayed + channel.sampleStart) - channel.sampleLooped) :
		(int)(state.SamplesPlayed - channel.sampleLooped);
	if (ret < 0)
		ret = 0;

	ResetEvent(getPositionEvent);
	UnlockCallbacks();
	return ret;
}


uint32 SoundEx::GetBufferQueue(const Channel& channel)
{
	if (!channel.pAudio)
		return 0;

	XAUDIO2_VOICE_STATE state;
	AS_VOICE(channel.pAudio)->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
	return state.BuffersQueued;
}

/*

--------- FILE IO ----------

*/

bool SoundEx::FileOpen(const wchar* fileName, File& outFile)
{
	auto ret = true;
	auto handle = new FileHandle;
	outFile = handle;

	DWORD dwSize, dwOffset;
	handle->hFile = (HFILE)Edif::SDK->mV->mvOpenHFileW(fileName, &dwSize, 0);
	if (handle->hFile == HFILE_ERROR)
	{
		ret = false;
		goto finished;
	}

	dwOffset = File_GetPosition(handle->hFile);
	if (handle->pFile.Create(handle->hFile, dwOffset, dwSize) != 0)
	{
		ret = false;
		goto finished;
	}

finished:
	if (!ret)
	{
		if (handle->hFile != HFILE_ERROR)
			Edif::SDK->mV->mvCloseHFile((HANDLE)handle->hFile);

		delete handle;
		outFile = SOUNDEX_FILE_ERROR;
	}
	return ret;
}

void SoundEx::FileClose(File file)
{
	auto handle = (FileHandle*)file;
	if (handle != SOUNDEX_FILE_ERROR)
	{
		if (handle->hFile != HFILE_ERROR)
			Edif::SDK->mV->mvCloseHFile((HANDLE)handle->hFile);
		delete handle;
	}
}


uint32 SoundEx::FileRead(File file, void* buffer, uint32 bufferSize)
{
	auto handle = (FileHandle*)file;
	if (!handle)
		return 0;

	DWORD numRead = 0;
	handle->pFile.Read((LPBYTE)buffer, (DWORD)bufferSize, &numRead);
	return (uint32)numRead;
}

uint32 SoundEx::FileSeek(File file, uint32 offset, FileDirection direction)
{
	auto handle = (FileHandle*)file;
	if (!handle)
		return 0;

	int method;
	switch (direction)
	{
	case FileDirection_Begin:
	default:
		method = SEEK_SET;
		break;
	case FileDirection_Current:
		method = SEEK_CUR;
		break;
	case FileDirection_End:
		method = SEEK_END;
		break;
	}
	return (uint32)handle->pFile.Seek((long)offset, method);
}

uint64 SoundEx::FileSize(File file)
{
	auto handle = (FileHandle*)file;
	if (!handle)
		return 0;

	return (uint64)handle->pFile.GetLengthEx();
}

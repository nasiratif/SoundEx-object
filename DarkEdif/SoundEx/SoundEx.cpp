#include "SoundEx.hpp"

namespace SoundEx
{
	AudioContext audioCtx;
}

using namespace SoundEx;

/*

---------- MAIN FUNCTIONS ----------

*/

bool SoundEx::Init()
{
	audioCtx.masterVolume = 1.0f;
	audioCtx.muted = false;

	audioCtx.lastError = AudioError_None;
	audioCtx.lastChannelPlayed = -1;

	for (auto& channel : audioCtx.channels)
		channel.inUse = false;

	return PlatformInit();
}

void SoundEx::Release()
{
	LockAudio();
	for (auto& channel : audioCtx.channels)
		if (channel.inUse)
			ReleaseChannel(channel);

	if (!audioCtx.waves.empty())
		audioCtx.waves.clear();

	UnlockAudio();
	PlatformRelease();
}


void SoundEx::Update()
{
	for (auto& channel : audioCtx.channels)
	{
		if (!channel.inUse)
			continue;

		switch (channel.wave->type)
		{
		case AudioType_WAV:
		{
			if (!GetBufferQueue(channel))
				ReleaseChannel(channel);

			break;
		}
		case AudioType_Ogg:
		{
			auto bufferQueue = GetBufferQueue(channel);
			if (!bufferQueue && !channel.loop && channel.streamEnding)
			{
				ReleaseChannel(channel);
			}
			else if (bufferQueue < SOUNDEX_NUM_STREAM_BUFFERS)
			{
				PlatformUpdateChannel(channel);
			}
			break;
		}
		}
	}
}


void SoundEx::SetMasterVolume(float volume)
{
	LockAudio();

	audioCtx.masterVolume = volume;
	if (audioCtx.muted)
		goto finished;

	PlatformSetMasterVolume(volume);

finished:
	UnlockAudio();
}


void SoundEx::LoadWaveData(const wchar* fileName, const wchar* name)
{
	if (!(*name) || std::wcslen(name) > SOUNDEX_MAX_WAVE_NAME_LENGTH)
	{
		LockAudio();
		audioCtx.lastError = AudioError_InvalidSoundName;
		UnlockAudio();
		return;
	}

	LockAudio();

	audioCtx.lastError = AudioError_None;
	auto error = audioCtx.lastError;

	WaveData wave = {};
	File file = SOUNDEX_FILE_ERROR;

	for (const auto& existingWave : audioCtx.waves)
	{
		if (std::wcscmp(name, existingWave->name) == 0)
		{
			UnlockAudio();
			goto finished;
		}
	}
	UnlockAudio();

	if (FileOpen(fileName, file))
	{
		uint32 signature = 0;
		FileRead(file, &signature, sizeof(signature));

		switch (signature)
		{
		case AudioFourCC_RIFF:
		{
			wave.type = AudioType_WAV;

			FileSeek(file, sizeof(uint32), FileDirection_Current); // ahead of the RIFF chunk's file size variable

			uint32 formType = 0;
			FileRead(file, &formType, sizeof(formType));
			if (formType != AudioFourCC_WAVE)
			{
				error = AudioError_WAVLoadFailed;
				goto finished;
			}

			// Read all further chunks:
			while (true)
			{
				uint32 chunkType = 0;
				if (!FileRead(file, &chunkType, sizeof(chunkType)))
				{
					if (!wave.samples || !wave.sampleRate)
						error = AudioError_WAVLoadFailed;

					goto finished;
				}

				uint32 chunkSize = 0;
				if (!FileRead(file, &chunkSize, sizeof(chunkSize)))
				{
					if (!wave.samples || !wave.sampleRate)
						error = AudioError_WAVLoadFailed;

					goto finished;
				}

				switch (chunkType)
				{
				case AudioFourCC_FMT:
				{
					WaveFormat wfm = {};

					if (wave.sampleRate || chunkSize > sizeof(wfm))
					{
						error = AudioError_WAVLoadFailed;
						goto finished;
					}

					FileRead(file, &wfm, chunkSize);

					if ((wfm.format != SOUNDEX_WAVE_FORMAT_PCM && wfm.format != SOUNDEX_WAVE_FORMAT_IEEE_FLOAT) || wfm.bitRate % CHAR_BIT || wfm.numChannels > 0xFF)
					{
						error = AudioError_WAVLoadFailed;
						goto finished;
					}
					wave.sampleRate = wfm.sampleRate;
					wave.bitRate = wfm.bitRate;
					wave.bitRateBytes = wave.bitRate / CHAR_BIT;
					wave.numChannels = (uint8)wfm.numChannels;
					wave.floatPCM = wfm.format == SOUNDEX_WAVE_FORMAT_IEEE_FLOAT;
					break;
				}
				case AudioFourCC_Data:
				{
					if (wave.samples || !wave.bitRateBytes || !wave.numChannels)
					{
						error = AudioError_WAVLoadFailed;
						goto finished;
					}

					wave.sampleSize = chunkSize;
					wave.numSamples = (wave.sampleSize / wave.bitRateBytes) / wave.numChannels;
					try
					{
						wave.samples.reset(new byte[wave.sampleSize]);
					}
					catch (const std::bad_alloc&)
					{
						error = AudioError_OutOfMemory;
						goto finished;
					}

					if (FileRead(file, wave.samples.get(), wave.sampleSize) != wave.sampleSize)
					{
						error = AudioError_WAVLoadFailed;
						goto finished;
					}
					break;
				}
				default:
				{
					FileSeek(file, chunkSize, FileDirection_Current);
				}
				}
			}

			break;
		}
		case AudioFourCC_Ogg:
		{
			wave.type = AudioType_Ogg;

			FileSeek(file, 0, FileDirection_Begin);

			auto fileSize = FileSize(file);
			wave.sampleSize = (uint32)fileSize;
			try
			{
				wave.samples.reset(new byte[wave.sampleSize]);
			}
			catch (const std::bad_alloc&)
			{
				error = AudioError_OutOfMemory;
				goto finished;
			}

			FileRead(file, wave.samples.get(), wave.sampleSize);

			int err;
			auto vorbis = stb_vorbis_open_memory(wave.samples.get(), wave.sampleSize, &err, nullptr);
			if (!vorbis)
			{
				error = AudioError_OggLoadFailed;
				goto finished;
			}

			auto info = stb_vorbis_get_info(vorbis);
			wave.sampleRate = info.sample_rate;
			wave.bitRate = 16; // Ogg is purely 16-bit
			wave.bitRateBytes = wave.bitRate / CHAR_BIT;
			wave.numChannels = info.channels;
			wave.numSamples = stb_vorbis_stream_length_in_samples(vorbis);
			wave.floatPCM = false;

			stb_vorbis_close(vorbis);
			break;
		}
		default:
		{
			error = AudioError_InvalidSignature;
			goto finished;
		}
		}
	}
	else
	{
		error = AudioError_FileNotOpen;
		goto finished;
	}

finished:
	LockAudio();
	if (error == AudioError_None && wave.samples)
	{
		std::wcsncpy(wave.name, name, SOUNDEX_MAX_WAVE_NAME_LENGTH);
		wave.name[SOUNDEX_MAX_WAVE_NAME_LENGTH - 1] = L'\0';

		audioCtx.waves.emplace_back(std::make_shared<WaveData>(wave));
	}
	else
	{
		audioCtx.lastError = error;
	}
	UnlockAudio();
	FileClose(file);
}

void SoundEx::UnloadWave(const wchar* name)
{
	LockAudio();

	audioCtx.lastError = AudioError_UnknownSoundName;
	for (auto it = audioCtx.waves.begin(); it != audioCtx.waves.end(); ++it)
	{
		const auto& wave = *it;
		if (std::wcscmp(name, wave->name) == 0)
		{
			for (auto& channel : audioCtx.channels)
			{
				if (!channel.inUse || channel.wave != wave)
					continue;

				ReleaseChannel(channel);
			}

			audioCtx.waves.erase(it);
			goto success;
		}
	}

	UnlockAudio();
	return;

success:
	UnlockAudio();
	audioCtx.lastError = AudioError_None;
}


void SoundEx::UnloadAllWaves()
{
	LockAudio();
	audioCtx.lastError = AudioError_None;

	for (auto& channel : audioCtx.channels)
	{
		if (!channel.inUse)
			continue;

		ReleaseChannel(channel);
	}

	if (!audioCtx.waves.empty())
		audioCtx.waves.clear();

	UnlockAudio();
}


void SoundEx::StartChannel(const wchar* name, int channel, bool loop, float volume, float frequency, float pan, int ms)
{
	if (ms != 0)
	{
		if (ms < 0)
		{
			LockAudio();
			audioCtx.lastError = AudioError_InvalidMS;
			UnlockAudio();
			return;
		}

		PlatformQueueChannel(name, channel, loop, volume, frequency, pan, ms);
		return;
	}
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel == SOUNDEX_CHANNEL_AUTO)
	{
		for (int i = 0; i < SOUNDEX_NUM_CHANNELS; ++i)
		{
			if (!audioCtx.channels[i].inUse)
			{
				channel = i;
				break;
			}
		}
		if (channel == SOUNDEX_CHANNEL_AUTO)
			channel = SOUNDEX_NUM_CHANNELS - 1;
	}
	else if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}

	// This is inside of it's own branch to prevent goto errors:
	{
		std::shared_ptr<WaveData> wave;
		for (const auto& currentWave : audioCtx.waves)
		{
			if (std::wcscmp(name, currentWave->name) == 0)
			{
				wave = currentWave;
				break;
			}
		}
		if (!wave)
		{
			audioCtx.lastError = AudioError_UnknownSoundName;
			goto finished;
		}

		stb_vorbis* vorbis;
		if (wave->type == AudioType_Ogg)
		{
			int err;
			vorbis = stb_vorbis_open_memory(wave->samples.get(), (int)wave->sampleSize, &err, nullptr);
			if (!vorbis)
			{
				audioCtx.lastError = AudioError_OggLoadFailed;
				goto finished;
			}
		}
		else
		{
			vorbis = nullptr;
		}

		auto& pChannel = audioCtx.channels[channel];
		if (pChannel.inUse)
			ReleaseChannel(pChannel);

		pChannel.inUse = true;
		pChannel.playing = true;
		pChannel.loop = loop;
		pChannel.volume = volume;
		pChannel.frequency = frequency;
		pChannel.pan = pan;
		pChannel.effect = AudioEffect_None;
		std::memset(&pChannel.fxReverb, 0x0, sizeof(pChannel.fxReverb));
		std::memset(&pChannel.princeton, 0x0, sizeof(pChannel.princeton));
		pChannel.reverbTails = true;
		pChannel.sampleStart = pChannel.sampleLooped = 0;
		pChannel.numSamples = wave->numSamples;
		pChannel.wave = wave;
		pChannel.vorbis = vorbis;
		if (pChannel.vorbis)
		{
			pChannel.vorbisSamplesCapacity = SOUNDEX_STREAM_BUFFER_SAMPLES * wave->numChannels;
			pChannel.vorbisSamples.reset(new int16[pChannel.vorbisSamplesCapacity * SOUNDEX_NUM_STREAM_BUFFERS]);
		}
		pChannel.vorbisSamplesSize = 0;
		pChannel.nextVorbisBuffer = 0;
		pChannel.streamEnding = false;
		PlatformCreateChannel(pChannel);
		PlatformUpdateChannel(pChannel);
		PlatformResumeChannel(pChannel);

		audioCtx.lastChannelPlayed = channel;
	}
finished:
	UnlockAudio();
}

void SoundEx::StopChannel(int channel)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ReleaseChannel(pChannel);
	}

finished:
	UnlockAudio();
}

void SoundEx::ClearQueue()
{
	LockAudio();
	audioCtx.lastError = AudioError_None;
	UnlockAudio();

	PlatformClearQueue();
}

void SoundEx::ReleaseChannel(Channel& channel)
{
	PlatformStopChannel(channel);
	if (channel.vorbis)
		stb_vorbis_close(channel.vorbis);

	channel.vorbisSamples.reset();
	channel.wave.reset();
	channel.inUse = false;
}


void SoundEx::PauseChannel(int channel)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformPauseChannel(pChannel);
		pChannel.playing = false;
	}

finished:
	UnlockAudio();
}

void SoundEx::ResumeChannel(int channel)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformResumeChannel(pChannel);
		pChannel.playing = true;
	}

finished:
	UnlockAudio();
}


void SoundEx::StopAllChannels()
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	for (auto& pChannel : audioCtx.channels)
	{
		if (!pChannel.inUse)
			continue;

		ReleaseChannel(pChannel);
	}

	UnlockAudio();
}

void SoundEx::PauseAllChannels()
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	for (auto& pChannel : audioCtx.channels)
	{
		if (!pChannel.inUse)
			continue;

		PlatformPauseChannel(pChannel);
		pChannel.playing = false;
	}

	UnlockAudio();
}

void SoundEx::ResumeAllChannels()
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	for (auto& pChannel : audioCtx.channels)
	{
		if (!pChannel.inUse)
			continue;

		PlatformResumeChannel(pChannel);
		pChannel.playing = true;
	}

	UnlockAudio();
}


void SoundEx::SetChannelVolume(int channel, float volume)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformSetChannelVolume(pChannel, volume);
		pChannel.volume = volume;
	}

finished:
	UnlockAudio();
}

void SoundEx::SetChannelFrequency(int channel, float frequency)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformSetChannelFrequency(pChannel, frequency);
		pChannel.frequency = frequency;
	}

finished:
	UnlockAudio();
}

void SoundEx::SetChannelPan(int channel, float pan)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		pan = std::clamp(pan, -1.0f, 1.0f);
		PlatformSetChannelPan(pChannel, pan);
		pChannel.pan = pan;
	}

finished:
	UnlockAudio();
}

void SoundEx::SetChannelPosition(int channel, int sample)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformSetChannelPosition(pChannel, sample);
	}

finished:
	UnlockAudio();
}


void SoundEx::SetChannelFXReverb(int channel, const FXReverbParams& params)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformSetFXReverb(pChannel, params);
		pChannel.effect = AudioEffect_FXReverb;
		pChannel.fxReverb = params;
	}

finished:
	UnlockAudio();
}

void SoundEx::SetChannelFXPrinceton(int channel, const PrincetonReverbParams& params)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformSetFXPrinceton(pChannel, params);
		pChannel.effect = AudioEffect_Princeton;
		pChannel.princeton = params;
	}

finished:
	UnlockAudio();
}

void SoundEx::ClearFX(int channel)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		PlatformClearFX(pChannel);
		pChannel.effect = AudioEffect_None;
	}

finished:
	UnlockAudio();
}


void SoundEx::SetReverbTails(int channel, bool reverbTails)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		pChannel.reverbTails = reverbTails;
	}

finished:
	UnlockAudio();
}


int SoundEx::FindChannel(const wchar* name)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	int ret = -1;
	for (int i = 0; i < SOUNDEX_NUM_CHANNELS; ++i)
	{
		const auto& channel = audioCtx.channels[i];
		if (channel.inUse && std::wcscmp(name, channel.wave->name) == 0)
		{
			ret = i;
			goto finished;
		}
	}

	audioCtx.lastError = AudioError_ChannelNotFound;

finished:
	UnlockAudio();
	return ret;
}


std::shared_ptr<WaveData> SoundEx::GetWave(const wchar* name)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	std::shared_ptr<WaveData> ret = nullptr;
	for (const auto& wave : audioCtx.waves)
	{
		if (std::wcscmp(wave->name, name) == 0)
		{
			ret = wave;
			goto finished;
		}
	}

	audioCtx.lastError = AudioError_UnknownSoundName;

finished:
	UnlockAudio();
	return ret;
}


float SoundEx::GetWaveAmplitude(const wchar* name, int sample, int sampleChannel)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (sampleChannel < 0)
	{
		audioCtx.lastError = AudioError_InvalidSampleChannel;
		goto failure;
	}
	else if (sample < 0)
	{
		audioCtx.lastError = AudioError_InvalidSample;
		goto failure;
	}
	else
	{
		std::shared_ptr<WaveData> wave;
		for (const auto& currentWave : audioCtx.waves)
		{
			if (std::wcscmp(currentWave->name, name) == 0)
			{
				wave = currentWave;
				break;
			}
		}
		if (!wave)
		{
			audioCtx.lastError = AudioError_UnknownSoundName;
			goto failure;
		}
		else if (wave->type != AudioType_WAV)
		{
			audioCtx.lastError = AudioError_NotWAV;
			goto failure;
		}
		else if ((uint32)sample >= wave->numSamples)
		{
			audioCtx.lastError = AudioError_InvalidSample;
			goto failure;
		}

		if (sampleChannel >= wave->numChannels)
		{
			audioCtx.lastError = AudioError_InvalidSampleChannel;
			goto failure;
		}

		UnlockAudio();

		auto byteOffset = (uint32)sample * wave->numChannels * wave->bitRateBytes;
		return GetSampleAmplitude(wave, wave->samples.get() + byteOffset, wave->samples.get() + wave->sampleSize, sampleChannel);
	}

failure:
	UnlockAudio();
	return 0.0f;
}


float SoundEx::GetMasterVolume()
{
	LockAudio();
	audioCtx.lastError = AudioError_None;
	auto ret = audioCtx.masterVolume;
	UnlockAudio();
	return ret;
}


bool SoundEx::GetChannelPlaying(int channel)
{
	LockAudio();

	bool ret = false;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = pChannel.playing;
	}

finished:
	UnlockAudio();
	return ret;
}

bool SoundEx::GetChannelLoop(int channel)
{
	LockAudio();

	bool ret = false;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = pChannel.loop;
	}

finished:
	UnlockAudio();
	return ret;
}


float SoundEx::GetChannelVolume(int channel)
{
	LockAudio();

	float ret = 0.0f;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = pChannel.volume;
	}

finished:
	UnlockAudio();
	return ret;
}

float SoundEx::GetChannelFrequency(int channel)
{
	LockAudio();

	float ret = 0.0f;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = pChannel.frequency;
	}

finished:
	UnlockAudio();
	return ret;
}

float SoundEx::GetChannelPan(int channel)
{
	LockAudio();

	float ret = 0.0f;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}
		ret = pChannel.pan;
	}

finished:
	UnlockAudio();
	return ret;
}

int SoundEx::GetChannelPosition(int channel)
{
	LockAudio();

	int ret = 0;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = PlatformGetChannelPosition(pChannel);
	}

finished:
	UnlockAudio();
	return ret;
}

float SoundEx::GetChannelAmplitude(int channel, int sampleChannel)
{
	LockAudio();

	float ret = 0.0f;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		if (sampleChannel < 0 || sampleChannel >= pChannel.wave->numChannels)
		{
			audioCtx.lastError = AudioError_InvalidSampleChannel;
			goto finished;
		}

		switch (pChannel.wave->type)
		{
		case AudioType_WAV:
		{
			auto sample = PlatformGetChannelPosition(pChannel);
			if ((uint32)sample >= pChannel.wave->numSamples)
			{
				audioCtx.lastError = AudioError_InvalidSample;
				goto finished;
			}
			auto byteOffset = (uint32)sample * pChannel.wave->numChannels * pChannel.wave->bitRateBytes;
			ret = GetSampleAmplitude(pChannel.wave, pChannel.wave->samples.get() + byteOffset, pChannel.wave->samples.get() + pChannel.wave->sampleSize, sampleChannel);
			break;
		}
		case AudioType_Ogg:
		{
			auto sample = PlatformGetChannelPosition(pChannel, false) % SOUNDEX_STREAM_BUFFER_SAMPLES;
			auto bufferQueue = GetBufferQueue(pChannel);
			if (!bufferQueue || bufferQueue > SOUNDEX_NUM_STREAM_BUFFERS)
			{
				audioCtx.lastError = AudioError_Error;
				goto finished;
			}

			auto bufferIndex = ((SOUNDEX_NUM_STREAM_BUFFERS - bufferQueue) + pChannel.nextVorbisBuffer) % SOUNDEX_NUM_STREAM_BUFFERS;
			auto samples = pChannel.vorbisSamples.get() + bufferIndex * pChannel.vorbisSamplesCapacity;
			auto samplesEnd = samples + pChannel.vorbisSamplesCapacity;

			auto byteOffset = (uint32)sample * pChannel.wave->numChannels * pChannel.wave->bitRateBytes;
			ret = GetSampleAmplitude(pChannel.wave, ((byte*)samples) + byteOffset, (byte*)samplesEnd, sampleChannel);
			break;
		}
		}
	}

finished:
	UnlockAudio();
	return ret;
}

std::shared_ptr<WaveData> SoundEx::GetChannelWave(int channel)
{
	LockAudio();

	std::shared_ptr<WaveData> ret;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = pChannel.wave;
	}

finished:
	UnlockAudio();
	return ret;
}


AudioEffect SoundEx::GetChannelFX(int channel)
{
	LockAudio();

	auto ret = AudioEffect_None;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = pChannel.effect;
	}

finished:
	UnlockAudio();
	return ret;
}


bool SoundEx::GetChannelReverbTails(int channel)
{
	LockAudio();

	bool ret = false;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = pChannel.reverbTails;
	}

finished:
	UnlockAudio();
	return ret;
}


const FXReverbParams* SoundEx::GetChannelFXReverbParams(int channel)
{
	LockAudio();

	const FXReverbParams* ret = nullptr;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = &pChannel.fxReverb;
	}

finished:
	UnlockAudio();
	return ret;
}

const PrincetonReverbParams* SoundEx::GetChannelFXPrincetonParams(int channel)
{
	LockAudio();

	const PrincetonReverbParams* ret = nullptr;
	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= SOUNDEX_NUM_CHANNELS)
	{
		audioCtx.lastError = AudioError_InvalidChannel;
		goto finished;
	}
	else
	{
		const auto& pChannel = audioCtx.channels[channel];
		if (!pChannel.inUse)
		{
			audioCtx.lastError = AudioError_ChannelNotUsed;
			goto finished;
		}

		ret = &pChannel.princeton;
	}

finished:
	UnlockAudio();
	return ret;
}


int SoundEx::GetLastChannelPlayed()
{
	LockAudio();
	auto ret = audioCtx.lastChannelPlayed;
	UnlockAudio();
	return ret;
}


int SoundEx::GetNumWaves()
{
	LockAudio();
	auto ret = (int)audioCtx.waves.size();
	UnlockAudio();
	return ret;
}

int SoundEx::GetNumActiveChannels()
{
	LockAudio();

	int ret = 0;
	for (auto& pChannel : audioCtx.channels)
		if (pChannel.inUse)
			ret++;

	UnlockAudio();
	return ret;
}


float SoundEx::GetSampleAmplitude(const std::shared_ptr<WaveData>& wave, const byte* samples, const byte* samplesEnd, int sampleChannel)
{
	float32 sample;
	switch (wave->bitRate)
	{
	case 8:
	{
		auto pSample = ((uint8*)samples) + sampleChannel;
		auto sampleValue = (((int)*pSample) - 128);
		sample = sampleValue >= 0 ?
			sampleValue / 127.0f :
			sampleValue / 128.0f;
		break;
	}
	case 16:
	{
		auto pSample = ((int16*)samples) + sampleChannel;
		auto sampleValue = *pSample;
		sample = sampleValue >= 0 ?
			sampleValue / 32767.0f :
			sampleValue / 32768.0f;
		break;
	}
	case 24:
	{
		auto pSample = samples + sampleChannel * 3;

		uint32 rawSample = 0;
		std::memcpy(&rawSample, pSample, 3);
		if (rawSample & 0x800000) // sign extending
			rawSample |= 0xFF000000;

		auto sampleValue = (int32)rawSample;
		sample = sampleValue >= 0 ?
			sampleValue / 8388607.0f :
			sampleValue / 8388608.0f;
		break;
	}
	case 32:
	{
		if (wave->floatPCM)
		{
			auto pSample = ((float32*)samples) + sampleChannel;
			sample = *pSample;
		}
		else
		{
			auto pSample = ((int32*)samples) + sampleChannel;
			auto sampleValue = *pSample;
			sample = sampleValue >= 0 ?
				sampleValue / 2147483647.0f :
				sampleValue / 2147483648.0f;
		}
		break;
	}
	case 64:
	{
		if (wave->floatPCM)
		{
			auto pSample = ((float64*)samples) + sampleChannel;
			sample = (float32)(*pSample);
		}
		else
		{
			sample = 0.0f;
		}
		break;
	}
	default:
		sample = 0.0f;
	}

	return sample;
}


bool SoundEx::ChannelsPlaying()
{
	LockAudio();
	bool ret = false;
	for (const auto& channel : audioCtx.channels)
	{
		if (channel.inUse && channel.playing)
		{
			ret = true;
			goto finished;
		}
	}
finished:
	UnlockAudio();
	return ret;
}


const wchar* SoundEx::GetLastAudioError()
{
	LockAudio();
	const wchar* ret;
	switch (audioCtx.lastError)
	{
	case AudioError_None:
	default:
		ret = L"";
		break;
	case AudioError_Error:
		ret = L"An undefined error occurred";
		break;
	case AudioError_FileNotOpen:
		ret = L"Failed to open sound file (file might not exist)";
		break;
	case AudioError_InvalidSignature:
		ret = L"Signature of sound file was not recognized (expected either RIFF or OggS, got neither)";
		break;
	case AudioError_WAVLoadFailed:
		ret = L"Error occurred during .wav file parsing";
		break;
	case AudioError_OggLoadFailed:
		ret = L"Error occurred during .ogg/oga file parsing";
		break;
	case AudioError_InvalidChannel:
		ret = L"An invalid channel number was entered";
		break;
	case AudioError_ChannelNotUsed:
		ret = L"There are no sounds playing on the channel number specified";
		break;
	case AudioError_ChannelNotFound:
		ret = L"No channels found with the specified sound name";
		break;
	case AudioError_InvalidSoundName:
		ret = L"An invalid sound name was entered (either empty, or the amount of characters is higher than the maximum)";
		break;
	case AudioError_UnknownSoundName:
		ret = L"An unknown sound name was specified";
		break;
	case AudioError_SeekFailure:
		ret = L"An error occurred when setting channel position (position may be out of bounds)";
		break;
	case AudioError_InvalidSample:
		ret = L"An invalid sample position was entered (probably out of bounds)";
		break;
	case AudioError_InvalidSampleChannel:
		ret = L"An invalid sample channel was entered (i.e, to get sample channel of left-side (or the sound is mono), 0 must be entered, likewise 1 must be entered for right-side)";
		break;
	case AudioError_NotWAV:
		ret = L"This operation is only supported for .WAV";
		break;
	case AudioError_InvalidMS:
		ret = L"Delay milliseconds must be greater than 0";
		break;
	case AudioError_OutOfMemory:
		ret = L"Not enough memory available to complete the operation!";
		break;
	}

	UnlockAudio();
	return ret;
}

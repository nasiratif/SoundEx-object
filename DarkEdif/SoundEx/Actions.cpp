#include "Common.hpp"

using namespace SoundEx;

/*

---------- UTILITY FUNCTIONS ---------

*/

FXReverbParams DefaultFXReverbParams()
{
	FXReverbParams params;
	params.diffusion = 0.5f;
	params.roomSize = 0.5f;
	return params;
}

PrincetonReverbParams DefaultPrincetonParams()
{
	PrincetonReverbParams params;
	params.wetDryMix = AUDIO_FXPRINCETON_DEFAULT_WET_DRY_MIX;
	params.reflectionsDelay = AUDIO_FXPRINCETON_DEFAULT_REFLECTIONS_DELAY;
	params.reverbDelay = AUDIO_FXPRINCETON_DEFAULT_REVERB_DELAY;
	params.rearDelay = AUDIO_FXPRINCETON_DEFAULT_REAR_DELAY;
	params.sideDelay = AUDIO_FXPRINCETON_DEFAULT_SIDE_DELAY;
	params.posLeft = AUDIO_FXPRINCETON_DEFAULT_POSITION;
	params.posRight = AUDIO_FXPRINCETON_DEFAULT_POSITION;
	params.posMatrixLeft = AUDIO_FXPRINCETON_DEFAULT_POSITION_MATRIX;
	params.posMatrixRight = AUDIO_FXPRINCETON_DEFAULT_POSITION_MATRIX;
	params.earlyDiffusion = AUDIO_FXPRINCETON_DEFAULT_EARLY_DIFFUSION;
	params.lateDiffusion = AUDIO_FXPRINCETON_DEFAULT_LATE_DIFFUSION;
	params.lowEQGain = AUDIO_FXPRINCETON_DEFAULT_LOW_EQ_GAIN;
	params.lowEQCutoff = AUDIO_FXPRINCETON_DEFAULT_LOW_EQ_CUTOFF;
	params.highEQGain = AUDIO_FXPRINCETON_DEFAULT_HIGH_EQ_GAIN;
	params.highEQCutoff = AUDIO_FXPRINCETON_DEFAULT_HIGH_EQ_CUTOFF;
	params.roomFilterFreq = AUDIO_FXPRINCETON_DEFAULT_ROOM_FILTER_FREQ;
	params.roomFilterMain = AUDIO_FXPRINCETON_DEFAULT_ROOM_FILTER_MAIN;
	params.roomFilterHF = AUDIO_FXPRINCETON_DEFAULT_ROOM_FILTER_HF;
	params.reflectionsGain = AUDIO_FXPRINCETON_DEFAULT_REFLECTIONS_GAIN;
	params.reverbGain = AUDIO_FXPRINCETON_DEFAULT_REVERB_GAIN;
	params.decayTime = AUDIO_FXPRINCETON_DEFAULT_DECAY_TIME;
	params.density = AUDIO_FXPRINCETON_DEFAULT_DENSITY;
	params.roomSize = AUDIO_FXPRINCETON_DEFAULT_ROOM_SIZE;
	params.disableLateField = AUDIO_FXPRINCETON_DEFAULT_DISABLE_LATE_FIELD;
	return params;
}

/*

---------- MAIN FUNCTIONS ---------

*/

void Extension::LoadSound(const TCHAR* fileName, const TCHAR* name)
{
	LoadWaveData(fileName, name);
}


void Extension::PlaySound(const TCHAR* name, int channel, int loop)
{
	StartChannel(name, channel, FUSION_BOOL(loop), 1.0f, 1.0f, 0.0f, 0);
}

void Extension::StopSound(int channel)
{
	StopChannel(channel);
}


void Extension::PauseSound(int channel)
{
	PauseChannel(channel);
}

void Extension::ResumeSound(int channel)
{
	ResumeChannel(channel);
}


void Extension::StopAllSounds()
{
	StopAllChannels();
}

void Extension::PauseAllSounds()
{
	PauseAllChannels();
}

void Extension::ResumeAllSounds()
{
	ResumeAllChannels();
}


void Extension::SetMasterSoundVolume(float volume)
{
	SetMasterVolume(volume);
}


void Extension::SetSoundVolume(int channel, float volume)
{
	SetChannelVolume(channel, volume);
}

void Extension::SetSoundFrequency(int channel, float frequency)
{
	SetChannelFrequency(channel, frequency);
}

void Extension::SetSoundPan(int channel, float pan)
{
	SetChannelPan(channel, pan);
}

void Extension::SetSoundPosition(int channel, int sample)
{
	SetChannelPosition(channel, sample);
}


void Extension::SetSoundFXReverb(int channel)
{
	auto params = DefaultFXReverbParams();
	SetChannelFXReverb(channel, params);
}

void Extension::SetSoundFXPrinceton(int channel)
{
	auto params = DefaultPrincetonParams();
	SetChannelFXPrinceton(channel, params);
}


void Extension::SetSoundFXReverbDiffusion(int channel, float diffusion)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.fxReverb;
		params.diffusion = diffusion;

		UnlockAudio();
		SetChannelFXReverb(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXReverbRoomSize(int channel, float roomSize)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.fxReverb;
		params.roomSize = roomSize;

		UnlockAudio();
		SetChannelFXReverb(channel, params);
		return;
	}

finished:
	UnlockAudio();
}


void Extension::SetSoundFXPrincetonWetDryMix(int channel, float wetDryMix)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.wetDryMix = wetDryMix;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonReflectionsDelay(int channel, int reflectionsDelay)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.reflectionsDelay = reflectionsDelay;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonReverbDelay(int channel, int reverbDelay)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.reverbDelay = reverbDelay;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonRearDelay(int channel, int rearDelay)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.rearDelay = rearDelay;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonSideDelay(int channel, int sideDelay)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.sideDelay = sideDelay;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonPositionLeft(int channel, int posLeft)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.posLeft = posLeft;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonPositionRight(int channel, int posRight)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.posRight = posRight;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonPositionMatrixLeft(int channel, int posMatrixLeft)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.posMatrixLeft = posMatrixLeft;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonPositionMatrixRight(int channel, int posMatrixRight)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.posMatrixRight = posMatrixRight;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonEarlyDiffusion(int channel, int earlyDiffusion)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.earlyDiffusion = earlyDiffusion;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonLateDiffusion(int channel, int lateDiffusion)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.lateDiffusion = lateDiffusion;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonLowEQGain(int channel, int lowEQGain)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.lowEQGain = lowEQGain;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonLowEQCutoff(int channel, int lowEQCutoff)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.lowEQCutoff = lowEQCutoff;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonHighEQGain(int channel, int highEQGain)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.highEQGain = highEQGain;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonHighEQCutoff(int channel, int highEQCutoff)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.highEQCutoff = highEQCutoff;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonRoomFilterFreq(int channel, float roomFilterFreq)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.roomFilterFreq = roomFilterFreq;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonRoomFilterMain(int channel, float roomFilterMain)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.roomFilterMain = roomFilterMain;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonRoomFilterHF(int channel, float roomFilterHF)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.roomFilterHF = roomFilterHF;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonReflectionsGain(int channel, float reflectionsGain)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.reflectionsGain = reflectionsGain;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonReverbGain(int channel, float reverbGain)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.reverbGain = reverbGain;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonDecayTime(int channel, float decayTime)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.decayTime = decayTime;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonDensity(int channel, float density)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.density = density;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonRoomSize(int channel, float roomSize)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.roomSize = roomSize;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}

void Extension::SetSoundFXPrincetonLateField(int channel, int disableLateField)
{
	LockAudio();

	audioCtx.lastError = AudioError_None;
	if (channel < 0 || channel >= AUDIO_NUM_CHANNELS)
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

		auto& params = pChannel.princeton;
		params.disableLateField = disableLateField;

		UnlockAudio();
		SetChannelFXPrinceton(channel, params);
		return;
	}

finished:
	UnlockAudio();
}


void Extension::ClearSoundFX(int channel)
{
	ClearFX(channel);
}


void Extension::EnableReverbTails(int channel, int reverbTails)
{
	SetReverbTails(channel, FUSION_BOOL(reverbTails));
}


void Extension::UnloadSound(const TCHAR* name)
{
	UnloadWave(name);
}

void Extension::UnloadAllSounds()
{
	UnloadAllWaves();
}


void Extension::PlaySoundEx(const TCHAR* name, int channel, int loop, float volume, float frequency, float pan, int ms)
{
	StartChannel(name, channel, loop, volume, frequency, pan, ms);
}


void Extension::ClearSoundQueue()
{
	ClearQueue();
}

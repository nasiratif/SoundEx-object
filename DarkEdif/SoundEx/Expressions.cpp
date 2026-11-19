#include "Common.hpp"
#include "Extension.hpp"

using namespace SoundEx;

const TCHAR* Extension::GetLastACEError()
{
	return GetLastAudioError();
}


int Extension::GetSampleLength(const TCHAR* name)
{
	auto wave = GetWave(name);
	return wave ? (int)wave->numSamples : 0;
}

int Extension::GetSampleRate(const TCHAR* name)
{
	auto wave = GetWave(name);
	return wave ? (int)wave->sampleRate : 0;
}

int Extension::GetSampleBitrate(const TCHAR* name)
{
	auto wave = GetWave(name);
	return wave ? (int)wave->bitRate : 0;
}

int Extension::GetSampleChannels(const TCHAR* name)
{
	auto wave = GetWave(name);
	return wave ? (int)wave->numChannels : 0;
}


float Extension::GetMasterSoundVolume()
{
	return GetMasterVolume();
}


float Extension::GetSoundVolume(int channel)
{
	return GetChannelVolume(channel);
}

float Extension::GetSoundFrequency(int channel)
{
	return GetChannelFrequency(channel);
}

float Extension::GetSoundPan(int channel)
{
	return GetChannelPan(channel);
}

int Extension::GetSoundPosition(int channel)
{
	return GetChannelPosition(channel);
}

const TCHAR* Extension::GetSoundWaveName(int channel)
{
	auto wave = GetChannelWave(channel);
	return wave ? Runtime.CopyString(wave->name) : _T("");
}


float Extension::GetSoundFXReverbDiffusion(int channel)
{
	auto params = GetChannelFXReverbParams(channel);
	return params ? params->diffusion : 0.0f;
}

float Extension::GetSoundFXReverbRoomSize(int channel)
{
	auto params = GetChannelFXReverbParams(channel);
	return params ? params->roomSize : 0.0f;
}


float Extension::GetSoundFXPrincetonWetDryMix(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->wetDryMix : 0.0f;
}

int Extension::GetSoundFXPrincetonReflectionsDelay(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->reflectionsDelay : 0;
}

int Extension::GetSoundFXPrincetonReverbDelay(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->reverbDelay : 0;
}

int Extension::GetSoundFXPrincetonRearDelay(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->rearDelay : 0;
}

int Extension::GetSoundFXPrincetonSideDelay(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->sideDelay : 0;
}

int Extension::GetSoundFXPrincetonPositionLeft(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->posLeft : 0;
}

int Extension::GetSoundFXPrincetonPositionRight(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->posRight : 0;
}

int Extension::GetSoundFXPrincetonPositionMatrixLeft(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->posMatrixLeft : 0;
}

int Extension::GetSoundFXPrincetonPositionMatrixRight(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->posMatrixRight : 0;
}

int Extension::GetSoundFXPrincetonEarlyDiffusion(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->earlyDiffusion : 0;
}

int Extension::GetSoundFXPrincetonLateDiffusion(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->lateDiffusion : 0;
}

int Extension::GetSoundFXPrincetonLowEQGain(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->lowEQGain : 0;
}

int Extension::GetSoundFXPrincetonLowEQCutoff(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->lowEQCutoff : 0;
}

int Extension::GetSoundFXPrincetonHighEQGain(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->highEQGain : 0;
}

int Extension::GetSoundFXPrincetonHighEQCutoff(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->highEQCutoff : 0;
}

float Extension::GetSoundFXPrincetonRoomFilterFrequency(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->roomFilterFreq : 0.0f;
}

float Extension::GetSoundFXPrincetonRoomFilterMain(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->roomFilterMain : 0.0f;
}

float Extension::GetSoundFXPrincetonRoomFilterHF(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->roomFilterHF : 0.0f;
}

float Extension::GetSoundFXPrincetonReflectionsGain(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->reflectionsGain : 0.0f;
}

float Extension::GetSoundFXPrincetonReverbGain(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->reverbGain : 0.0f;
}

float Extension::GetSoundFXPrincetonDecayTime(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->decayTime : 0.0f;
}

float Extension::GetSoundFXPrincetonDensity(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->density : 0.0f;
}

float Extension::GetSoundFXPrincetonRoomSize(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->roomSize : 0.0f;
}

int Extension::GetSoundFXPrincetonDisableLateField(int channel)
{
	auto params = GetChannelFXPrincetonParams(channel);
	return params ? params->disableLateField : 0;
}


int Extension::GetLastSoundChannelPlayed()
{
	return GetLastChannelPlayed();
}

int Extension::FindSoundChannel(const TCHAR* name)
{
	return FindChannel(name);
}


int Extension::GetNumActiveSoundChannels()
{
	return GetNumActiveChannels();
}


float Extension::GetWAVAmplitude(const TCHAR* name, int sample, int sampleChannel)
{
	return GetWaveAmplitude(name, sample, sampleChannel);
}


float Extension::GetSoundAmplitude(int channel, int sampleChannel)
{
	return GetChannelAmplitude(channel, sampleChannel);
}


int Extension::GetNumSounds()
{
	return GetNumWaves();
}

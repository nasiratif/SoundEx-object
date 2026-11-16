#include "Common.hpp"

using namespace SoundEx;

bool Extension::HasErrorOccurred()
{
	LockAudio();
	auto ret = audioCtx.lastError != AudioError_None;
	UnlockAudio();
	return ret;
}


bool Extension::IsAnyChannelPlaying()
{
	return ChannelsPlaying();
}


bool Extension::IsChannelPlaying(int channel)
{
	return GetChannelPlaying(channel);
}

bool Extension::IsChannelLooping(int channel)
{
	return GetChannelLoop(channel);
}

bool Extension::IsChannelUsingReverbTails(int channel)
{
	return GetChannelReverbTails(channel);
}


bool Extension::IsChannelUsingFX(int channel)
{
	return GetChannelFX(channel) != AudioEffect_None;
}

bool Extension::IsChannelUsingFXReverb(int channel)
{
	return GetChannelFX(channel) == AudioEffect_FXReverb;
}

bool Extension::IsChannelUsingFXPrinceton(int channel)
{
	return GetChannelFX(channel) == AudioEffect_Princeton;
}

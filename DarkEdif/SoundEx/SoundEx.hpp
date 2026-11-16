#pragma once
#include <cstdint>
#include <vector>
#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#include <algorithm>

// STANDARD TYPES:
// -----
typedef std::uint8_t uint8, byte, uchar;
typedef std::int8_t int8;
typedef std::int32_t int32;
typedef std::int16_t int16;
typedef std::uint16_t uint16, word;
typedef std::uint32_t uint32, dword;
typedef wchar_t wchar;
typedef std::uint64_t uint64;
typedef float float32;
typedef double float64;
// -----

typedef void* File;

// DEFINES:
// -----
#define SOUNDEX_NUM_CHANNELS 64
#define SOUNDEX_NUM_STREAM_BUFFERS 2
#define SOUNDEX_STREAM_BUFFER_SAMPLES 32768

#define SOUNDEX_MAX_WAVE_NAME_LENGTH 64

#define SOUNDEX_CHANNEL_AUTO -1

#define SOUNDEX_FXPRINCETON_DEFAULT_WET_DRY_MIX 100.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_REFLECTIONS_DELAY 5
#define SOUNDEX_FXPRINCETON_DEFAULT_REVERB_DELAY 5
#define SOUNDEX_FXPRINCETON_DEFAULT_REAR_DELAY 5
#define SOUNDEX_FXPRINCETON_DEFAULT_SIDE_DELAY 5
#define SOUNDEX_FXPRINCETON_DEFAULT_POSITION 6
#define SOUNDEX_FXPRINCETON_DEFAULT_POSITION_MATRIX 27
#define SOUNDEX_FXPRINCETON_DEFAULT_EARLY_DIFFUSION 8
#define SOUNDEX_FXPRINCETON_DEFAULT_LATE_DIFFUSION 8
#define SOUNDEX_FXPRINCETON_DEFAULT_LOW_EQ_GAIN 8
#define SOUNDEX_FXPRINCETON_DEFAULT_LOW_EQ_CUTOFF 4
#define SOUNDEX_FXPRINCETON_DEFAULT_HIGH_EQ_GAIN 8
#define SOUNDEX_FXPRINCETON_DEFAULT_HIGH_EQ_CUTOFF 4
#define SOUNDEX_FXPRINCETON_DEFAULT_ROOM_FILTER_FREQ 5000.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_ROOM_FILTER_MAIN 0.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_ROOM_FILTER_HF 0.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_REFLECTIONS_GAIN 0.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_REVERB_GAIN 0.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_DECAY_TIME 1.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_DENSITY 100.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_ROOM_SIZE 100.0f
#define SOUNDEX_FXPRINCETON_DEFAULT_DISABLE_LATE_FIELD 0

#define SOUNDEX_FILE_ERROR nullptr

// From the first field of the wave format header in the fmt chunk of .wav files
// ---
#define SOUNDEX_WAVE_FORMAT_PCM 1
#define SOUNDEX_WAVE_FORMAT_IEEE_FLOAT 0x0003
// ---
// -----

namespace SoundEx
{
	// Whether to play sounds in background (window unfocused) or not
	extern bool playInBG;

	// Sound file format
	enum AudioType : uint8
	{
		AudioType_WAV,
		AudioType_Ogg
	};

	// Four character codes for various sound file formats/chunks
	enum AudioFourCC : uint32
	{
		AudioFourCC_RIFF = 'FFIR',
		AudioFourCC_WAVE = 'EVAW',
		AudioFourCC_FMT = ' tmf',
		AudioFourCC_Data = 'atad',
		AudioFourCC_Ogg = 'SggO',
	};

	// Various errors that could happen when running A/C/Es (Actions/Conditions/Expressions)
	enum AudioError : uint8
	{
		AudioError_None,
		AudioError_Error,
		AudioError_FileNotOpen,
		AudioError_InvalidSignature,
		AudioError_WAVLoadFailed,
		AudioError_OggLoadFailed,
		AudioError_InvalidChannel,
		AudioError_ChannelNotUsed,
		AudioError_ChannelNotFound,
		AudioError_InvalidSoundName,
		AudioError_UnknownSoundName,
		AudioError_SeekFailure,
		AudioError_InvalidSample,
		AudioError_InvalidSampleChannel,
		AudioError_NotWAV,
		AudioError_InvalidMS,
		AudioError_OutOfMemory,
	};

	// Audio sound effect
	enum AudioEffect : uint8
	{
		AudioEffect_None,
		AudioEffect_FXReverb,
		AudioEffect_Princeton
	};

	// File seek directions
	enum FileDirection : uint8
	{
		FileDirection_Begin,
		FileDirection_Current,
		FileDirection_End
	};

#pragma pack(push, 1)
	// Interpretation of WAVEFORMATEX from Windows
	struct WaveFormat
	{
		uint16 format = 0;
		uint16 numChannels = 0;
		uint32 sampleRate = 0;
		uint32 averageBytes = 0;
		uint16 blockAlign = 0;
		uint16 bitRate = 0;
		uint16 extra = 0;
	};
#pragma pack(pop)

	// Describes a loaded sound file
	struct WaveData
	{
		wchar name[SOUNDEX_MAX_WAVE_NAME_LENGTH + 1];
		AudioType type;
		// For WAV, this is raw PCM, for Ogg it is just the Ogg file in memory
		std::shared_ptr<byte[]> samples;
		// (in bytes!!)
		uint32 sampleSize;
		uint32 numSamples;
		uint32 sampleRate;
		uint16 bitRate;
		uint8 bitRateBytes;
		uint8 numChannels;
		bool floatPCM;
	};

#pragma pack(push, 1)
	// Parameter structure for XAPO FXReverb
	struct FXReverbParams
	{
		float32 diffusion;
		float32 roomSize;
	};

	// Parameter structure for Princeton Digital Reverb (from XAudio2Fx)
	struct PrincetonReverbParams
	{
		float32 wetDryMix;
		uint32 reflectionsDelay;
		uint8 reverbDelay;
		uint8 rearDelay;
		uint8 sideDelay;
		uint8 posLeft;
		uint8 posRight;
		uint8 posMatrixLeft;
		uint8 posMatrixRight;
		uint8 earlyDiffusion;
		uint8 lateDiffusion;
		uint8 lowEQGain;
		uint8 lowEQCutoff;
		uint8 highEQGain;
		uint8 highEQCutoff;
		float32 roomFilterFreq;
		float32 roomFilterMain;
		float32 roomFilterHF;
		float32 reflectionsGain;
		float32 reverbGain;
		float32 decayTime;
		float32 density;
		float32 roomSize;
		int32 disableLateField;
	};
#pragma pack(pop)

	struct Channel
	{
		bool inUse;

		bool playing;
		bool loop;
		float32 volume;
		float32 frequency;
		float32 pan;
		AudioEffect effect;
		FXReverbParams fxReverb;
		PrincetonReverbParams princeton;
		bool reverbTails;

		uint64 sampleStart;
		uint64 sampleLooped;
		uint32 numSamples;
		std::shared_ptr<WaveData> wave;

		stb_vorbis* vorbis;
		// (this is used as a multi-dimensional array)
		std::shared_ptr<int16[]> vorbisSamples;
		// Capacity, in elements, for each element of the vorbis samples array (SOUNDEX_STREAM_BUFFER_SAMPLES * numChannels)
		int32 vorbisSamplesCapacity;
		int32 vorbisSamplesSize;
		uint32 nextVorbisBuffer;
		bool streamEnding;

		// Platform-specifc voice pointer (IXAudio2SourceVoice*)
		void* pAudio;
		// Platform-specifc voice callback
		void* pCallback;
	};

	extern struct AudioContext
	{
		float32 masterVolume;
		bool muted;

		// Loaded sound files
		std::vector<std::shared_ptr<WaveData>> waves;

		Channel channels[SOUNDEX_NUM_CHANNELS];
		AudioError lastError;
		int32 lastChannelPlayed;
	} audioCtx;

	// Initialize the SoundEx audio engine
	bool Init();
	// Performs platform-specific initialization, this is called by Init()
	// The 'recovering' parameter is used to determine whether this was only called due to XAudio2 needing to restart
	bool PlatformInit(bool recovering = false);

	void Release();
	void PlatformRelease();

	// Lock the audio thread for synchronization with the main thread
	void LockAudio();
	// Unlock the audio thread for synchronization with the main thread
	void UnlockAudio();

	// Lock only the callback portions of the audio thread for synchronization with the main thread
	void LockCallbacks();
	// Unlock only the callback portions of the audio thread for synchronization with the main thread
	void UnlockCallbacks();
	// Attempts to lock the callback portions of the audio thread
	bool TryLockCallbacks();

	// Updates the audio engine, this is to be called in the audio thread
	void Update();

	void SetMasterVolume(float volume);

	void LoadWaveData(const wchar* fileName, const wchar* name);
	void UnloadWave(const wchar* name);
	void UnloadAllWaves();

	void StartChannel(const wchar* name, int channel, bool loop, float volume, float frequency, float pan, int ms);
	void StopChannel(int channel);
	void ClearQueue();
	void ReleaseChannel(Channel& channel);

	void PauseChannel(int channel);
	void ResumeChannel(int channel);

	void StopAllChannels();
	void PauseAllChannels();
	void ResumeAllChannels();

	void SetChannelVolume(int channel, float volume);
	void SetChannelFrequency(int channel, float frequency);
	void SetChannelPan(int channel, float pan);
	void SetChannelPosition(int channel, int sample);

	void SetChannelFXReverb(int channel, const FXReverbParams& params);
	void SetChannelFXPrinceton(int channel, const PrincetonReverbParams& params);
	void ClearFX(int channel);

	void SetReverbTails(int channel, bool reverbTails);

	// Returnes an index into the first channel found that uses the specified sound name, otherwise -1 if not found
	int FindChannel(const wchar* name);

	std::shared_ptr<WaveData> GetWave(const wchar* name);
	float GetWaveAmplitude(const wchar* name, int sample, int sampleChannel);

	float GetMasterVolume();

	bool GetChannelPlaying(int channel);
	bool GetChannelLoop(int channel);
	float GetChannelVolume(int channel);
	float GetChannelFrequency(int channel);
	float GetChannelPan(int channel);
	int GetChannelPosition(int channel);
	float GetChannelAmplitude(int channel, int sampleChannel);
	std::shared_ptr<WaveData> GetChannelWave(int channel);

	AudioEffect GetChannelFX(int channel);

	bool GetChannelReverbTails(int channel);

	const FXReverbParams* GetChannelFXReverbParams(int channel);
	const PrincetonReverbParams* GetChannelFXPrincetonParams(int channel);

	int GetLastChannelPlayed();

	int GetNumWaves();
	int GetNumActiveChannels();

	float GetSampleAmplitude(const std::shared_ptr<WaveData>& wave, const byte* samples, const byte* samplesEnd, int sampleChannel);

	bool ChannelsPlaying();

	void PlatformSetMasterVolume(float volume);

	void PlatformCreateChannel(Channel& channel);
	void PlatformQueueChannel(const wchar* name, int channel, bool loop, float volume, float frequency, float pan, int ms);
	void PlatformStopChannel(Channel& channel);
	void PlatformClearQueue();
	void PlatformPauseChannel(Channel& channel);
	void PlatformResumeChannel(Channel& channel);

	void PlatformSetChannelVolume(Channel& channel, float volume);
	void PlatformSetChannelFrequency(Channel& channel, float frequency);
	void PlatformSetChannelPan(Channel& channel, float pan);
	void PlatformSetChannelPosition(Channel& channel, int sample);

	void PlatformSetFXReverb(Channel& channel, const FXReverbParams& params, bool forceCreateFX = false);
	void PlatformSetFXPrinceton(Channel& channel, const PrincetonReverbParams& params, bool forceCreateFX = false);
	void PlatformClearFX(Channel& channel);

	uint64 PlatformUpdateChannel(Channel& channel);

	int PlatformGetChannelPosition(Channel& channel, bool sampleStart = true);
	 
	uint32 GetBufferQueue(Channel& channel);

	const wchar* GetLastAudioError();

	// FILE IO:
	// -----
	bool FileOpen(const wchar* fileName, File& outFile);
	void FileClose(File file);

	uint32 FileRead(File file, void* buffer, uint32 bufferSize);
	uint32 FileSeek(File file, uint32 offset, FileDirection direction);
	uint64 FileSize(File file);
	// -----
}

#pragma once
#include "DarkEdif.hpp"
#include "SoundEx.hpp"

// Create a boolean out of an int value passed by Fusion, anything other than 0 is treated as true
#define FUSION_BOOL(v) ((v) ? true : false)

class Extension final
{
public:
	// ======================================
	// Required variables + functions
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;
#ifdef __ANDROID__
	global<jobject> javaExtPtr;
#elif defined(__APPLE__)
	void* const objCExtPtr;
#endif

	Edif::Runtime Runtime;

	static const int MinimumBuild = 296;
	static const int Version = 6;

	// Warning: OEFLAGS/OEPREFS cannot be freely modified when you have used them in MFAs.
	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;
	// If OEFLAGS::WINDOW_PROC (otherwise you can delete)
	static constexpr int WindowProcPriority = 100;
	// If OEFLAGS::TEXT (otherwise you can delete)
	// static constexpr TextCapacity TextCapacity = TextCapacity::None;

#ifdef _WIN32
	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
#elif defined(__ANDROID__)
	Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr);
#else
	Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr);
#endif
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	// To add items to the Fusion Debugger, just uncomment this line.
	DarkEdif::FusionDebugger FusionDebugger;
	// After enabling it, you run FusionDebugger.AddItemToDebugger() inside Extension's constructor
	// As an example:
	std::tstring exampleDebuggerTextItem;


	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr in old SDKs).

		Unlike rdPtr, you can store real C++ objects with constructors
		and destructors, without having to call them manually or store
		a pointer.
	*/

	// int MyVariable;


	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/

	// Actions

	void LoadSound(const TCHAR* fileName, const TCHAR* name);

	void PlaySound(const TCHAR* name, int channel, int loop);
	void StopSound(int channel);

	void PauseSound(int channel);
	void ResumeSound(int channel);

	void StopAllSounds();
	void PauseAllSounds();
	void ResumeAllSounds();

	void SetMasterSoundVolume(float volume);

	void SetSoundVolume(int channel, float volume);
	void SetSoundFrequency(int channel, float frequency);
	void SetSoundPan(int channel, float pan);
	void SetSoundPosition(int channel, int sample);

	void SetSoundFXReverb(int channel);
	void SetSoundFXPrinceton(int channel);

	void SetSoundFXReverbDiffusion(int channel, float diffusion);
	void SetSoundFXReverbRoomSize(int channel, float roomSize);

	void SetSoundFXPrincetonWetDryMix(int channel, float wetDryMix);
	void SetSoundFXPrincetonReflectionsDelay(int channel, int reflectionsDelay);
	void SetSoundFXPrincetonReverbDelay(int channel, int reverbDelay);
	void SetSoundFXPrincetonRearDelay(int channel, int rearDelay);
	void SetSoundFXPrincetonSideDelay(int channel, int sideDelay);
	void SetSoundFXPrincetonPositionLeft(int channel, int posLeft);
	void SetSoundFXPrincetonPositionRight(int channel, int posRight);
	void SetSoundFXPrincetonPositionMatrixLeft(int channel, int posMatrixLeft);
	void SetSoundFXPrincetonPositionMatrixRight(int channel, int posMatrixRight);
	void SetSoundFXPrincetonEarlyDiffusion(int channel, int earlyDiffusion);
	void SetSoundFXPrincetonLateDiffusion(int channel, int lateDiffusion);
	void SetSoundFXPrincetonLowEQGain(int channel, int lowEQGain);
	void SetSoundFXPrincetonLowEQCutoff(int channel, int lowEQCutoff);
	void SetSoundFXPrincetonHighEQGain(int channel, int highEQGain);
	void SetSoundFXPrincetonHighEQCutoff(int channel, int highEQCutoff);
	void SetSoundFXPrincetonRoomFilterFreq(int channel, float roomFilterFreq);
	void SetSoundFXPrincetonRoomFilterMain(int channel, float roomFilterMain);
	void SetSoundFXPrincetonRoomFilterHF(int channel, float roomFilterHF);
	void SetSoundFXPrincetonReflectionsGain(int channel, float reflectionsGain);
	void SetSoundFXPrincetonReverbGain(int channel, float reverbGain);
	void SetSoundFXPrincetonDecayTime(int channel, float decayTime);
	void SetSoundFXPrincetonDensity(int channel, float density);
	void SetSoundFXPrincetonRoomSize(int channel, float roomSize);
	void SetSoundFXPrincetonLateField(int channel, int disableLateField);

	void ClearSoundFX(int channel);

	void EnableReverbTails(int channel, int reverbTails);

	void UnloadSound(const TCHAR* name);
	void UnloadAllSounds();

	void PlaySoundEx(const TCHAR* name, int channel, int loop, float volume, float frequency, float pan, int ms);

	void ClearSoundQueue();

	// Conditions

	bool HasErrorOccurred();

	bool IsAnyChannelPlaying();

	bool IsChannelPlaying(int channel);
	bool IsChannelLooping(int channel);
	bool IsChannelUsingReverbTails(int channel);

	bool IsChannelUsingFX(int channel);
	bool IsChannelUsingFXReverb(int channel);
	bool IsChannelUsingFXPrinceton(int channel);

	// Expressions

	const TCHAR* GetLastACEError();

	int GetSampleLength(const TCHAR* name);
	int GetSampleRate(const TCHAR* name);
	int GetSampleBitrate(const TCHAR* name);
	int GetSampleChannels(const TCHAR* name);

	float GetMasterSoundVolume();

	float GetSoundVolume(int channel);
	float GetSoundFrequency(int channel);
	float GetSoundPan(int channel);
	int GetSoundPosition(int channel);
	const TCHAR* GetSoundWaveName(int channel);

	float GetSoundFXReverbDiffusion(int channel);
	float GetSoundFXReverbRoomSize(int channel);

	float GetSoundFXPrincetonWetDryMix(int channel);
	int GetSoundFXPrincetonReflectionsDelay(int channel);
	int GetSoundFXPrincetonReverbDelay(int channel);
	int GetSoundFXPrincetonRearDelay(int channel);
	int GetSoundFXPrincetonSideDelay(int channel);
	int GetSoundFXPrincetonPositionLeft(int channel);
	int GetSoundFXPrincetonPositionRight(int channel);
	int GetSoundFXPrincetonPositionMatrixLeft(int channel);
	int GetSoundFXPrincetonPositionMatrixRight(int channel);
	int GetSoundFXPrincetonEarlyDiffusion(int channel);
	int GetSoundFXPrincetonLateDiffusion(int channel);
	int GetSoundFXPrincetonLowEQGain(int channel);
	int GetSoundFXPrincetonLowEQCutoff(int channel);
	int GetSoundFXPrincetonHighEQGain(int channel);
	int GetSoundFXPrincetonHighEQCutoff(int channel);
	float GetSoundFXPrincetonRoomFilterFrequency(int channel);
	float GetSoundFXPrincetonRoomFilterMain(int channel);
	float GetSoundFXPrincetonRoomFilterHF(int channel);
	float GetSoundFXPrincetonReflectionsGain(int channel);
	float GetSoundFXPrincetonReverbGain(int channel);
	float GetSoundFXPrincetonDecayTime(int channel);
	float GetSoundFXPrincetonDensity(int channel);
	float GetSoundFXPrincetonRoomSize(int channel);
	int GetSoundFXPrincetonDisableLateField(int channel);

	int GetLastSoundChannelPlayed();
	int FindSoundChannel(const TCHAR* name);

	int GetNumActiveSoundChannels();

	float GetWAVAmplitude(const TCHAR* name, int sample, int sampleChannel);

	float GetSoundAmplitude(int channel, int sampleChannel);

	int GetNumSounds();

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

#if TEXT_OEFLAG_EXTENSION
	// Extension text struct. Required for text exts.
	DarkEdif::FontInfoMultiPlat font;
	void OnFontChanged(bool colorEdit, DarkEdif::Rect* rc);
#endif
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
	// Extension display surface ptr. Required for simple display exts.
	DarkEdif::Surface * surf = nullptr;
#elif DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL
	void Display();
	void GetZoneInfos();
	DarkEdif::Surface * GetDisplaySurface();
	DarkEdif::CollisionMask * GetCollisionMask(std::uint32_t flags);
#endif

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);

#if PAUSABLE_EXTENSION
	// Called when Fusion runtime is pausing - not just the F3 pause dialog
	void FusionRuntimePaused();
	// Called when Fusion runtime is resuming after a pause
	void FusionRuntimeContinued();
#endif // PAUSABLE_EXTENSION
};

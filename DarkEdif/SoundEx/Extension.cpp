#include "Common.hpp"
#include <iomanip>
using namespace std::chrono_literals;

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///
/// 

#ifdef _WIN32
Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this), FusionDebugger(this)
#elif defined(__ANDROID__)
Extension::Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr) :
	javaExtPtr(javaExtPtr, "Extension::javaExtPtr from Extension ctor"),
	Runtime(this, this->javaExtPtr), FusionDebugger(this)
#else
Extension::Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr) :
	objCExtPtr(objCExtPtr), Runtime(this, objCExtPtr), FusionDebugger(this)
#endif
{
	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/
	
	LinkAction(0, LoadSound);
	LinkAction(1, PlaySound);
	LinkAction(2, StopSound);
	LinkAction(3, PauseSound);
	LinkAction(4, ResumeSound);
	LinkAction(5, StopAllSounds);
	LinkAction(6, PauseAllSounds);
	LinkAction(7, ResumeAllSounds);
	LinkAction(8, SetMasterSoundVolume);
	LinkAction(9, SetSoundVolume);
	LinkAction(10, SetSoundFrequency);
	LinkAction(11, SetSoundPan);
	LinkAction(12, SetSoundPosition);

	LinkAction(13, SetSoundFXReverb);
	LinkAction(14, SetSoundFXPrinceton);

	LinkAction(15, SetSoundFXReverbDiffusion);
	LinkAction(16, SetSoundFXReverbRoomSize);

	LinkAction(17, SetSoundFXPrincetonWetDryMix);
	LinkAction(18, SetSoundFXPrincetonReflectionsDelay);
	LinkAction(19, SetSoundFXPrincetonReverbDelay);
	LinkAction(20, SetSoundFXPrincetonRearDelay);
	LinkAction(21, SetSoundFXPrincetonSideDelay);
	LinkAction(22, SetSoundFXPrincetonPositionLeft);
	LinkAction(23, SetSoundFXPrincetonPositionRight);
	LinkAction(24, SetSoundFXPrincetonPositionMatrixLeft);
	LinkAction(25, SetSoundFXPrincetonPositionMatrixRight);
	LinkAction(26, SetSoundFXPrincetonEarlyDiffusion);
	LinkAction(27, SetSoundFXPrincetonLateDiffusion);
	LinkAction(28, SetSoundFXPrincetonLowEQGain);
	LinkAction(29, SetSoundFXPrincetonLowEQCutoff);
	LinkAction(30, SetSoundFXPrincetonHighEQGain);
	LinkAction(31, SetSoundFXPrincetonHighEQCutoff);
	LinkAction(32, SetSoundFXPrincetonRoomFilterFreq);
	LinkAction(33, SetSoundFXPrincetonRoomFilterMain);
	LinkAction(34, SetSoundFXPrincetonRoomFilterHF);
	LinkAction(35, SetSoundFXPrincetonReflectionsGain);
	LinkAction(36, SetSoundFXPrincetonReverbGain);
	LinkAction(37, SetSoundFXPrincetonDecayTime);
	LinkAction(38, SetSoundFXPrincetonDensity);
	LinkAction(39, SetSoundFXPrincetonRoomSize);
	LinkAction(40, SetSoundFXPrincetonLateField);

	LinkAction(41, ClearSoundFX);

	LinkAction(42, EnableReverbTails);

	LinkAction(43, UnloadSound);
	LinkAction(44, UnloadAllSounds);

	LinkAction(45, PlaySoundEx);

	LinkAction(46, ClearSoundQueue);

	LinkCondition(0, HasErrorOccurred);

	LinkCondition(1, IsAnyChannelPlaying);

	LinkCondition(2, IsChannelPlaying);
	LinkCondition(3, IsChannelLooping);
	LinkCondition(4, IsChannelUsingReverbTails);

	LinkCondition(5, IsChannelUsingFX);
	LinkCondition(6, IsChannelUsingFXReverb);
	LinkCondition(7, IsChannelUsingFXPrinceton);

	LinkExpression(0, GetLastACEError);
	LinkExpression(1, GetSampleLength);
	LinkExpression(2, GetSampleRate);
	LinkExpression(3, GetSampleBitrate);
	LinkExpression(4, GetSampleChannels);
	LinkExpression(5, GetMasterSoundVolume);
	LinkExpression(6, GetSoundVolume);
	LinkExpression(7, GetSoundFrequency);
	LinkExpression(8, GetSoundPan);
	LinkExpression(9, GetSoundPosition);
	LinkExpression(10, GetSoundWaveName);

	LinkExpression(11, GetSoundFXReverbDiffusion);
	LinkExpression(12, GetSoundFXReverbRoomSize);

	LinkExpression(13, GetSoundFXPrincetonWetDryMix);
	LinkExpression(14, GetSoundFXPrincetonReflectionsDelay);
	LinkExpression(15, GetSoundFXPrincetonReverbDelay);
	LinkExpression(16, GetSoundFXPrincetonRearDelay);
	LinkExpression(17, GetSoundFXPrincetonSideDelay);
	LinkExpression(18, GetSoundFXPrincetonPositionLeft);
	LinkExpression(19, GetSoundFXPrincetonPositionRight);
	LinkExpression(20, GetSoundFXPrincetonPositionMatrixLeft);
	LinkExpression(21, GetSoundFXPrincetonPositionMatrixRight);
	LinkExpression(22, GetSoundFXPrincetonEarlyDiffusion);
	LinkExpression(23, GetSoundFXPrincetonLateDiffusion);
	LinkExpression(24, GetSoundFXPrincetonLowEQGain);
	LinkExpression(25, GetSoundFXPrincetonLowEQCutoff);
	LinkExpression(26, GetSoundFXPrincetonHighEQGain);
	LinkExpression(27, GetSoundFXPrincetonHighEQCutoff);
	LinkExpression(28, GetSoundFXPrincetonRoomFilterFrequency);
	LinkExpression(29, GetSoundFXPrincetonRoomFilterMain);
	LinkExpression(30, GetSoundFXPrincetonRoomFilterHF);
	LinkExpression(31, GetSoundFXPrincetonReflectionsGain);
	LinkExpression(32, GetSoundFXPrincetonReverbGain);
	LinkExpression(33, GetSoundFXPrincetonDecayTime);
	LinkExpression(34, GetSoundFXPrincetonDensity);
	LinkExpression(35, GetSoundFXPrincetonRoomSize);
	LinkExpression(36, GetSoundFXPrincetonDisableLateField);

	LinkExpression(37, GetLastSoundChannelPlayed);
	LinkExpression(38, FindSoundChannel);

	LinkExpression(39, GetNumActiveSoundChannels);

	LinkExpression(40, GetWAVAmplitude);

	LinkExpression(41, GetSoundAmplitude);

	LinkExpression(42, GetNumSounds);

	/*
		This is where you'd do anything you'd do in CreateRunObject in the original SDK

		It's the only place you'll get access to edPtr at runtime, so you should transfer
		anything from edPtr to the extension class here.

	*/

	// Don't use "this" inside these lambda functions, always ext.
	// There can be nothing in the [] section of the lambda.
	// If you're not sure about lambdas, you can remove this debugger stuff without any side effects;
	// it's just an example of how to use the debugger. You can view it in Fusion itself to see.
	FusionDebugger.AddItemToDebugger(
		// Prefix before debugger value, and initial text; if we pass null for initial text, it uses reader func
		_T("Master volume: "sv), NULL,
		// reader function for your debug item
		[](Extension* const ext, std::tstring& writeTo)
		{
			writeTo = std::to_tstring(SoundEx::GetMasterVolume());
		},
		// writer function (can be null if you don't want user to be able to edit it in debugger)
		[](Extension* const ext, std::tstring& newText)
		{
			SoundEx::SetMasterVolume(std::stof(newText));
			return true; // accept the changes
		}, 100, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Number of sound files loaded: "sv), NULL,
		// reader function for your debug item
		[](Extension* const ext, std::tstring& writeTo)
		{
			writeTo = std::to_tstring(SoundEx::GetNumWaves());
		},
		NULL, 500, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Number of sounds active: "sv), NULL,
		// reader function for your debug item
		[](Extension* const ext, std::tstring& writeTo)
		{
			writeTo = std::to_tstring(SoundEx::GetNumActiveChannels()) + _T("/") + std::to_tstring(AUDIO_NUM_CHANNELS);
		},
		NULL, 100, NULL
	);

	// Read object DarkEdif properties; you can pass property name, or property index
	// This will work on all platforms the same way.
	// See edPtr->Props functions.
	//bool checkboxWithinFolder = edPtr->Props.IsPropChecked("Checkbox within folder"sv);
	//std::tstring editable6Text = edPtr->Props.GetPropertyStr("Editable 6"sv);

	//// These lines do nothing, but prevent the compiler warning the variables are unused
	//(void)checkboxWithinFolder;
	//(void)editable6Text;

#if TEXT_OEFLAG_EXTENSION
	// Copy from edittime data into runtime data
	font.CopyFromEditFont(this, edPtr->font);

	// Set Runtime.fontChangedFunc to trigger a function if the runtime
	// changes your ext's font via the built-in font actions
	Runtime.fontChangedFunc = &Extension::OnFontChanged;
#endif

#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
	Runtime.SetSurfaceWithSize(edPtr->objSize.width, edPtr->objSize.height);
	// surf is already constructed
	// To start, let's make the image a solid blue fill
	surf->FillImageWith(DarkEdif::SurfaceFill::Solid(DarkEdif::ColorRGB(0, 0, 127)));
#endif
}

Extension::~Extension()
{

}

// Runs every tick of Fusion's runtime, can be toggled off and back on
REFLAG Extension::Handle()
{
#ifdef _WIN32
	// Handle window focus:
	if (!SoundEx::playInBG)
	{
		auto window = GetFocus();
		auto parent = GetParent(window);
		// It's possible that the focus window might actually be an edit-box or a button object because those are seperate Win32 windows ofc:
		auto focused = window == rhPtr->rhHMainWin || window == rhPtr->rhHEditWin ||
			parent == rhPtr->rhHMainWin || parent == rhPtr->rhHEditWin;

		if (focused)
		{
			SoundEx::PlatformSetMasterVolume(SoundEx::audioCtx.masterVolume);
			SoundEx::audioCtx.muted = false;
		}
		else
		{
			SoundEx::PlatformSetMasterVolume(0.0f);
			SoundEx::audioCtx.muted = true;
		}
	}
#endif

	return REFLAG::NONE;
}

#if PAUSABLE_EXTENSION
// Called when Fusion runtime is pausing - not just the F3 pause dialog
void Extension::FusionRuntimePaused()
{

}

// Called when Fusion runtime is resuming after a pause
void Extension::FusionRuntimeContinued()
{

}
#endif // PAUSABLE_EXTENSION

#if TEXT_OEFLAG_EXTENSION
/* Triggered when the runtime changes the font name or size via built-in actions
 * @param colorEdit If true, font color was edited. If false, the font typeface was.
 * @param rect		If not null, contains a hint as to what size the object should be.
 *					This is based on differences between original font size and current one.
 *					You can ignore this hint, but it indicates you may want to call
 *					rdPtr->get_roc()->SetSize(). */
void Extension::OnFontChanged(bool colorEdit, DarkEdif::Rect * rect)
{
	// Prevent warnings about unused variables
	(void)colorEdit;
	(void)rect;

	// We could call Runtime.Redraw() to redraw ext immediately,
	// but we'll be patient and wait for next event tick
	rdPtr->get_roc()->set_changed(true);
	Runtime.Rehandle();
}

#endif

#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL

void Extension::Display()
{
	// You should manually draw the surface(s) here.
	// Surface::BlitToFrameWithExtEffects will let you draw with the ext's sprites/X/Y accounted for,
	// but note you should also manage the collision mask in GetCollisionMask().
	// If you have a single surface, and don't want simple drawing, use the GetDisplaySurface().
	//
	//surf->BlitToFrameWithExtEffects(this, Point { x offset, y offset });
}


void Extension::GetZoneInfos()
{
	LOGI(_T("##### Extension::GetZoneInfos() called.\n"));
	// You can safely do nothing here. Some extensions do:
	// rdPtr->get_rHo()->set_ImgWidth()/set_ImgHeight()
	// but it is done automatically in CF2.5, so shouldn't be necessary.
}

DarkEdif::Surface * Extension::GetDisplaySurface()
{
	// Old style MMF2 fade-in/fade-out transistions requires this function,
	// Auto-rendering of Sprite effects (alpha coeff etc) will be applied by returning a surface here.
	// If you have multiple surfaces, you should can return null here and draw them yourself.
	// If you return null here, Display() will be called instead.
	return nullptr;
}
DarkEdif::CollisionMask * Extension::GetCollisionMask(std::uint32_t flags)
{
	// If your ext supports fine collision, you should return the collision mask corresponding to
	// which pixels have collision and which don't. DarkEdif::Surface can generate collision masks,
	// but it can be computationally expensive.
	// If your ext uses only box collision, DE will detect that and never call this function.
	(void)flags;

	return nullptr;
}

#endif // DARKEDIF_DISPLAY_MANUAL

// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error, but try not to return null string and definitely crash it
	if ((std::size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)_T("");
	return 0;
}

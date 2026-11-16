# ![Ext icon](DarkEdif/SoundEx/Icon.png "Icon") SoundEx object

**SoundEx object** (Windows) is an advanced sound extension for Clickteam Fusion 2.5, utilizing features from the XAudio 2.9 Windows API. You can think of it as a sort-of successor to my older [irrKlang extension](https://github.com/nasiratif/irrKlang-object). It was designed to be a replacement for Fusion's built-in sound system mainly because of it's quirks, while also adding in additional features such as sample-accurate sample position retrieval (for *.wav*, at least), real-time reverb, amplitude reads & more.

## How to use the extension

[Download the latest release](https://github.com/nasiratif/SoundEx-object/releases) and unzip it to Fusion's root folder. Afterwards, in Fusion, click *Refresh* in the *Create new object* window and drop the extension onto your frame.

For more details, please see the extension's bundled HTML help file in the *About* section of the extension properties.

## How to build the source code

The extension source code resides in **DarkEdif/SoundEx**. It is based on a modified variant of the [DarkEdif SDK](https://github.com/SortaCore/MMF2Exts). Build the solution with Visual Studio 2026 & make sure you have the *Desktop development with C++* feature set installed.

What you'll specifically need, most importantly, is the *Windows SDK* (*10.0* or higher is good enough) and the *v145* toolset.

Please note that this extension can only be compiled under the Unicode build configurations.

## Credits

This extension uses the [stb_vorbis](https://github.com/nothings/stb) library to decode OGG files. Thank you stb for your work!
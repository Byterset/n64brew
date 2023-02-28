# Exporting sound and music for use with the engine

## credit

This engine is using the `sfz2n64` tool by James Lambert (https://github.com/lambertjamesd/sfz2n64 - Huge credit to him!) to convert .wav .ins & .aif|.aifc|.aiff files into a sound bank and an accompanying sound table for use with the integrated soundplayer.
This removes the need for outdated unstable win98 sound-tools and other shenanigans.


## to export sound effects and music
- convert audio to WAV, AIF or INS Format -> 22050hz
- loops that are embedded in the .wav format will be processed by the soundplayer(!)
- save the file in one of assets/sounds or assets/music both will automatically be searched for matching file types when building.
- running `make` will execute the `sfzn64` tool and create both a `sounds.sounds` and a `sounds.sounds.tbl` file. These are referenced by the specfile and built into the final rom.
- the build will also run the `generate_sound_ids.js` script which is used in the MAKEFILE to create a `clips.h` header file.
This header can be included in the code to reference the sound-clip you want to play.
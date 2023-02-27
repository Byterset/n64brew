#ifndef SEGMENTS_H
#define SEGMENTS_H

#include "constants.h"

// define stuff like _modelsSegmentRomStart
EXTERN_SEGMENT(models);
EXTERN_SEGMENT(sprites);
EXTERN_SEGMENT(collision);

EXTERN_SEGMENT(wbank);
EXTERN_SEGMENT(pbank);
EXTERN_SEGMENT(sfx);
EXTERN_SEGMENT(song);
//segment for new audio player test
EXTERN_SEGMENT(sounds);
EXTERN_SEGMENT(soundsTbl);
// EXTERN_SEGMENT(newaudio);

#define WBANK_START _wbankSegmentRomStart
#define PBANK_START _pbankSegmentRomStart
#define PBANK_END _pbankSegmentRomEnd
#define SFX_START _sfxSegmentRomStart
#define SFX_END _sfxSegmentRomEnd
#define SONG_START _songSegmentRomStart
#define SONG_END _songSegmentRomEnd
#define SOUNDS_START _soundsSegmentRomStart
#define SOUNDS_END _soundsSegmentRomEnd
#define SOUNDSTBL_START _soundsTblSegmentRomStart
#define SOUNDSTBL_END _soundsTblSegmentRomEnd
// #define NEWAUDIO_START _newaudioSegmentRomStart
// #define NEWAUDIO_END _newaudioSegmentRomEnd


#endif /* SEGMENTS_H */
#ifndef SEGMENTS_H
#define SEGMENTS_H

#include "constants.h"

// define stuff like _modelsSegmentRomStart
EXTERN_SEGMENT(models);
EXTERN_SEGMENT(sprites);
EXTERN_SEGMENT(collision);

//segment for new audio player test
EXTERN_SEGMENT(sounds);
EXTERN_SEGMENT(soundsTbl);
// EXTERN_SEGMENT(newaudio);

#define SOUNDS_START _soundsSegmentRomStart
#define SOUNDS_END _soundsSegmentRomEnd
#define SOUNDSTBL_START _soundsTblSegmentRomStart
#define SOUNDSTBL_END _soundsTblSegmentRomEnd



#endif /* SEGMENTS_H */
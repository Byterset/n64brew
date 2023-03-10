
#include <ultra64.h>

#include "ed64io_usb.h"

#include "ed64io_os_error.h"

#define PRINTF ed64PrintfSync2

extern OSErrorHandler __osCommonHandler;

typedef struct {
  u32 code;
  char* fmt;
} errorFormat;

// from ultra/usr/lib/PR/error.fmt for OS2.0J
static errorFormat errorfmts[] = {
    {1, "osCreateThread: stack pointer not aligned to 8 bytes (0x%x)"},
    {2, "osCreateThread: priority not in range [0-255] (%d)"},
    {3, "osStartThread: thread has bad state (running/runnable/other)"},
    {4, "osSetThreadPri: priority not in range [0-255] (%d)"},
    {5, "osCreateMesgQueue: message count not > 0 (%d)"},
    {6, "osSendMesg: flag not OS_MESG_NOBLOCK or OS_MESG_BLOCK (%d)"},
    {7, "osJamMesg: flag not OS_MESG_NOBLOCK or OS_MESG_BLOCK (%d)"},
    {8, "osRecvMesg: flag not OS_MESG_NOBLOCK or OS_MESG_BLOCK (%d)"},
    {9, "osSetEventMesg: unknown event type (%d)"},
    {10, "osMapTLB: index not in range [0-30] (%d)"},
    {11, "osMapTLB: asid argument not -1 or in range [0-255] (%d)"},
    {12, "osUnmapTLB: index not in range [0-30] (%d)"},
    {13, "osSetTLBASID: asid not in range [0-255] (%d)"},
    {14, "osAiSetFrequency: freq not in range [%d-%d] (%d)"},
    {15, "osAiSetNextBuffer: address not aligned to 8 bytes (0x%x)"},
    {16, "osAiSetNextBuffer: size not aligned to 8 bytes (0x%x)"},
    {17, "osDpSetNextBuffer: address not aligned to 8 bytes (0x%x)"},
    {18, "osDpSetNextBuffer: size not aligned to 8 bytes (0x%x)"},
    {19, "osPiRawReadIo: address not aligned to 4 bytes (0x%x)"},
    {20, "osPiRawWriteIo: address not aligned to 4 bytes (0x%x)"},
    {21, "osPiRawStartDma: direction not OS_READ or OS_WRITE (%d)"},
    {22, "osPiRawStartDma: device address not aligned to 2 bytes (0x%x)"},
    {23, "osPiRawStartDma: DRAM address not aligned to 8 bytes (0x%x)"},
    {24, "osPiRawStartDma: size not aligned to 2 bytes (%d)"},
    {25, "osPiRawStartDma: size not in range [0,16777216] (%d)"},
    {26, "osPiReadIo: address not aligned to 4 bytes (0x%x)"},
    {27, "osPiWriteIo: address not aligned to 4 bytes (0x%x)"},
    {28, "osPiStartDma: PI Manager not yet begun by osCreatePiManager"},
    {29, "osPiStartDma: priority not OS_MESG_PRI_[NORMAL|HIGH] (%d)"},
    {30, "osPiStartDma: direction not OS_READ or OS_WRITE (%d)"},
    {31, "osPiStartDma: device address not aligned to 2 bytes (0x%x)"},
    {32, "osPiStartDma: DRAM address not aligned to 8 bytes (0x%x)"},
    {33, "osPiStartDma: size not aligned to 2 bytes (%d)"},
    {34, "osPiStartDma: size not in range [0,16777216] (%d)"},
    {35, "osCreatePiManager: priority not in range [0-255] (%d)"},
    {36, "osViGetCurrentMode: VI Manager not yet begun"},
    {37, "osViGetCurrentFramebuffer: VI Manager not yet begun"},
    {38, "osViGetNextFramebuffer: VI Manager not yet begun"},
    {39, "osViSetXScale: value not in range [0.25,1.0] (%f)"},
    {40, "osViSetXScale: VI Manager not yet begun by osCreateViManager"},
    {41, "osViSetYScale: value not in range [0.05,1.0] (%f)"},
    {42, "osViSetYScale: VI Manager not yet begun by osCreateViManager"},
    {43, "osViSetSpecialFeatures: not a known feature value (%d)"},
    {44, "osViSetSpecialFeatures: VI Manager not yet begun"},
    {45, "osViSetMode: VI Manager not yet begun by osCreateViManager"},
    {46, "osViSetEvent: VI Manager not yet begun by osCreateViManager"},
    {47, "osViSwapBuffer: frame buffer not aligned to 64 bytes (0x%x)"},
    {48, "osViSwapBuffer: VI Manager not yet begun"},
    {49, "osCreateViManager: priority not in range [0-255] (%d)"},
    {50, "osCreateRegion: not a known alignment (%d)"},
    {51, "osCreateRegion: length (%d) too small for buffer size (%d)"},
    {52, "osMalloc: invalid or corrupt region (0x%x)"},
    {53, "osFree: invalid or corrupt region (0x%x)"},
    {54,
     "osFree: invalid address (0x%x) or\n                           corrupt "
     "region (0x%x)"},
    {55, "osGetRegionBufCount: invalid or corrupt region (0x%x)"},
    {56, "osGetRegionBufSize: invalid or corrupt region (0x%x)"},
    {57, "osSpTaskLoad: dram_stack not aligned to 16 bytes (0x%x)"},
    {58, "osSpTaskLoad: output_buff not aligned to 16 bytes (0x%x)"},
    {59, "osSpTaskLoad: output_buff_size not aligned to 16 bytes (0x%x)"},
    {60, "osSpTaskLoad: yield_data_ptr not aligned to 16 bytes (0x%x)"},
    {61,
     "osProfileInit: profile counter is running, call osProfileStop before "
     "init"},
    {62, "osProfileInit: profcnt is %d"},
    {63, "osProfileInit: histo_base pointer must be 32-bit aligned (%x)"},
    {64, "osProfileInit: text_start (%x) >= text_end (%x)"},
    {65, "osProfileInit: histo_size is an illegal size (%d)"},
    {66, "osProfileStart: microseconds is < PROF_MIN_INTERVAL (%d)"},
    {67, "osProfileStart: profiling has already been started"},
    {68, "osProfileStop: profiling has already been stopped"},
    {69, "osProfileStop: no profile timer to stop"},
    {70, "osReadHost: address not aligned to 8 bytes (0x%x)"},
    {71, "osReadHost: size either 0 or not aligned to 4 bytes (0x%x)"},
    {72, "osWriteHost: address not aligned to 8 bytes (0x%x)"},
    {73, "osWriteHost: size either 0 or not aligned to 4 bytes (0x%x)"},
    {74, "osGetTime: VI manager not yet begun by osCreateViManager"},
    {75, "osSetTime: VI manager not yet begun by osCreateViManager"},
    {76, "osSetTimer: VI manager not yet begun by osCreateViManager"},
    {77, "osStopTimer: VI manager not yet begun by osCreateViManager"},
    {100, "_handleMIDIMsg: no sound mapped"},
    {101, "_handleMIDIMsg: no free voices"},
    {102, "_handleMIDIMsg: couldn't map voice"},
    {103, "_handleMIDIMsg: note off - couldn't find voice"},
    {104, "_handleMIDIMsg: poly pressure - couldn't find voice"},
    {105, "_handleEvent: no free voices"},
    {106, "Synthesizer: no free updates"},
    {107, "alSndPDeallocate: attempt to deallocate a sound which is playing"},
    {108, "alSndpDelete: attempt to delete player with playing sounds"},
    {109, "alSndpPlay: attempt to play a sound which is playing"},
    {110, "alSndpSetSound: sound id (%d) out of range (0 - %d)"},
    {111, "alSndpSetPriority: sound id (%d) out of range (0 - %d)"},
    {112, "alSndpSet Parameter: target (%d) out of range (0 - %d)"},
    {113, "alBnkfNew: bank file out of date"},
    {114, "alSeqNew: 0x%x is not a midi file"},
    {115, "alSeqNew: 0x%x is not a type 0 midi file"},
    {116, "alSeqNew: 0x%x has more than 1 track"},
    {117, "alSeqNew: SMPTE delta times not supported"},
    {118, "alSeqNew: Error parsing file 0x%x (no track header)"},
    {119, "alSeqNextEvent: Unsupported system exclusive"},
    {120, "alSeqNextEvent: Unsupported midi meta event 0x%x"},
    {121, "_handleMIDIMsg: Invalid program change to %d, max instruments %d"},
    {122, "_handleMIDIMsg: Unknown midi message 0x%x"},
    {123, "_unmapVoice: Couldn't unmap voice 0x%x"},
    {124, "alEvtqPostEvent: Out of free events"},
    {125, "alHeapAlloc: Can't allocate %d bytes"},
    {126, "alHeapCheck: Heap corrupt"},
    {127, "alHeapCheck: Heap corrupt - first block is bad"},
    {128, "alCSeqGetTrackEvent: Running status of zero on track %d"},
    {129, "alCSeqGetTrackEvent: Note on velocity of zero on track %d"},
    {130,
     "alCSPVoiceHandler: Stopping sequence but voice not free chan %d, key %d"},
    {131, "alSeqNextEvent: Read past end of sequence"},
    {132,
     "osAiSetNextBuffer: DMA buffer location may cause audio clicks (0x%x)"},
    {133,
     "_loadOutputBuffer: Modulated delay greater than total delay by %d "
     "samples"},
    {134, "osViExtendVStart: VI Manager not yet begun by osCreateViManager"},
    {135, "osViExtendVStart: value not in range [0-48] %d"},
    {136, "osThreadProfileStart: thread profiler is not initialized"},
    {137, "osThreadProfileStart: profiling has already been started"},
    {138, "osThreadProfileStop: thread profiler is not initialized"},
    {139, "osThreadProfileReadCount: thread profiler is not initialized"},
    {140, "osThreadProfileReadCountTh: thread profiler is not initialized"},
    {141, "osThreadProfileReadTime: thread profiler is not initialized"},
    {142, "osThreadProfileReadTimeTh: thread profiler is not initialized"},
    {143, "osThreadProfileReadCount: thread ID is too large(%d)"},
    {144, "osThreadProfileReadTime: thread ID is too large(%d)"},
    {145, "osThreadProfileReadCountTh: thread ID is too large(%d)"},
    {146, "osThreadProfileReadTimeTh: thread ID is too large(%d)"},
    {147, "osThreadProfileStop: current thread ID is too large(%d)"},

    // mark end of list
    {0, ""}};

static void errorhandler(s16 code, s16 numArgs, ...) {
  va_list argPtr;
  errorFormat* fmt = errorfmts;

  va_start(argPtr, numArgs);
  PRINTF("libultra error %d: ", code);
  while (fmt->code != 0) {
    if (fmt->code == code) {
      PRINTF(fmt->fmt, argPtr);
      break;
    }
    fmt++;
  }
  va_end(argPtr);
  PRINTF("\n");

  while (1) {
  }
}

void ed64RegisterOSErrorHandler() {
  // osSetErrorHandler(errorhandler);
  __osCommonHandler = errorhandler;
}

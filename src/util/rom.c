
#include <ultra64.h>
#include <nusys.h>
#include "rom.h"


//read data from the ROM in the specified range into RDRAM
void romCopy(const char *src, const char *dest, const int len)
{
    nuPiReadRom((u32)src, dest, len);
    //http://n64devkit.square7.ch/n64man/os/osEPiReadIo.htm
    // osEPiStartDma(gPiHandle, &dmaIoMesgBuf, OS_READ);
    // (void) osRecvMesg(&dmaMessageQ, &dummyMesg, OS_MESG_BLOCK);
}
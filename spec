/*
	ROM spec file

	Main memory map

  	0x80000000  exception vectors, ...
  	0x80000400  zbuffer (size 320*240*2)
  	0x80025c00  codesegment (must end by 0x80100000)
	      :       models
                collision
                audio data
                heap (512 * 1024)
    0x8030F800  Audio Heap
  	0x8038F800  cfb 16b 3buffer (size 320*240*2*3)
    0x80400000  expansion pack
                trace buffer
    0x8053E000  hi res cfb (size 640*480*2*3)
    0x80700000  hi res zbuffer (size 640*480*2)
    0x80796000  
*/

#include <nusys.h>

/* Use all graphic micro codes */
beginseg
	name	"code"
	flags	BOOT OBJECT
	entry 	nuBoot
	address NU_SPEC_BOOT_ADDR
        stack   NU_SPEC_BOOT_STACK

  /* maxsize 0xDA400  keep inside first mb of RDRAM */

	include "build/codesegment.o"
  include "$(ROOT)/usr/lib/PR/rspboot.o"   

  /*
  gfx microcodes should match the order of these defines:
  NU_GFX_UCODE_F3DEX2 0   // F3DEX microcode  
  NU_GFX_UCODE_F3DEX2_NON 1   // F3DEX.NoN microcode  
  NU_GFX_UCODE_F3DEX2_REJ 2   // F3DEX.ReJ microcode  
  NU_GFX_UCODE_F3DLX2_REJ 3   // F3DLX2.ReJ microcode  
  NU_GFX_UCODE_L3DEX2 4   // L3DEX microcode  
  NU_GFX_UCODE_S2DEX2 5   // S2DEX microcode  
  */

	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DEX2.Rej.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DLX2.Rej.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspS2DEX2.fifo.o"

  include "$(ROOT)/usr/lib/PR/n_aspMain.o"
  include "$(ROOT)/usr/lib/PR/aspMain.o"
endseg

beginseg
  name  "models"
  flags OBJECT
  after "code"
  include "build/src/models.o"
endseg

beginseg
  name  "sprites"
  flags OBJECT
  after "models"
  include "build/src/sprite_data.o"
endseg

beginseg
  name  "collision"
  flags OBJECT
  after "sprites"
  include "build/assets/levels/garden_map_collision.o"
endseg

beginseg
  name  "memheap"
  flags OBJECT
  after "collision"
  include "build/src/mem_heap.o"
endseg


/* sounds */
beginseg
  name "sounds"
  flags RAW
  include "build/assets/sounds/sounds.sounds"
endseg

/* sounds table */
beginseg
  name "soundsTbl"
  flags RAW
  include "build/assets/sounds/sounds.sounds.tbl"
endseg


beginseg
  name  "trace"
  flags OBJECT

  address 0x80400000
  include "build/src/trace.o"
endseg

beginwave
	name	"goose"
  include "code"
  include "models"
  include "sprites"
  include "collision" 
  include "memheap"
  include "sounds"
  include "soundsTbl"
  include "trace"
endwave

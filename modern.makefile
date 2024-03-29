# include for modern n64 toolchain

# expects the following environment variables to be defined
# ROOT="/etc/n64" (location of the n64 toolkit)

NUSYSINCDIR = $(ROOT)/usr/include/nusys
NUSYSLIBDIR = $(ROOT)/usr/lib/nusys
NUSTDINCDIR = $(ROOT)/usr/include/nustd
NUSTDLIBDIR = $(ROOT)/usr/lib/nustd

GCCLIBDIR = /opt/crashsdk/lib/gcc/mips64-elf/11.2.0
GCCINCDIR = /opt/crashsdk/lib/gcc/mips64-elf/11.2.0/include

INC = $(ROOT)/usr/include
LIB = $(ROOT)/usr/lib
EXEGCC_INC = $(ROOT)/GCC/MIPSE/INCLUDE

MAKEROM = tools/spicy/spicy
MAKEMASK = makemask

N64_CC ?= mips-n64-gcc
N64_CFLAGS ?= -c -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32 -D_LANGUAGE_C -D_ULTRA64 -D__EXTENSIONS__ -mabi=32  -march=vr4300 -mtune=vr4300 -mfix4300 

N64_AS ?= mips-n64-as
N64_ASFLAGS ?= -march=r4300 -mabi=32

N64_LD ?= mips-n64-ld
N64_LDFLAGS ?= --no-check-sections

N64_SIZE ?= mips-n64-size
N64_SIZEFLAGS ?=

N64_OBJCOPY ?= mips-n64-objcopy
N64_OBJCOPYFLAGS ?=

CC=$(N64_CC) $(N64_CFLAGS)
AS=$(N64_AS) $(N64_ASFLAGS)
LD=$(N64_LD) $(N64_LDFLAGS)
SIZE=$(N64_SIZE) $(N64_SIZEFLAGS)
OBJCOPY=$(N64_OBJCOPY) $(N64_OBJCOPYFLAGS)
NAWK=awk
# makefile for modern toolchain

# set up modern toolchain (gcc, etc)
include ./modern.makefile

LCINCS = -I. -I./include -I$(GCCINCDIR) -I$(NUSYSINCDIR) -I$(NUSTDINCDIR) -I$(ROOT)/usr/include/PR -I$(INC) -I$(EXEGCC_INC)
LCOPTS =	-G 0 -std=gnu90 -nostdinc -Wno-comment -Werror-implicit-function-declaration 

# include overrides file from local repo
ifndef RELEASE
# not using -include method because it doesn't work in old make version
ifneq ("$(wildcard localdefs.makefile)","")
$(info using localdefs.makefile)
include localdefs.makefile
else
$(info no localdefs.makefile)
endif
endif

# this improves CPU perf (but we're usually RDP-bound)
# OPTIMIZE = y 
ifdef RELEASE
ED64 =
OPTIMIZE = y 
endif

ifdef OPTIMIZE
NUAUDIOLIB = -lnualstl_n -ln_mus -ln_audio_sc
else
NUAUDIOLIB = -lnualstl_n_d -ln_mus_d -ln_audio_sc
endif

LCDEFS = -DN_AUDIO -DF3DEX_GBI_2 -D__N64__

ifndef OPTIMIZE
LCDEFS += -DNU_DEBUG -DDEBUG
endif

ifdef RELEASE
LCDEFS += -DRELEASE
endif

ifdef ED64
LCDEFS += -DED64
endif

ifdef OPTIMIZE
CORELIBS = -lnusys -lnustd -lultra 
else
CORELIBS = -lnusys_d -lnustd_d -lultra_d 
endif

ifdef OPTIMIZE
OPTIMIZER =	-O2
else
OPTIMIZER =	-g
endif

BUILDDIR = build

APP =		$(BUILDDIR)/goose64.out

TARGETS =	$(BUILDDIR)/goose64.n64

SPECFILE = spec

HFILES =	src/main.h src/graphic.h build/assets/models/testingCube.h src/vec3d.h src/vec2d.h src/gameobject.h src/game.h src/modeltype.h src/renderer.h src/input.h src/character.h src/player.h src/gameutils.h src/gametypes.h src/item.h src/animation.h src/physics.h src/rotation.h src/collision.h src/garden_map_collision.h src/pathfinding.h src/trace.h src/frustum.h src/garden_map_graph.h

MODEL_OBJS = $(wildcard assets/models/**/*.obj) $(wildcard assets/models/*.obj)
SPRITE_IMGS = $(wildcard assets/sprites/*.png) $(wildcard assets/sprites/**/*.png) $(wildcard assets/sprites/*.bmp) $(wildcard assets/sprites/**/*.bmp) 

MODEL_HEADERS = $(MODEL_OBJS:assets/models/%.obj=$(BUILDDIR)/assets/models/%.h)
SPRITE_HEADERS = $(patsubst assets/sprites/%.png,$(BUILDDIR)/assets/sprites/%.h,$(patsubst assets/sprites/%.bmp,$(BUILDDIR)/assets/sprites/%.h,$(SPRITE_IMGS)))

LEVELS = 

ED64CODEFILES = src/ed64/ed64io_usb.c src/ed64/ed64io_sys.c src/ed64/ed64io_everdrive.c src/ed64/ed64io_fault.c src/ed64/ed64io_os_error.c src/ed64/ed64io_watchdog.c

CODEFILES   = 	src/main.c src/stage00.c src/graphic.c src/gfxinit.c src/vec3d.c src/vec2d.c src/gameobject.c src/game.c src/modeltype.c src/renderer.c src/input.c src/character.c src/characterstate.c src/player.c src/gameutils.c src/item.c src/animation.c src/physics.c src/rotation.c src/collision.c  src/pathfinding.c src/frustum.c  src/garden_map_graph.c src/sprite.c

ifdef ED64
CODEFILES  += $(ED64CODEFILES)
endif

CODEOBJECTS =	$(CODEFILES:src/%.c=$(BUILDDIR)/src/%.o)  $(NUSYSLIBDIR)/nusys.o

DATAFILES   = src/mem_heap.c src/trace.c src/garden_map_collision.c src/models.c src/sprite_data.c
DATAOBJECTS =	$(DATAFILES:src/%.c=$(BUILDDIR)/src/%.o)

CODESEGMENT =	$(BUILDDIR)/codesegment.o


OBJECTS =	$(CODESEGMENT) $(MODELSSEGMENT) $(DATAOBJECTS)

CFLAGS = $(LCDEFS) $(LCINCS) $(LCOPTS) $(OPTIMIZER) 

# the order of $(NUAUDIOLIB) and -lgultra_d (CORELIBS) matter :|
LDFLAGS = $(MKDEPOPT) -L$(LIB)  -L$(NUSYSLIBDIR) -L$(NUSTDLIBDIR) $(NUAUDIOLIB) $(CORELIBS) -L$(GCCLIBDIR) -lgcc


default: $(TARGETS)


# $(MODEL_OBJS):
# 	# empty rule for object files

$(BUILDDIR)/src/%.o: src/%.c | $(BUILDDIR) $(HFILES)
# to print resolved include paths, add -M flag
	$(CC) $(CFLAGS) -o $@ $<

$(BUILDDIR)/assets/models/%.h: assets/models/%.obj | $(BUILDDIR)
	lua tools/wavefront64/wavefront64.lua obj $< $@

$(BUILDDIR)/assets/sprites/%.h: assets/sprites/%.png | $(BUILDDIR)
	python3 tools/ultratex.py $< $@

$(BUILDDIR)/assets/sprites/%.h: assets/sprites/%.bmp | $(BUILDDIR)
	python3 tools/ultratex.py $< $@
	
$(BUILDDIR):
	mkdir -p $@
	rsync -a --exclude='*.*' assets/ build/assets
	# cp -pR assets build/assets
	cp -pR src build/src

clean: 
	rm -f $(BUILDDIR)/*.o src/*.o *.o $(BUILDDIR)/*.out src/*.out *.out 
	rm -r -f $(BUILDDIR)

clobber:
	rm -f src/*.o *.o *.n64 *.out
	rm -r -f $(BUILDDIR)

$(CODESEGMENT): $(CODEOBJECTS) Makefile $(HFILES) $(MODEL_HEADERS) $(SPRITE_HEADERS)
# use -M to print memory map from ld
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS) 

$(TARGETS):	$(OBJECTS) $(SPECFILE) $(CODESEGMENT)
	$(MAKEROM) -I$(NUSYSINCDIR) -r $(TARGETS) -s 16 -e $(APP)  --ld_command=mips-n64-ld --as_command=mips-n64-as --cpp_command=mips-n64-gcc --objcopy_command=mips-n64-objcopy  $(SPECFILE) # --verbose=true  --verbose_linking=true 
	makemask $(TARGETS)


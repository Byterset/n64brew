# makefile for modern toolchain

# set up modern toolchain (gcc, etc)
include ./modern.makefile

BLENDER= blender
EMULATOR=/mnt/c/Users/kevin/Documents/Emulation/N64/Mupen64plus/mupen64plus/simple64-gui.exe

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

ROMHEADER = romheader

APP =		$(BUILDDIR)/game.out

TARGETS =	$(BUILDDIR)/game.z64

SPECFILE = spec

HFILES =	src/main.h src/graphics/graphic.h src/math/vec3d.h src/math/vec2d.h src/gameobject.h src/game.h src/modeltype.h src/graphics/renderer.h src/input.h src/character.h src/player.h src/gameutils.h src/gametypes.h src/item.h src/animation/animation.h src/physics/physics.h src/math/rotation.h src/physics/collision.h assets/levels/garden_map_collision.h src/pathfinding.h src/trace.h src/math/frustum.h src/garden_map_graph.h src/segments.h
HFILES += 	src/math/vector2.h src/math/vector3.h src/math/quaternion.h src/math/vector4.h src/math/vector2s16.h src/util/rom.h 
HFILES += src/audio/audio.h src/audio/soundplayer.h src/audio/soundarray.h
HFILES += src/util/memory.h src/util/time.h src/graphics/color.h src/graphics/renderstate.h src/defs.h
LEVELS = $(wildcard assets/levels/**/**/*.blend) $(wildcard assets/levels/**/*.blend) $(wildcard assets/levels/*.blend)
LEVEL_MAP_HEADERS = $(LEVELS:%.blend=%_map.h)
LEVEL_MAP_COLLISION_HEADERS = $(LEVELS:%.blend=%_map_collision.h)
LEVEL_MAP_COLLISION_C_FILES = $(LEVEL_MAP_COLLISION_HEADERS:%.h=%.c)
LEVELS_DATA = $(LEVEL_MAP_HEADERS) $(LEVEL_MAP_COLLISION_C_FILES) $(LEVEL_MAP_COLLISION_HEADERS)

MODEL_OBJS = $(wildcard assets/models/**/**/*.obj) $(wildcard assets/models/**/*.obj) $(wildcard assets/models/*.obj)
SPRITE_IMGS = $(wildcard assets/sprites/*.png) $(wildcard assets/sprites/**/*.png) $(wildcard assets/sprites/*.bmp) $(wildcard assets/sprites/**/*.bmp) 

MODEL_HEADERS = $(MODEL_OBJS:assets/models/%.obj=$(BUILDDIR)/assets/models/%.h)
SPRITE_HEADERS = $(patsubst assets/sprites/%.png,$(BUILDDIR)/assets/sprites/%.h,$(patsubst assets/sprites/%.bmp,$(BUILDDIR)/assets/sprites/%.h,$(SPRITE_IMGS)))


ED64CODEFILES = src/ed64/ed64io_usb.c src/ed64/ed64io_sys.c src/ed64/ed64io_everdrive.c src/ed64/ed64io_fault.c src/ed64/ed64io_os_error.c src/ed64/ed64io_watchdog.c

CODEFILES = 	src/main.c src/stage00.c src/graphics/graphic.c src/graphics/gfxinit.c src/math/vec3d.c src/math/vec2d.c src/gameobject.c src/game.c src/modeltype.c src/graphics/renderer.c src/input.c src/character.c src/characterstate.c src/player.c src/gameutils.c src/item.c src/animation/animation.c src/physics/physics.c src/math/rotation.c src/physics/collision.c  src/pathfinding.c src/math/frustum.c  src/garden_map_graph.c src/sprite.c
CODEFILES +=	src/math/mathf.c src/math/vector2.c src/math/vector3.c src/math/vector2s16.c src/math/vector4.c src/util/rom.c src/math/quaternion.c
CODEFILES += src/audio/audiomgr.c src/audio/audio.c src/audio/soundarray.c src/audio/soundplayer.c
CODEFILES += src/util/memory.c src/util/time.c src/graphics/color.c src/graphics/renderstate.c 
ifdef ED64
CODEFILES  += $(ED64CODEFILES)
endif

CODEOBJECTS =	$(CODEFILES:src/%.c=$(BUILDDIR)/src/%.o)  $(NUSYSLIBDIR)/nusys.o

DATA_SRC_FILES   = src/mem_heap.c src/trace.c src/models.c src/sprite_data.c
# DATA_SRC_FILES += src/aud_heap.c
DATA_SRC_ASSETS = assets/levels/garden_map_collision.c
DATAOBJECTS =	$(DATA_SRC_FILES:src/%.c=$(BUILDDIR)/src/%.o) $(DATA_SRC_ASSETS:assets/%.c=$(BUILDDIR)/assets/%.o)

CODESEGMENT =	$(BUILDDIR)/codesegment.o

MUSIC_CLIPS = $(shell find assets/music/ -type f -name '*.wav')
SOUND_CLIPS = $(shell find assets/sounds/ -type f -name '*.wav') #$(shell find assets/sounds -type f -name '*.aif') $(shell find assets/sounds_ins -type f -name '*.ins')

ALL_SOUND_WAV = $(MUSIC_CLIPS) $(SOUND_CLIPS)


OBJECTS =	$(CODESEGMENT) $(MODELSSEGMENT) $(DATAOBJECTS)

CFLAGS = $(LCDEFS) $(LCINCS) $(LCOPTS) $(OPTIMIZER) 

# the order of $(NUAUDIOLIB) and -lgultra_d (CORELIBS) matter :|
LDFLAGS = $(MKDEPOPT) -L$(LIB)  -L$(NUSYSLIBDIR) -L$(NUSTDLIBDIR) $(NUAUDIOLIB) $(CORELIBS) -L$(GCCLIBDIR) -lgcc


default: $(TARGETS)

# $(MODEL_OBJS):
# 	# empty rule for object files

$(BUILDDIR)/src/%.o: src/%.c | $(BUILDDIR) $(HFILES) build/src/audio/clips.h
# to print resolved include paths, add -M flag
	$(CC) $(CFLAGS) -o $@ $<

$(BUILDDIR)/assets/%.o: assets/%.c | $(BUILDDIR) $(HFILES)
# to print resolved include paths, add -M flag
	$(CC) $(CFLAGS) -o $@ $<



SFZ2N64 = tools/sfz2n64
build/assets/sounds/sounds.sounds build/assets/sounds/sounds.sounds.tbl: $(ALL_SOUND_WAV)
	@mkdir -p $(@D)
	$(SFZ2N64) --compress -o $@ $^

build/src/audio/clips.h: tools/generate_sound_ids.js $(ALL_SOUND_WAV)
	@mkdir -p $(@D)
	node tools/generate_sound_ids.js -o $@ -p SOUNDS_ $(ALL_SOUND_WAV)

build/src/stage00.o: build/src/audio/clips.h
assets/levels/%_map.h: assets/levels/%.blend 
	$(BLENDER) -b $< -P tools/export_positions.py

assets/levels/%_map_collision.h assets/levels/%_map_collision.c: assets/levels/%.blend 
	$(BLENDER) -b $< -P tools/export_collision_mesh.py


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

$(CODESEGMENT): $(CODEOBJECTS) Makefile $(HFILES) $(MODEL_HEADERS) $(SPRITE_HEADERS) $(LEVELS_DATA)
# use -M to print memory map from ld
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS) 

$(TARGETS):	$(OBJECTS) $(SPECFILE) $(CODESEGMENT) build/assets/sounds/sounds.sounds
	$(MAKEROM) -I$(NUSYSINCDIR) -r $(TARGETS) -s 0 -e $(APP) -h $(ROMHEADER)  --ld_command=mips-n64-ld --as_command=mips-n64-as --cpp_command=mips-n64-gcc --objcopy_command=mips-n64-objcopy  $(SPECFILE) # --verbose=true  --verbose_linking=true
	makemask $(TARGETS)
#	$(EMULATOR) $(TARGETS)
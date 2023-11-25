# makefile for modern toolchain

# set up modern toolchain (gcc, etc)
include ./modern.makefile

BLENDER= blender

SFZ2N64 = tools/sfz2n64


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

ROMHEADER = romheader_ntsc

APP =		$(BUILDDIR)/game.out

TARGETS =	$(BUILDDIR)/game.z64

SPECFILE = spec

HFILES =	src/main.h src/gameobject.h src/game.h src/modeltype.h src/graphics/renderer.h src/controls/input.h src/actors/gardener/character.h src/player.h src/gameutils.h src/gametypes.h src/item.h src/animation/animation.h src/physics/physics.h src/math/rotation.h src/physics/collision.h src/pathfinding/pathfinding.h src/util/trace.h src/math/frustum.h src/segments.h
HFILES += 	src/math/vector2.h src/math/vector3.h src/math/quaternion.h src/math/vector4.h src/math/vector2s16.h src/util/rom.h src/constants.h
HFILES += 	src/audio/audio.h src/audio/soundplayer.h src/audio/soundarray.h
HFILES += 	src/util/memory.h src/util/time.h src/graphics/color.h src/graphics/renderstate.h src/defs.h
HFILES += 	src/graphics/graphics.h src/graphics/initgfx.h src/controls/controller.h src/math/matrix.h src/font/font.h src/font/font_ext.h src/font/letters_img.h src/util/debug_console.h
HFILES +=	src/sausage64/sausage64.h
HFILES += 	assets/levels/garden/garden_map_graph.h assets/levels/garden/garden_map_collision.h

LEVELS = $(wildcard assets/levels/**/**/*.blend) $(wildcard assets/levels/**/*.blend) $(wildcard assets/levels/*.blend)
LEVEL_MAP_HEADERS = $(LEVELS:%.blend=%_map.h)
LEVEL_MAP_COLLISION_HEADERS = $(LEVELS:%.blend=%_map_collision.h)
LEVEL_MAP_COLLISION_C_FILES = $(LEVEL_MAP_COLLISION_HEADERS:%.h=%.c)
LEVELS_DATA = $(LEVEL_MAP_HEADERS) $(LEVEL_MAP_COLLISION_C_FILES) $(LEVEL_MAP_COLLISION_HEADERS)

MODEL_OBJS = $(wildcard assets/models/**/**/*.obj) $(wildcard assets/models/**/*.obj) $(wildcard assets/models/*.obj)
SPRITE_IMGS = $(wildcard assets/sprites/*.png) $(wildcard assets/sprites/**/*.png) $(wildcard assets/sprites/*.bmp) $(wildcard assets/sprites/**/*.bmp) 

MODEL_HEADERS = $(MODEL_OBJS:assets/models/%.obj=assets/models/%.h)
SPRITE_HEADERS = $(patsubst assets/sprites/%.png,assets/sprites/%.h,$(patsubst assets/sprites/%.bmp,assets/sprites/%.h,$(SPRITE_IMGS)))

# Code Files regarding Everdrive
ED64CODEFILES = src/ed64/ed64io_usb.c src/ed64/ed64io_sys.c src/ed64/ed64io_everdrive.c src/ed64/ed64io_fault.c src/ed64/ed64io_os_error.c src/ed64/ed64io_watchdog.c

# Code Files
CODEFILES =  src/main.c src/stage00.c src/gameobject.c src/game.c src/modeltype.c src/graphics/renderer.c src/controls/input.c src/actors/gardener/character.c 
CODEFILES += src/actors/gardener/characterstate.c src/player.c src/gameutils.c src/item.c src/animation/animation.c src/physics/physics.c src/math/rotation.c 
CODEFILES += src/physics/collision.c  src/pathfinding/pathfinding.c src/math/frustum.c src/sprite.c
CODEFILES += src/math/mathf.c src/math/vector2.c src/math/vector3.c src/math/vector2s16.c src/math/vector4.c src/util/rom.c src/math/quaternion.c
CODEFILES += src/audio/audiomgr.c src/audio/audio.c src/audio/soundarray.c src/audio/soundplayer.c
CODEFILES += src/util/memory.c src/util/time.c src/graphics/color.c src/graphics/renderstate.c 
CODEFILES += src/graphics/graphics.c src/graphics/initgfx.c src/controls/controller.c src/math/matrix.c src/font/font.c src/util/debug_console.c
CODEFILES += src/sausage64/sausage64.c
CODEFILES += assets/levels/garden/garden_map_graph.c
ifdef ED64
CODEFILES  += $(ED64CODEFILES)
endif

CODEOBJECTS =	$(CODEFILES:%.c=$(BUILDDIR)/%.o) $(NUSYSLIBDIR)/nusys.o

DATA_SRC_FILES   = src/util/mem_heap.c src/util/trace.c src/models.c src/sprite_data.c
DATA_SRC_ASSETS = assets/levels/garden/garden_map_collision.c
DATAOBJECTS =	$(DATA_SRC_FILES:src/%.c=$(BUILDDIR)/src/%.o) $(DATA_SRC_ASSETS:assets/%.c=$(BUILDDIR)/assets/%.o)
CODESEGMENT =	$(BUILDDIR)/codesegment.o

MUSIC_CLIPS = $(shell find assets/music/ -type f -name '*.wav')
SOUND_CLIPS = $(shell find assets/sounds/ -type f -name '*.wav') #$(shell find assets/sounds -type f -name '*.aif') $(shell find assets/sounds_ins -type f -name '*.ins')
SOUNDS_FILES = build/assets/sounds/sounds.sounds
ALL_SOUND_WAV = $(MUSIC_CLIPS) $(SOUND_CLIPS)

# All necessary .o Files for the build
OBJECTS =	$(CODESEGMENT) $(MODELSSEGMENT) $(DATAOBJECTS)

CFLAGS = $(LCDEFS) $(LCINCS) $(LCOPTS) $(OPTIMIZER) 

# the order of $(NUAUDIOLIB) and -lgultra_d (CORELIBS) matter :|
LDFLAGS = $(MKDEPOPT) -L$(LIB)  -L$(NUSYSLIBDIR) -L$(NUSTDLIBDIR) $(NUAUDIOLIB) $(CORELIBS) -L$(GCCLIBDIR) -lgcc

default: $(TARGETS)

$(BUILDDIR)/src/%.o: src/%.c $(HFILES) | $(BUILDDIR) src/audio/clips.h
# to print resolved include paths, add -M flag
	$(CC) $(CFLAGS) -o $@ $<

$(BUILDDIR)/assets/%.o: assets/%.c | $(BUILDDIR) $(HFILES)
# to print resolved include paths, add -M flag
	$(CC) $(CFLAGS) -o $@ $<

# build the sounds file and the sounds table, both will be included as raw data in the ROM and referenced via the sound player
build/assets/sounds/%.sounds build/assets/sounds/%.sounds.tbl: $(ALL_SOUND_WAV)
	@mkdir -p $(@D)
	$(SFZ2N64) --compress -o $@ $^

#generate the header containing the sound/song ids for use with the sound player
src/audio/clips.h: tools/generate_sound_ids.js $(ALL_SOUND_WAV)
	@mkdir -p $(@D)
	node tools/generate_sound_ids.js -o $@ -p SOUNDS_ $(ALL_SOUND_WAV)

#create the level data for a level in .blend format (AABBs, Object Transforms, etc)
assets/levels/%_map.h: assets/levels/%.blend 
	$(BLENDER) -b $< -P tools/export_level.py

#create the collision data for a level in .blend format (Collision Meshes, Spatial Hash Buckets, etc)
assets/levels/%_map_collision.h assets/levels/%_map_collision.c: assets/levels/%.blend 
	$(BLENDER) -b $< -P tools/export_collision_mesh.py

#convert from wavefront .obj to n64 compatible header
assets/models/%.h: assets/models/%.obj
	lua tools/wavefront64/wavefront64.lua obj $< $@

#convert from PNG to n64 compatible header
assets/sprites/%.h: assets/sprites/%.png
	python3 tools/ultratex.py $< $@

#convert from Bitmap to n64 compatible header
assets/sprites/%.h: assets/sprites/%.bmp
	python3 tools/ultratex.py $< $@
	
#create the build directory and mirror the assets and src directory structure inside
$(BUILDDIR):
	mkdir -p $@
	rsync -a --exclude='*.*' assets/ build/assets
	rsync -a --exclude='*.*' src/ build/src

clean: 
	rm -f $(BUILDDIR)/*.o src/*.o *.o $(BUILDDIR)/*.out src/*.out *.out 
	rm -r -f $(BUILDDIR)

clobber:
	rm -f src/*.o *.o *.n64 *.out
	rm -r -f $(BUILDDIR)

# the object containing the linked code objects
$(CODESEGMENT): $(CODEOBJECTS) $(HFILES) $(MODEL_HEADERS) $(SPRITE_HEADERS) $(LEVELS_DATA)
# use -M to print memory map from ld
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS) 

# building the target rom
$(TARGETS):	$(OBJECTS) $(SPECFILE) $(SOUNDS_FILES)
	$(MAKEROM) -I$(NUSYSINCDIR) -r $(TARGETS) -s 0 -e $(APP) -h $(ROMHEADER)  --ld_command=mips-n64-ld --as_command=mips-n64-as --cpp_command=mips-n64-gcc --objcopy_command=mips-n64-objcopy  $(SPECFILE) # --verbose=true  --verbose_linking=true
	$(MAKEMASK) $(TARGETS)
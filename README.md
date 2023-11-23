# n64brew

This ist based off of https://github.com/jsdf/goose64. This aims to transform the work done by jsdf into a general purpose 3d engine for the n64.

## what has been done so far

- build process was mostly automized and need for manual export of meshes, collision or levels has been removed
- opengl build was removed
- makefile was redone
- various python scripts and tools have been adapted
- completely overhauled the structure of the project to scale better
- changed audio player to support wav, aiff & ins (done by migrating to the existing solution by the amazing https://github.com/lambertjamesd/)
- added modified version of https://github.com/trhodeos/spicy to tools to support custom rom headers (romheaders for PAL & NTSC included in the root)
- implemented sausage64 Sausage Link library (https://github.com/buu342/N64-Sausage64) to make general animated models easy to import and use

## TODO

- improve model handling, right now I feel it is too much manual work to define the model properties
- support more collider shapes
- optimize collision solver (right now there is some funky behaviour with the collision solver)
- switch from obj to fbx file format (likely to go hand in hand with the skeletal animation update)
- remove old animation code and replace with sausage64 instances

## build for emulator/console

### dependencies for building

- `crashsdk` (see below)
- `sfz2n64` (https://github.com/lambertjamesd/sfz2n64)
- `blender` (level creation, model creation, character animation)
- `python` (animation export, collision export, level export) via `blender`
- `pillow` pip install pillow (if textures are changed)
- `wavefront64` (https://github.com/tffdev/Wavefront64) Lua script that converts textured .obj Files and .bmp Sprites to N64 compatible .h Header Files. **lua script and dependencies are included under ./tools/wavefront64**
- `spicy` (https://github.com/Byterset/spicy) forked from (https://github.com/trhodeos) open source replacement for nintendos `mild.exe` **binary is included under ./tools/spicy**

The version of `spicy` contained in this repo is a custom Build that allows the use of custom ROM Headers. If you want to use the original build of `spicy` or even the original `mild` binary you should be able to do so by changing the **MAKEROM** variable in the modern.makefile

### Installing the n64 sdk

I am using the modern n64 homebrew sdk (crashsdk) and have based the build process around that.

macOS/linux instructions: https://crashoveride95.github.io/n64hbrew/modernsdk/startoff.html

It is important that the instructions on the crashsdk site are followed and all necessary tools are installed.

### Building the game

All commands should be run in the `root` directory of this repo

Make sure your Makefile and modern.makefile are set up correctly to reflect the location of the sdk on your machine and the names of the compiler, linker etc binaries.

run `make` to build 

run `make clean` or `make clobber` to clean up the build directory and any build-byproducts

this produces the rom file `game.z64` (configurable via Makefile) which you can then run with your favorite emulator or flashcart


environment variables which affect the build:

- `OPTIMIZE=1`: enable gcc optimization and use non-debug versions of sdk libraries
- `ED64=1`: build with everdrive64 logging support (see [ed64log](https://github.com/jsdf/ed64log)). don't use unless running on an everdrive64

you can also create a file called `localdefs.makefile` containing any variables to override in the build, and it will be automatically included by the makefile.


## regenerate n64 header files for sprites
if you update any texture files:

make sure you have python and pillow installed

```bash
pip install pillow
```

then, to rebuild sprites

```bash
./tools/sh/sprites.sh 
```

## export map object data

- open blender (or use `./blender.sh` to see console output)
- in the blender text editor editor, open and run export_positions.py then open and run export_collision_mesh.py
- see header files are created
- this script is run automatically when running `make`. So after updating a .blend file the headers should be rebuilt automatically. (given the .blend file for the level is in `./assets/levels/*`)


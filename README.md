# Moon Child Ports

Miscellaneous ports of Moon Child to different targets with a modular platform system.

The N64 target is currently unfinished and may not work correctly/work at all.

Original repository + README: [Bitbucket link](https://bitbucket.org/rhinoid/moonchild/src/main/)

# Building

## Wii

[Install devkitPro](https://github.com/devkitPro/installer/releases) and make sure your environment variables are set up correctly

Clone the repo anywhere and run
```
make PLATFORM=WII
```
to build the .dol

Copy the ``assets`` folder to the root of your SD card (should work on Dolphin with SD card emulation, too) and rename it to ``moonchild_assets``

Run the .dol with wiiload (``make -f Makefile_wii.mk run``) or drag it into Dolphin

## N64

[Install MSYS2](https://www.msys2.org/) and [build/install libdragon](https://github.com/DragonMinded/libdragon/wiki/Installing-libdragon) (specifically the preview branch) to it

Open a UCRT64 terminal and clone the repo anywhere

Run
```
make PLATFORM=N64
```
to build the .z64

Use a flashcart or the [Ares emulator](https://ares-emu.net/) to run and debug it

## SDL

TODO!! (should be nearly the same as the original repo except without zlib)
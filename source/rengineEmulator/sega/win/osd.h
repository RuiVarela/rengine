
#ifndef _OSD_H_
#define _OSD_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <conio.h>
#include <stdlib.h>

#include "shared.h"
#include "config.h"
#include "error.h"


extern "C"
{
	void error(char *format, ...);

	unsigned long crc32(unsigned long crc, unsigned char* data, unsigned int size);
	void osd_input_Update();
	int load_archive(char* const filename);

} // extern c

#define GG_ROM    "data\\emulator\\roms\\Game_Genie_(Unl)_(Jun 1992)_[c][!].bin"
#define AR_ROM    "data\\emulator\\roms\\Pro_Action_Replay_(JUE)_[c][!].bin"
#define OS_ROM    "Genesis_OS_ROM.bin"
#define SK_ROM    "data\\emulator\\roms\\Sonic_and_Knuckles_(JUE)_[!].bin"
#define SK_UPMEM  "data\\emulator\\roms\\Sonic_and_Knuckles_upmem.bin"

#define GENESIS_DATABASE		"data\\emulator\\roms\\genesis.zip"
#define MASTERSYSTEM_DATABASE	"data\\emulator\\roms\\mastersystem.zip"
#define SG1000_DATABASE			"data\\emulator\\roms\\sg1000.zip"
#define GAMEGEAR_DATABASE		"data\\emulator\\roms\\gamegear.zip"

#define DEFAULT_SEGA_SAVE_PATH	"data/emulator/saves/sega/"
#define DEFAULT_SEGA_SCRIPT		"data/emulator/sega_startup.rss"
#define DEFAULT_SEGA_EFFECT		"data/emulator/sega.eff"

#endif 

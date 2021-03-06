#ifndef _SHARED_H_
#define _SHARED_H_

//
// Rui Varela 11.12.11
// ->
//
#ifndef M_PI
	#define M_PI           3.14159265358979323846
#endif


#include <stdio.h>
#include <math.h>
#include <zlib.h>

#include "types.h"
#include "macros.h"
#include "m68k.h"
#include "z80.h"
#include "system.h"
#include "genesis.h"
#include "vdp_ctrl.h"
#include "vdp_render.h"
#include "mem68k.h"
#include "memz80.h"
#include "membnk.h"
#include "io_ctrl.h"
#include "input.h"
#include "state.h"
#include "sound.h"
#include "sn76489.h"
#include "ym2413.h"
#include "ym2612.h"
#include "loadrom.h"
#include "sms_cart.h"
#include "md_cart.h"
#include "md_eeprom.h"
#include "gg_eeprom.h"
#include "sram.h"
#include "ggenie.h"
#include "areplay.h"
#include "svp.h"
#include "osd.h"

#endif /* _SHARED_H_ */


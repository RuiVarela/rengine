/***************************************************************************************
 *  Genesis Plus
 *  Video Display Processor (68k & Z80 CPU interface)
 *
 *  Support for SG-1000, Master System (315-5124 & 315-5246), Game Gear & Mega Drive VDP
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2011  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "hvc.h"

/* Mark a pattern as dirty */
#define MARK_BG_DIRTY(addr)                         \
{                                                   \
  name = (addr >> 5) & 0x7FF;                       \
  if(bg_name_dirty[name] == 0)                      \
  {                                                 \
    bg_name_list[bg_list_index++] = name;           \
  }                                                 \
  bg_name_dirty[name] |= (1 << ((addr >> 2) & 7));  \
}

/* VDP context */
uint8 sat[0x400];     /* Internal copy of sprite attribute table */
uint8 vram[0x10000];  /* Video RAM (64K x 8-bit) */
uint8 cram[0x80];     /* On-chip color RAM (64 x 9-bit) */
uint8 vsram[0x80];    /* On-chip vertical scroll RAM (40 x 11-bit) */
uint8 reg[0x20];      /* Internal VDP registers (23 x 8-bit) */
uint8 hint_pending;   /* 0= Line interrupt is pending */
uint8 vint_pending;   /* 1= Frame interrupt is pending */
uint16 status;        /* VDP status flags */
uint32 dma_length;    /* DMA remaining length */

/* Global variables */
uint16 ntab;                      /* Name table A base address */
uint16 ntbb;                      /* Name table B base address */
uint16 ntwb;                      /* Name table W base address */
uint16 satb;                      /* Sprite attribute table base address */
uint16 hscb;                      /* Horizontal scroll table base address */
uint8 bg_name_dirty[0x800];       /* 1= This pattern is dirty */
uint16 bg_name_list[0x800];       /* List of modified pattern indices */
uint16 bg_list_index;             /* # of modified patterns in list */
uint8 bg_pattern_cache[0x80000];  /* Cached and flipped patterns */
uint8 hscroll_mask;               /* Horizontal Scrolling line mask */
uint8 playfield_shift;            /* Width of planes A, B (in bits) */
uint8 playfield_col_mask;         /* Playfield column mask */
uint16 playfield_row_mask;        /* Playfield row mask */
uint16 vscroll;                   /* Latched vertical scroll value */
uint8 odd_frame;                  /* 1: odd field, 0: even field */
uint8 im2_flag;                   /* 1= Interlace mode 2 is being used */
uint8 interlaced;                 /* 1: Interlaced mode 1 or 2 */
uint8 vdp_pal;                    /* 1: PAL , 0: NTSC (default) */
uint16 v_counter;                 /* Vertical counter */
uint16 vc_max;                    /* Vertical counter overflow value */
uint16 lines_per_frame;           /* PAL: 313 lines, NTSC: 262 lines */
int32 fifo_write_cnt;             /* VDP writes fifo count */
uint32 fifo_lastwrite;            /* last VDP write cycle */
uint32 hvc_latch;                 /* latched HV counter */
const uint8 *hctab;               /* pointer to H Counter table */

/* Function pointers */
void (*vdp_68k_data_w)(unsigned int data);
void (*vdp_z80_data_w)(unsigned int data);
unsigned int (*vdp_68k_data_r)(void);
unsigned int (*vdp_z80_data_r)(void);

/* Tables that define the playfield layout */
static const uint8 hscroll_mask_table[] = { 0x00, 0x07, 0xF8, 0xFF };
static const uint8 shift_table[]        = { 6, 7, 0, 8 };
static const uint8 col_mask_table[]     = { 0x0F, 0x1F, 0x0F, 0x3F };
static const uint16 row_mask_table[]    = { 0x0FF, 0x1FF, 0x2FF, 0x3FF };

static uint8 border;          /* Border color index */
static uint8 pending;         /* Pending write flag */
static uint8 code;            /* Code register */
static uint8 dma_type;        /* DMA mode */
static uint16 dmafill;        /* DMA Fill setup */
static uint16 addr;           /* Address register */
static uint16 addr_latch;     /* Latched A15, A14 of address */
static uint16 sat_base_mask;  /* Base bits of SAT */
static uint16 sat_addr_mask;  /* Index bits of SAT */
static uint32 dma_endCycles;  /* 68k cycles to DMA end */
static uint32 fifo_latency;   /* CPU access latency */
static int cached_write;      /* 2nd part of 32-bit CTRL port write (Genesis mode) or LSB of CRAM data (Game Gear mode) */
static uint16 fifo[4];        /* FIFO buffer */

 /* set Z80 or 68k interrupt lines */
static void (*set_irq_line)(unsigned int level);
static void (*set_irq_line_delay)(unsigned int level);

/* DMA Timings */
static const uint8 dma_timing[2][2] =
{
/* H32, H40 */
  {16 , 18},  /* active display */
  {167, 205}  /* blank display */
};

/* Vertical counter overflow values (see hvc.h) */
static const uint16 vc_table[4][2] = 
{
  /* NTSC, PAL */
  {0xDA , 0xF2},  /* Mode 4 (192 lines) */
  {0xEA , 0x102}, /* Mode 5 (224 lines) */
  {0xDA , 0xF2},  /* Mode 4 (192 lines) */
  {0x106, 0x10A}  /* Mode 5 (240 lines) */
};


/*--------------------------------------------------------------------------*/
/* Function prototypes                                                      */
/*--------------------------------------------------------------------------*/

static void vdp_68k_data_w_m4(unsigned int data);
static void vdp_68k_data_w_m5(unsigned int data);
static unsigned int vdp_68k_data_r_m4(void);
static unsigned int vdp_68k_data_r_m5(void);
static void vdp_z80_data_w_m4(unsigned int data);
static void vdp_z80_data_w_m5(unsigned int data);
static unsigned int vdp_z80_data_r_m4(void);
static unsigned int vdp_z80_data_r_m5(void);
static void vdp_z80_data_w_ms(unsigned int data);
static void vdp_z80_data_w_gg(unsigned int data);
static void vdp_z80_data_w_sg(unsigned int data);
static void vdp_bus_w(unsigned int data);
static void vdp_fifo_update(unsigned int cycles);
static void vdp_reg_w(unsigned int r, unsigned int d, unsigned int cycles);
static void vdp_dma_copy(int length);
static void vdp_dma_vbus(int length);
static void vdp_dma_fill(unsigned int data, int length);


/*--------------------------------------------------------------------------*/
/* Init, reset, context functions                                           */
/*--------------------------------------------------------------------------*/

void vdp_init(void)
{
  /* PAL/NTSC timings */
  lines_per_frame = vdp_pal ? 313: 262;

  /* CPU interrupt line(s)*/
  switch (system_hw)
  {
    case SYSTEM_MD:
    case SYSTEM_PICO:
    {
      /* 68k cpu */
      set_irq_line = m68k_set_irq;
      set_irq_line_delay = m68k_set_irq_delay;
      break;
    }

    default:
    {
      /* Z80 cpu */
      set_irq_line = z80_set_irq_line;
      set_irq_line_delay = z80_set_irq_line;
      break;
    }
  }
}

void vdp_reset(void)
{
  memset ((char *) sat, 0, sizeof (sat));
  memset ((char *) vram, 0, sizeof (vram));
  memset ((char *) cram, 0, sizeof (cram));
  memset ((char *) vsram, 0, sizeof (vsram));
  memset ((char *) reg, 0, sizeof (reg));

  addr            = 0;
  addr_latch      = 0;
  code            = 0;
  pending         = 0;
  border          = 0;
  hint_pending    = 0;
  vint_pending    = 0;
  dmafill         = 0;
  dma_type        = 0;
  dma_length      = 0;
  dma_endCycles   = 0;
  odd_frame       = 0;
  im2_flag        = 0;
  interlaced      = 0;
  fifo_write_cnt  = 0;
  fifo_lastwrite  = 0;
  cached_write   = -1;

  ntab = 0;
  ntbb = 0;
  ntwb = 0;
  satb = 0;
  hscb = 0;

  vscroll = 0;

  hscroll_mask        = 0x00;
  playfield_shift     = 6;
  playfield_col_mask  = 0x0F;
  playfield_row_mask  = 0x0FF;
  sat_base_mask       = 0xFE00;
  sat_addr_mask       = 0x01FF;

  /* clear pattern cache */
  bg_list_index = 0;
  memset ((char *) bg_name_dirty, 0, sizeof (bg_name_dirty));
  memset ((char *) bg_name_list, 0, sizeof (bg_name_list));
  memset ((char *) bg_pattern_cache, 0, sizeof (bg_pattern_cache));

  /* default HVC */
  hvc_latch = 0x10000;
  hctab = cycle2hc32;
  vc_max = vc_table[0][vdp_pal];
  v_counter = lines_per_frame - 1;

  /* default Window clipping */
  window_clip(0,0);

  /* default FIFO timings */
  fifo_latency = 214;

  /* reset VDP status (FIFO empty flag is set) */
  if (system_hw & SYSTEM_MD)
  {
    status = vdp_pal | 0x200;
  }
  else
  {
    status = 0;
  }

  /* default display area */
  bitmap.viewport.w   = 256;
  bitmap.viewport.h   = 192;
  bitmap.viewport.ow  = 256;
  bitmap.viewport.oh  = 192;

  /* default overscan area */
  if (system_hw == SYSTEM_GG)
  {
    /* Display area reduced to 160x144 if overscan is disabled */
    bitmap.viewport.x = (config.overscan & 2) ? 14 : -48;
    bitmap.viewport.y = (config.overscan & 1) ? (24 * (vdp_pal + 1)) : -24;
  }
  else
  {
    bitmap.viewport.x = (config.overscan & 2) * 7;
    bitmap.viewport.y = (config.overscan & 1) * 24 * (vdp_pal + 1);
  }

  /* default rendering mode */
  update_bg_pattern_cache = update_bg_pattern_cache_m4;
  if (system_hw < SYSTEM_SMS)
  {
    /* Mode 0 */
    render_bg = render_bg_m0;
    render_obj = render_obj_tms;
    parse_satb = parse_satb_tms;
  }
  else
  {
    /* Mode 4 */
    render_bg = render_bg_m4;
    render_obj = render_obj_m4;
    parse_satb = parse_satb_m4;
  }

  /* default 68k bus access mode (Mode 4) */
  vdp_68k_data_w = vdp_68k_data_w_m4;
  vdp_68k_data_r = vdp_68k_data_r_m4;

  /* default Z80 bus access mode */
  switch (system_hw)
  {
    case SYSTEM_SG:
    {
      /* SG-1000 port access */
      vdp_z80_data_w = vdp_z80_data_w_sg;
      vdp_z80_data_r = vdp_z80_data_r_m4;
      break;
    }

    case SYSTEM_GG:
    {
      /* Game Gear port access */
      vdp_z80_data_w = vdp_z80_data_w_gg;
      vdp_z80_data_r = vdp_z80_data_r_m4;
      break;
    }

    case SYSTEM_MARKIII:
    case SYSTEM_SMS:
    case SYSTEM_SMS2:
    case SYSTEM_GGMS:
    {
      /* Master System port access */
      vdp_z80_data_w = vdp_z80_data_w_ms;
      vdp_z80_data_r = vdp_z80_data_r_m4;
      break;
    }

    default:
    {
      /* Genesis port access */
      vdp_z80_data_w = vdp_z80_data_w_m4;
      vdp_z80_data_r = vdp_z80_data_r_m4;
      break;
    }
  }

  if (system_hw == SYSTEM_SG)
  {
    /* disable H-INT */
    vdp_reg_w(10, 0xFF, 0);
  }
  else if ((system_hw > SYSTEM_MARKIII) && (system_hw < SYSTEM_MD))
  {
    /* initialize registers normally set by Master System BIOS */
    vdp_reg_w(0 , 0x36, 0);
    vdp_reg_w(1 , 0x80, 0);
    vdp_reg_w(2 , 0xFF, 0);
    vdp_reg_w(3 , 0xFF, 0);
    vdp_reg_w(4 , 0xFF, 0);
    vdp_reg_w(5 , 0xFF, 0);
    vdp_reg_w(6 , 0xFF, 0);
    vdp_reg_w(10, 0xFF, 0);
  }
  else if ((system_hw != SYSTEM_PBC) && (config.tmss == 1))
  {
    /* initialize registers if BIOS is simulated */
    vdp_reg_w(0 , 0x04, 0);
    vdp_reg_w(1 , 0x04, 0);
    vdp_reg_w(10, 0xFF, 0);
    vdp_reg_w(12, 0x81, 0);
    vdp_reg_w(15, 0x02, 0);
  }

  /* reset color palette */
  int i;
  for(i = 0; i < 0x20; i ++)
  {
    color_update_m4(i, 0x00);
  }
  color_update_m4(0x40, 0x00);
}

int vdp_context_save(uint8 *state)
{
  int bufferptr = 0;

  save_param(sat, sizeof(sat));
  save_param(vram, sizeof(vram));
  save_param(cram, sizeof(cram));
  save_param(vsram, sizeof(vsram));
  save_param(reg, sizeof(reg));
  save_param(&addr, sizeof(addr));
  save_param(&addr_latch, sizeof(addr_latch));
  save_param(&code, sizeof(code));
  save_param(&pending, sizeof(pending));
  save_param(&status, sizeof(status));
  save_param(&dmafill, sizeof(dmafill));
  save_param(&hint_pending, sizeof(hint_pending));
  save_param(&vint_pending, sizeof(vint_pending));
  save_param(&dma_length, sizeof(dma_length));
  save_param(&dma_type, sizeof(dma_type));
  save_param(&cached_write, sizeof(cached_write));
  return bufferptr;
}

int vdp_context_load(uint8 *state, char *version)
{
  int i, bufferptr = 0;
  uint8 temp_reg[0x20];

  load_param(sat, sizeof(sat));
  load_param(vram, sizeof(vram));
  load_param(cram, sizeof(cram));
  load_param(vsram, sizeof(vsram));
  load_param(temp_reg, sizeof(temp_reg));
  load_param(&addr, sizeof(addr));
  load_param(&addr_latch, sizeof(addr_latch));
  load_param(&code, sizeof(code));
  load_param(&pending, sizeof(pending));
  load_param(&status, sizeof(status));
  load_param(&dmafill, sizeof(dmafill));
  load_param(&hint_pending, sizeof(hint_pending));
  load_param(&vint_pending, sizeof(vint_pending));
  if ((version[11] <= 0x31) && (version[13] <= 0x35))
  {
    uint8 dummy;
    load_param(&dummy, sizeof(dummy));
  }
  load_param(&dma_length, sizeof(dma_length));
  load_param(&dma_type, sizeof(dma_type));
  load_param(&cached_write, sizeof(cached_write));

  /* restore VDP registers */
  if (system_hw < SYSTEM_MD)
  {
    /* save internal data */
    uint8 old_pending = pending;  
    uint8 old_code = code;
    uint16 old_addr = addr;
    uint16 old_addr_latch = addr_latch;

    if (system_hw > SYSTEM_SG)
    {
      for (i=0;i<0x10;i++) 
      {
        pending = 1;
        addr_latch = temp_reg[i];
        vdp_sms_ctrl_w(0x80 | i);
      }
    }
    else
    {
      for (i=0;i<0x08;i++) 
      {
        pending = 1;
        addr_latch = temp_reg[i];
        vdp_tms_ctrl_w(0x80 | i);
      }
    }

    /* restore internal data */
    pending = old_pending;  
    code = old_code;
    addr = old_addr;
    addr_latch = old_addr_latch;
  }
  else
  {
    for (i=0;i<0x20;i++) 
    {
      vdp_reg_w(i, temp_reg[i], 0);
    }
  }

  /* restore FIFO timings */
  fifo_latency = 214 - (reg[12] & 1) * 24;
  fifo_latency <<= ((code & 0x0F) == 0x01);

  /* restore current NTSC/PAL mode */
  if (system_hw & SYSTEM_MD)
  {
    status = (status & ~1) | vdp_pal;
  }

  if (reg[1] & 0x04)
  {
    /* Mode 5 */
    bg_list_index = 0x800;

    /* reinitialize palette */
    color_update_m5(0, *(uint16 *)&cram[border << 1]);
    for(i = 1; i < 0x40; i++)
    {
      color_update_m5(i, *(uint16 *)&cram[i << 1]);
    }
  }
  else
  {
    /* Modes 0,1,2,3,4 */
    bg_list_index = 0x200;

    /* reinitialize palette */
    for(i = 0; i < 0x20; i ++)
    {
      color_update_m4(i, *(uint16 *)&cram[i << 1]);
    }
    color_update_m4(0x40, *(uint16 *)&cram[(0x10 | (border & 0x0F)) << 1]);
  }

  /* invalidate cache */
  for (i=0;i<bg_list_index;i++) 
  {
    bg_name_list[i]=i;
    bg_name_dirty[i]=0xFF;
  }

  return bufferptr;
}


/*--------------------------------------------------------------------------*/
/* DMA update function                                                      */
/*--------------------------------------------------------------------------*/

void vdp_dma_update(unsigned int cycles)
{
  int dma_cycles;

  /* DMA transfer rate (bytes per line) 

     According to the manual, here's a table that describes the transfer
   rates of each of the three DMA types:

      DMA Mode      Width       Display      Transfer Count
      -----------------------------------------------------
      68K > VDP     32-cell     Active       16
                                Blanking     167
                    40-cell     Active       18
                                Blanking     205
      VRAM Fill     32-cell     Active       15
                                Blanking     166
                    40-cell     Active       17
                                Blanking     204
      VRAM Copy     32-cell     Active       8
                                Blanking     83
                    40-cell     Active       9
                                Blanking     102

   'Active' is the active display period, 'Blanking' is either the vertical
   blanking period or when the display is forcibly blanked via register #1.

   The above transfer counts are all in bytes, unless the destination is
   CRAM or VSRAM for a 68K > VDP transfer, in which case it is in words.
  */
  unsigned int rate = dma_timing[(status & 8) || !(reg[1] & 0x40)][reg[12] & 1];

  /* Adjust for 68k bus DMA to VRAM (one word = 2 access) or DMA Copy (one read + one write = 2 access) */
  rate = rate >> (dma_type & 1);

  /* Remaining DMA cycles */
  if (status & 8)
  {
    /* Process DMA until the end of VBLANK (speed optimization) */
    /* Note: This is not 100% accurate since rate could change if display width */
    /* is changed during VBLANK but no games seem to do this. */
    dma_cycles = (lines_per_frame * MCYCLES_PER_LINE) - cycles;
  }
  else
  {
    /* Process DMA until the end of current line */
    dma_cycles = (mcycles_vdp + MCYCLES_PER_LINE) - cycles;
  }

  /* Remaining DMA bytes for that line */
  int dma_bytes = (dma_cycles * rate) / MCYCLES_PER_LINE;

#ifdef LOGVDP
  error("[%d(%d)][%d(%d)] DMA type %d (%d access/line)(%d cycles left)-> %d access (%d remaining) (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE,dma_type/4, rate, dma_cycles, dma_bytes, dma_length, m68k_get_reg(M68K_REG_PC));
#endif

  /* Check if DMA can be finished before the end of current line */
  if (dma_length < dma_bytes)
  {
    /* Adjust remaining DMA bytes */
    dma_bytes = dma_length;
    dma_cycles = (dma_bytes * MCYCLES_PER_LINE) / rate;
  }

  /* Update DMA timings */
  if (dma_type < 2)
  {
    /* 68K is frozen during DMA from V-Bus */
    mcycles_68k = cycles + dma_cycles;
  #ifdef LOGVDP
    error("-->CPU frozen for %d cycles\n", dma_cycles);
  #endif
  }
  else
  {
    /* Set DMA Busy flag */
    status |= 0x02;

    /* 68K is still running, set DMA end cycle */
    dma_endCycles = cycles + dma_cycles;
#ifdef LOGVDP
    error("-->DMA ends in %d cycles\n", dma_cycles);
#endif
  }

  /* Process DMA */
  if (dma_bytes > 0)
  {
    /* Update DMA length */
    dma_length -= dma_bytes;

    /* Select DMA operation */
    switch (dma_type)
    {
      case 0:
      case 1:
      {
        /* 68K bus to VRAM, CRAM or VSRAM */
        vdp_dma_vbus(dma_bytes);
        break;
      }

      case 2:
      {
        /* VRAM Fill */
        vdp_dma_fill(dmafill, dma_bytes);
        break;
      }

      case 3:
      {
        /* VRAM Copy */
        vdp_dma_copy(dma_bytes);
        break;
      }
    }

    /* Check if DMA is finished */
    if (!dma_length)
    {
      /* Reset DMA length registers */
      reg[19] = reg[20] = 0;

      /* Perform cached write, if any */
      if (cached_write >= 0)
      {
        vdp_68k_ctrl_w(cached_write);
        cached_write = -1;
      }
    }
  }
}


/*--------------------------------------------------------------------------*/
/* Control port access functions                                            */
/*--------------------------------------------------------------------------*/

void vdp_68k_ctrl_w(unsigned int data)
{
  /* Check pending flag */
  if (pending == 0)
  {
    /* A single long word write instruction could have started DMA with the first word */
    if (dma_length)
    {
      /* 68k is frozen during 68k bus DMA */
      /* Second word should be written after DMA completion */
      /* See Formula One & Kawasaki Superbike Challenge */
      if (dma_type < 2)
      {
        /* Latch second control word for later */
        cached_write = data;
        return;
      }
    }

    /* Check CD0-CD1 bits */
    if ((data & 0xC000) == 0x8000)
    {
      /* VDP register write */
      vdp_reg_w((data >> 8) & 0x1F, data & 0xFF, mcycles_68k);
    }
    else
    {
      /* Set pending flag (Mode 5 only) */
      pending = reg[1] & 4;
    }

    /* Update address and code registers */
    addr = addr_latch | (data & 0x3FFF);
    code = ((code & 0x3C) | ((data >> 14) & 0x03));
  }
  else
  {
    /* Clear pending flag */
    pending = 0;

    /* Save address bits A15 and A14 */
    addr_latch = (data & 3) << 14;

    /* Update address and code registers */
    addr = addr_latch | (addr & 0x3FFF);
    code = ((code & 0x03) | ((data >> 2) & 0x3C));

    /* Detect DMA operation (CD5 bit set) */
    if ((code & 0x20) && (reg[1] & 0x10))
    {
      /* DMA type */
      switch (reg[23] >> 6)
      {
        case 2:
        {
          /* VRAM write operation only (Williams Greatest Hits after soft reset) */
          if ((code & 0x0F) == 1)
          {
            /* VRAM fill will be triggered by next write to DATA port */
            dmafill = 0x100;
          }
          break;
        }

        case 3:
        {
          /* VRAM read/write operation only */
          if ((code & 0x1F) == 0x10)
          {
            /* DMA length */
            dma_length = (reg[20] << 8) | reg[19];

            /* Zero DMA length */
            if (!dma_length)
            {
              dma_length = 0x10000;
            }

            /* VRAM copy */
            dma_type = 3;
            vdp_dma_update(mcycles_68k);
          }
          break;
        }

        default:
        {
          /* DMA length */
          dma_length = (reg[20] << 8) | reg[19];

          /* Zero DMA length */
          if (!dma_length)
          {
            dma_length = 0x10000;
          }

          /* SVP RAM transfer latency */
          reg[21] -= (svp && !(reg[23] & 0x60));

          /* 68k to VDP DMA */
          dma_type = (code & 0x06) ? 0 : 1;
          vdp_dma_update(mcycles_68k);
          break;
        }
      }
    }
  }

  /* 
     FIFO emulation (Chaos Engine/Soldier of Fortune, Double Clutch, Sol Deace) 
     --------------------------------------------------------------------------

      CPU access per line is limited during active display:
         H32: 16 access --> 3420/16 = ~214 Mcycles between access
         H40: 18 access --> 3420/18 = ~190 Mcycles between access

      This is an approximation, on real hardware, the delay between access is
      more likely 16 pixels (128 or 160 Mcycles) with no access allowed during
      HBLANK (~860 Mcycles), H40 mode being probably a little more restricted.

      Each VRAM access is byte wide, so one VRAM write (word) need twice cycles.

  */
  fifo_latency = 214 - (reg[12] & 1) * 24;
  fifo_latency <<= ((code & 0x0F) == 0x01);
}

/* Mega Drive VDP control port specific (MS compatibility mode) */
void vdp_z80_ctrl_w(unsigned int data)
{
  switch (pending)
  {
    case 0:
    {
      /* Latch LSB */
      addr_latch = data;

      /* Set LSB pending flag */
      pending = 1;
      return;
    }

    case 1:
    {
      /* Update address and code registers */
      addr = (addr & 0xC000) | ((data & 0x3F) << 8) | addr_latch ;
      code = ((code & 0x3C) | ((data >> 6) & 0x03));

      if ((code & 0x03) == 0x02)
      {
        /* VDP register write */
        vdp_reg_w(data & 0x1F, addr_latch, mcycles_z80);

        /* Clear pending flag  */
        pending = 0;
        return;
      }

      /* Set Mode 5 pending flag  */
      pending = (reg[1] & 4) >> 1;

      if (!pending && !(code & 0x03))
      {
        /* Process VRAM read */
        fifo[0] = vram[addr & 0x3FFF];

        /* Increment address register */
        addr += (reg[15] + 1);
      }
      return;
    }

    case 2:
    {
      /* Latch LSB */
      addr_latch = data;

      /* Set LSB pending flag */
      pending = 3;
      return;
    }

    case 3:
    {
      /* Clear pending flag  */
      pending = 0;

      /* Update address and code registers */
      addr = ((addr_latch & 3) << 14) | (addr & 0x3FFF);
      code = ((code & 0x03) | ((addr_latch >> 2) & 0x3C));

      /* Detect DMA operation */
      if ((code & 0x20) && (reg[1] & 0x10))
      {
        switch (reg[23] >> 6)
        {
          case 2:
          {
            /* VRAM write operation only (Williams Greatest Hits after soft reset) */
            if ((code & 0x0F) == 1)
            {
              /* VRAM fill will be triggered by next write to DATA port */
              dmafill = 0x100;
            }
            break;
          }

          case 3:
          {
            /* VRAM read/write operation only */
            if ((code & 0x1F) == 0x10)
            {
              /* DMA length */
              dma_length = (reg[20] << 8) | reg[19];

              /* Zero DMA length */
              if (!dma_length)
              {
                dma_length = 0x10000;
              }

              /* VRAM copy */
              dma_type = 3;
              vdp_dma_update(mcycles_z80);
            }
            break;
          }

          default:
          {
            /* DMA from V-Bus does not work when Z80 is in control */
            break;
          }
        }
      }
    }
    return;
  }
}

/* Master System & Game Gear VDP control port specific */
void vdp_sms_ctrl_w(unsigned int data)
{
  if(pending == 0)
  {
    /* Update address register LSB */
    addr = (addr & 0x3F00) | (data & 0xFF);

    /* Latch LSB */
    addr_latch = data;

    /* Set LSB pending flag */
    pending = 1;
  }
  else
  {
    /* Update address and code registers */
    code = (data >> 6) & 3;
    addr = (data << 8 | addr_latch) & 0x3FFF;

    /* Clear pending flag  */
    pending = 0;

    if (code == 0)
    {
      /* Process VRAM read */
      fifo[0] = vram[addr & 0x3FFF];

      /* Increment address register */
      addr = (addr + 1) & 0x3FFF;
      return;
    }

    if (code == 2)
    {
      /* VDP register index (0-15) */
      data &= 0x0F;

      /* Save current VDP mode */
      int prev = (reg[0] & 0x06) | (reg[1] & 0x18);

      /* Write VDP register */
      vdp_reg_w(data, addr_latch, mcycles_z80);

      /* Check VDP mode changes */
      int mode = (reg[0] & 0x06) | (reg[1] & 0x18);
      prev ^= mode;
 
      if (prev)
      {
        /* Check for extended modes */
        if (system_hw > SYSTEM_SMS)
        {
          int height;

          if (mode == 0x0E) /* M1=0,M2=1,M3=1,M4=1 */
          {
            /* Mode 4 extended (240 lines) */
            height = 240;

            /* Update vertical counter max value */
            vc_max = vc_table[3][vdp_pal];
          }
          else if (mode == 0x16) /* M1=1,M2=1,M3=0,M4=1 */
          {
            /* Mode 4 extended (224 lines) */
            height = 224;

            /* Update vertical counter max value */
            vc_max = vc_table[1][vdp_pal];
          }
          else
          {
            /* Mode 4 default (224 lines) */
            height = 192;

            /* Default vertical counter max value */
            vc_max = vc_table[0][vdp_pal];
          }

          if (height != bitmap.viewport.h)
          {
            if (v_counter < bitmap.viewport.h)
            {
              /* update active display */
              bitmap.viewport.h = height;

              /* update vertical overscan */
              if (config.overscan & 1)
              {
                bitmap.viewport.y = (240 + 48*vdp_pal - height) >> 1;
              }
              else
              {
                if (system_hw == SYSTEM_GG)
                {
                  /* Display area reduced to 160x144 */
                  bitmap.viewport.y = (144 - height) / 2;
                }
                else
                {
                  bitmap.viewport.y = 0;
                }
              }
            }
            else
            {
              /* Changes should be applied on next frame */
              bitmap.viewport.changed |= 2;
            }
          }
        }

        /* Rendering mode */
        switch (mode)
        {
          case 0x00: /* Graphics I */
          {
            render_bg = render_bg_m0;
            break;
          }

          case 0x10: /* Text */
          {
            render_bg = render_bg_m1;
           break;
          }

          case 0x02: /* Graphics II */
          {
            render_bg = render_bg_m2;
            break;
          }

          case 0x12: /* Text (Extended PG) */
          {
            render_bg = render_bg_m1x;
            break;
          }

          case 0x08: /* Multicolor */
          {
            render_bg = render_bg_m3;
            break;
          }

          case 0x18: /* Invalid (1+3) */
          {
            render_bg = render_bg_inv;
            break;
          }

          case 0x0A: /* Multicolor (Extended PG) */
          {
            render_bg = render_bg_m3x;
            break;
          }

          case 0x1A: /* Invalid (1+2+3) */
          {
            render_bg = render_bg_inv;
           break;
          }

          default: /* Mode 4 */
          {
            render_bg = render_bg_m4;
            break;
          }
        }

        /* Mode switching */
        if (prev & 0x04)
        {
          if (mode & 0x04)
          {
            /* Mode 4 sprites */
            parse_satb = parse_satb_m4;
            render_obj = render_obj_m4;

            /* force BG cache update*/
            bg_list_index = 0x200;
          }
          else
          {
            /* TMS-mode sprites */
            parse_satb = parse_satb_tms;
            render_obj = render_obj_tms;

            /* BG cache is not used */
            bg_list_index = 0;
          }

          /* reinitialize palette */
          int i;
          for(i = 0; i < 0x20; i ++)
          {
            color_update_m4(i, *(uint16 *)&cram[i << 1]);
          }
          color_update_m4(0x40, *(uint16 *)&cram[(0x10 | (border & 0x0F)) << 1]);
        }
      }
    }
  }
}

/* TMS9918 (SG-1000) VDP control port specific */
void vdp_tms_ctrl_w(unsigned int data)
{
  if(pending == 0)
  {
    /* Latch LSB */
    addr_latch = data;

    /* Set LSB pending flag */
    pending = 1;
  }
  else
  {
    /* Update address and code registers */
    code = (data >> 6) & 3;
    addr = (data << 8 | addr_latch) & 0x3FFF;

    /* Clear pending flag  */
    pending = 0;

    if (code == 0)
    {
      /* Process VRAM read */
      fifo[0] = vram[addr & 0x3FFF];

      /* Increment address register */
      addr = (addr + 1) & 0x3FFF;
      return;
    }

    if (code & 2)
    {
      /* VDP register index (0-7) */
      data &= 0x07;

      /* Write VDP register */
      vdp_reg_w(data, addr_latch, mcycles_z80);
 
      /* Check VDP mode changes */
      if (data < 2)
      {
        int mode = (reg[0] & 0x02) | (reg[1] & 0x18);

        /* Rendering mode */
        switch (mode)
        {
          case 0x00: /* Graphics I */
          {
            render_bg = render_bg_m0;
            break;
          }

          case 0x10: /* Text */
          {
            render_bg = render_bg_m1;
            break;
          }

          case 0x02: /* Graphics II */
          {
            render_bg = render_bg_m2;
            break;
          }

          case 0x12: /* Text (Extended PG) */
          {
            render_bg = render_bg_m1x;
            break;
          }

          case 0x08: /* Multicolor */
          {
            render_bg = render_bg_m3;
            break;
          }

          case 0x18: /* Invalid (1+3) */
          {
            render_bg = render_bg_inv;
            break;
          }

          case 0x0A: /* Multicolor (Extended PG) */
          {
            render_bg = render_bg_m3x;
            break;
          }

          case 0x1A: /* Invalid (1+2+3) */
          {
            render_bg = render_bg_inv;
            break;
          }
        }
      }
    }
  }
}

  /*
   * Status register
   *
   * Bits
   * 0  NTSC(0)/PAL(1)
   * 1  DMA Busy
   * 2  During HBlank
   * 3  During VBlank
   * 4  0:1 even:odd field (interlaced modes only)
   * 5  Sprite collision
   * 6  Too many sprites per line
   * 7  v interrupt occurred
   * 8  Write FIFO full
   * 9  Write FIFO empty
   * 10 - 15  Open Bus
   */
unsigned int vdp_68k_ctrl_r(unsigned int cycles)
{

  /* Update FIFO flags */
  vdp_fifo_update(cycles);

  /* Update DMA Busy flag */
  if ((status & 2) && !dma_length && (cycles >= dma_endCycles))
  {
    status &= 0xFFFD;
  }

  /* Return VDP status */
  unsigned int temp = status;

  /* Clear pending flag */
  pending = 0;

  /* Clear SOVR & SCOL flags */
  status &= 0xFF9F;

  /* Display OFF: VBLANK flag is set */
  if (!(reg[1] & 0x40))
  {
    temp |= 0x08;
  }

  /* HBLANK flag (Sonic 3 and Sonic 2 "VS Modes", Lemmings 2, Mega Turrican, V.R Troopers, Gouketsuji Ichizoku,...) */
  /* NB: this is not 100% accurate and need to be verified on real hardware */
  if ((cycles % MCYCLES_PER_LINE) < 588)
  {
    temp |= 0x04;
  }

#ifdef LOGVDP
  error("[%d(%d)][%d(%d)] VDP 68k status read -> 0x%x (0x%x) (%x)\n", v_counter, cycles/MCYCLES_PER_LINE-1, cycles, cycles%MCYCLES_PER_LINE, temp, status, m68k_get_reg(M68K_REG_PC));
#endif
  return (temp);
}

unsigned int vdp_z80_ctrl_r(unsigned int cycles)
{
  /* Update DMA Busy flag (Mega Drive VDP specific) */
  if ((system_hw & SYSTEM_MD) && (status & 2) && !dma_length && (cycles >= dma_endCycles))
  {
    status &= 0xFD;
  }

  /* Cycle-accurate SOVR & VINT flags */
  int line = (lines_per_frame + (mcycles_z80 / MCYCLES_PER_LINE) - 1) % lines_per_frame;

  /* Check if we are already on next line */
  if (line > v_counter)
  {
    v_counter = line;
    if (line == (bitmap.viewport.h + 1))
    {
      /* set VINT flag (immediately cleared after) */
      status |= 0x80;
    }
    else if ((line >= 0) && (line < bitmap.viewport.h) && !(work_ram[0x1ffb] & cart.special))
    {
      /* Check sprites overflow & collision */
      render_line(line);
    }
  }

  /* Return VDP status */
  unsigned int temp = status;

  /* Clear pending flag */
  pending = 0;

  /* Clear VINT, SOVR & SCOL flags */
  status &= 0xFF1F;

  /* Mega Drive VDP specific */
  if (system_hw & SYSTEM_MD)
  {
    /* Display OFF: VBLANK flag is set */
    if (!(reg[1] & 0x40))
    {
      temp |= 0x08;
    }

    /* HBLANK flag */
    if ((cycles % MCYCLES_PER_LINE) < 588)
    {
      temp |= 0x04;
    }
  }
  else if (reg[0] & 0x04)
  {
    /* Mode 4 unused bits (fixes PGA Tour Golf) */
    temp |= 0x1F;
  }

  /* Cycle-accurate SCOL flag */
  if ((temp & 0x20) && (line == (spr_col >> 8)))
  {
    if (system_hw & SYSTEM_MD)
    {
      /* COL flag is set at HCount 0xFF on MD */
      if ((cycles % MCYCLES_PER_LINE) < 105)
      {
        status |= 0x20;
        temp &= ~0x20;
      }
    }
    else
    {
      /* COL flag is set at the pixel it occurs */
      uint8 hc = hctab[(cycles + SMS_CYCLE_OFFSET + 15) % MCYCLES_PER_LINE];
      if ((hc < (spr_col & 0xff)) || (hc > 0xf3))
      {
        status |= 0x20;
        temp &= ~0x20;
      }
    }
  }

  /* Clear HINT & VINT pending flags */
  hint_pending = vint_pending = 0;

  /* Clear Z80 interrupt */
  Z80.irq_state = CLEAR_LINE;

#ifdef LOGVDP
  error("[%d(%d)][%d(%d)] VDP Z80 status read -> 0x%x (0x%x) (%x)\n", v_counter, cycles/MCYCLES_PER_LINE-1, cycles, cycles%MCYCLES_PER_LINE, temp, status, Z80.pc.w.l);
#endif
  return (temp);
}

/*--------------------------------------------------------------------------*/
/* HV Counters                                                              */
/*--------------------------------------------------------------------------*/

unsigned int vdp_hvc_r(unsigned int cycles)
{
  /* VCounter */
  int vc = (cycles / MCYCLES_PER_LINE) - 1;

  /* Check counter overflow */
  if (vc > vc_max)
  {
    vc -= lines_per_frame;
  }

  /* Check interlaced modes */
  if (interlaced)
  {
    /* Interlace mode 2 (Sonic the Hedgehog 2, Combat Cars) */
    vc <<= im2_flag;

    /* Replace bit 0 with bit 8 */
    vc = (vc & ~1) | ((vc >> 8) & 1);
  }

  /* Returned value */
  unsigned int temp = (vc & 0xff) << 8;

  /* Check if HVC is frozen */
  if (!hvc_latch)
  {
    /* Cycle-accurate HCounter (Striker, Mickey Mania, Skitchin, Road Rash I,II,III, Sonic 3D Blast...) */
    temp |= hctab[cycles % MCYCLES_PER_LINE];
  }
  else
  {
    if (reg[1] & 4)
    {
      /* Mode 5: both counters are frozen (Lightgun games, Sunset Riders) */
      temp = hvc_latch & 0xffff;
    }
    else
    {
      /* Mode 4: VCounter runs normally, HCounter is frozen */
      temp |= (hvc_latch & 0xff);
    }
  }

#ifdef LOGVDP
  error("[%d(%d)][%d(%d)] HVC read -> 0x%x (%x)\n", v_counter, (cycles/MCYCLES_PER_LINE-1)%lines_per_frame, cycles, cycles%MCYCLES_PER_LINE, temp, m68k_get_reg(M68K_REG_PC));
#endif
  return (temp);
}


/*--------------------------------------------------------------------------*/
/* Test registers                                                           */
/*--------------------------------------------------------------------------*/

void vdp_test_w(unsigned int data)
{
#ifdef LOGERROR
  error("Unused VDP Write 0x%x (%08x)\n", data, m68k_get_reg(M68K_REG_PC));
#endif
}


/*--------------------------------------------------------------------------*/
/* 68k interrupt handler (TODO: check how interrupts are handled in Mode 4) */
/*--------------------------------------------------------------------------*/

int vdp_68k_irq_ack(int int_level)
{
#ifdef LOGVDP
  error("[%d(%d)][%d(%d)] INT Level %d ack (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE,int_level, m68k_get_reg(M68K_REG_PC));
#endif

  /* VINT has higher priority (Fatal Rewind) */
  if (vint_pending & reg[1])
  {
#ifdef LOGVDP
    error("---> VINT cleared\n");
#endif

    /* Clear VINT pending flag */
    vint_pending = 0;
    status &= ~0x80;

    /* Update IRQ status */
    if (hint_pending & reg[0])
    {
      m68k_set_irq(4);
    }
    else
    {
      m68k_set_irq(0);
    }
  }
  else
  {
#ifdef LOGVDP
    error("---> HINT cleared\n");
#endif

    /* Clear HINT pending flag */
    hint_pending = 0;

    /* Update IRQ status */
    m68k_set_irq(0);
  }

  return M68K_INT_ACK_AUTOVECTOR;
}


/*--------------------------------------------------------------------------*/
/* VDP registers update function                                            */
/*--------------------------------------------------------------------------*/

static void vdp_reg_w(unsigned int r, unsigned int d, unsigned int cycles)
{
#ifdef LOGVDP
  error("[%d(%d)][%d(%d)] VDP register %d write -> 0x%x (%x)\n", v_counter, cycles/MCYCLES_PER_LINE-1, cycles, cycles%MCYCLES_PER_LINE, r, d, m68k_get_reg(M68K_REG_PC));
#endif

  /* VDP registers #11 to #23 cannot be updated in Mode 4 (Captain Planet & Avengers, Bass Master Classic Pro Edition) */
  if (!(reg[1] & 4) && (r > 10))
  {
    return;
  }

  switch(r)
  {
    case 0: /* CTRL #1 */
    {
      /* Look for changed bits */
      r = d ^ reg[0];
      reg[0] = d;

      /* Line Interrupt */
      if ((r & 0x10) && hint_pending)
      {
        /* Update IRQ status */
        if (vint_pending & reg[1])
        {
          set_irq_line(6);
        }
        else if (d & 0x10)
        {
          set_irq_line_delay(4);
        }
        else
        {
          set_irq_line(0);
        }
      }

      /* Palette selection */
      if (r & 0x04)
      {
        /* Mega Drive VDP only */
        if (system_hw & SYSTEM_MD)
        {
          /* Reset color palette */
          int i;
          if (reg[1] & 0x04)
          {
            /* Mode 5 */
            color_update_m5(0x00, *(uint16 *)&cram[border << 1]);
            for (i = 1; i < 0x40; i++)
            {
              color_update_m5(i, *(uint16 *)&cram[i << 1]);
            }
          }
          else
          {
            /* Mode 4 */
            for (i = 0; i < 0x20; i++)
            {
              color_update_m4(i, *(uint16 *)&cram[i << 1]);
            }
            color_update_m4(0x40, *(uint16 *)&cram[(0x10 | (border & 0x0F)) << 1]);
          }
        }
      }

      /* HVC latch (Sunset Riders, Lightgun games) */
      if (r & 0x02)
      {
        /* Mega Drive VDP only */
        if (system_hw & SYSTEM_MD)
        {
          /* Mode 5 only */
          if (reg[1] & 0x04)
          {
            if (d & 0x02)
            {
              /* Latch current HVC */
              hvc_latch = vdp_hvc_r(cycles) | 0x10000;
            }
            else
            {
              /* Free-running HVC */
              hvc_latch = 0;
            }
          }
        }
      }
      break;
    }

    case 1: /* CTRL #2 */
    {
      /* Look for changed bits */
      r = d ^ reg[1];
      reg[1] = d;

      /* Display status (modified during active display) */
      if ((r & 0x40) && (v_counter < bitmap.viewport.h))
      {
        /* Cycle offset vs HBLANK */
        int offset = cycles - mcycles_vdp - 860;
        if (offset <= 0)
        {
          /* If display was disabled during HBLANK (Mickey Mania 3D level), sprite rendering is limited  */
          if ((d & 0x40) && (object_count > 5) && (offset >= -500))
          {
            object_count = 5;
          }

          /* Redraw entire line (Legend of Galahad, Lemmings 2, Formula One, Kawasaki Super Bike, Deadly Moves,...) */
          render_line(v_counter);

#ifdef LOGVDP
          error("Line redrawn (%d sprites) \n",object_count);
#endif
        }
        else if (system_hw & SYSTEM_MD)
        {
          /* Active pixel offset  */
          if (reg[12] & 1)
          {
            /* dot clock = MCLK / 8 */
            offset = (offset / 8);
          }
          else
          {
            /* dot clock = MCLK / 10 */
            offset = (offset / 10) + 16;
          }

          /* Line is partially blanked (Nigel Mansell's World Championship Racing , Ren & Stimpy Show, ...) */
          if (offset < bitmap.viewport.w)
          {
#ifdef LOGVDP
            error("Line %d redrawn from pixel %d\n",v_counter,offset);
#endif
            if (d & 0x40)
            {
              render_line(v_counter);
              blank_line(v_counter, 0, offset);
            }
            else
            {
              blank_line(v_counter, offset, bitmap.viewport.w - offset);
            }
          }
        }
      }

      /* Frame Interrupt */
      if ((r & 0x20) && vint_pending)
      {
        /* Update IRQ status */
        if (d & 0x20) 
        {
          set_irq_line_delay(6);
        }
        else if (hint_pending & reg[0])
        {
          set_irq_line(4);
        }
        else
        {
          set_irq_line(0);
        }
      }

      /* Active display height */
      if (r & 0x08)
      {
        /* Mega Drive VDP only */
        if (system_hw & SYSTEM_MD)
        {
          /* Mode 5 only */
          if (d & 0x04)
          {
            if (v_counter < bitmap.viewport.h)
            {
              /* Update active display height */
              bitmap.viewport.h = 224 + ((d & 8) << 1);
              bitmap.viewport.y = (config.overscan & 1) * (8 - (d & 8) + 24*vdp_pal);
            }
            else
            {
              /* Changes should be applied on next frame */
              bitmap.viewport.changed |= 2;
            }

            /* Update vertical counter max value */
            vc_max = vc_table[(d >> 2) & 3][vdp_pal];
          }
        }
      }

      /* Rendering mode */
      if (r & 0x04)
      {
        /* Mega Drive VDP only */
        if (system_hw & SYSTEM_MD)
        {
          int i;
          if (d & 0x04)
          {
            /* Mode 5 rendering */
            parse_satb = parse_satb_m5;
            update_bg_pattern_cache = update_bg_pattern_cache_m5;
            if (im2_flag)
            {
              render_bg = (reg[11] & 0x04) ? render_bg_m5_im2_vs : render_bg_m5_im2;
              render_obj = (reg[12] & 0x08) ? render_obj_m5_im2_ste : render_obj_m5_im2;
            }
            else
            {
              render_bg = (reg[11] & 0x04) ? render_bg_m5_vs : render_bg_m5;
              render_obj = (reg[12] & 0x08) ? render_obj_m5_ste : render_obj_m5;
            }

            /* Reset color palette */
            color_update_m5(0x00, *(uint16 *)&cram[border << 1]);
            for (i = 1; i < 0x40; i++)
            {
              color_update_m5(i, *(uint16 *)&cram[i << 1]);
            }

            /* Mode 5 bus access */
            vdp_68k_data_w = vdp_68k_data_w_m5;
            vdp_z80_data_w = vdp_z80_data_w_m5;
            vdp_68k_data_r = vdp_68k_data_r_m5;
            vdp_z80_data_r = vdp_z80_data_r_m5;

            /* Change display height */
            if (v_counter < bitmap.viewport.h)
            {
              /* Update active display */
              bitmap.viewport.h = 224 + ((d & 8) << 1);
              bitmap.viewport.y = (config.overscan & 1) * (8 - (d & 8) + 24*vdp_pal);
            }
            else
            {
              /* Changes should be applied on next frame */
              bitmap.viewport.changed |= 2;
            }

            /* Clear HVC latched value */
            hvc_latch = 0;

            /* Check if HVC latch bit is set */
            if (reg[0] & 0x02)
            {
              /* Latch current HVC */
              hvc_latch = vdp_hvc_r(cycles) | 0x10000;
            }

            /* max tiles to invalidate */
            bg_list_index = 0x800;
          }
          else
          {
            /* Mode 4 rendering */
            parse_satb = parse_satb_m4;
            update_bg_pattern_cache = update_bg_pattern_cache_m4;
            render_bg = render_bg_m4;
            render_obj = render_obj_m4;

            /* Reset color palette */
            for (i = 0; i < 0x20; i++)
            {
              color_update_m4(i, *(uint16 *)&cram[i << 1]);
            }
            color_update_m4(0x40, *(uint16 *)&cram[(0x10 | (border & 0x0F)) << 1]);

            /* Mode 4 bus access */
            vdp_68k_data_w = vdp_68k_data_w_m4;
            vdp_z80_data_w = vdp_z80_data_w_m4;
            vdp_68k_data_r = vdp_68k_data_r_m4;
            vdp_z80_data_r = vdp_z80_data_r_m4;

            if (v_counter < bitmap.viewport.h)
            {
              /* Update active display height */
              bitmap.viewport.h = 192;
              bitmap.viewport.y = (config.overscan & 1) * 24 * (vdp_pal + 1);
            }
            else
            {
              /* Changes should be applied on next frame */
              bitmap.viewport.changed |= 2;
            }

            /* Latch current HVC */
            hvc_latch = vdp_hvc_r(cycles) | 0x10000;

            /* max tiles to invalidate */
            bg_list_index = 0x200;
          }

          /* Invalidate pattern cache */
          for (i=0;i<bg_list_index;i++) 
          {
            bg_name_list[i] = i;
            bg_name_dirty[i] = 0xFF;
          }

          /* Update vertical counter max value */
          vc_max = vc_table[(d >> 2) & 3][vdp_pal];
        }
        else
        {
          /* No effect (cleared to avoid mode 5 detection elsewhere) */
          reg[1] &= ~0x04;
        }
      }
      break;
    }

    case 2: /* Plane A Name Table Base */
    {
      reg[2] = d;
      ntab = (d << 10) & 0xE000;

      /* Plane A Name Table Base changed during HBLANK */
      if ((v_counter < bitmap.viewport.h) && (reg[1] & 0x40) && (cycles <= (mcycles_vdp + 860)))
      {
        /* render entire line */
        render_line(v_counter);
      }
      break;
    }

    case 3: /* Window Plane Name Table Base */
    {
      reg[3] = d;
      if (reg[12] & 0x01)
      {
        ntwb = (d << 10) & 0xF000;
      }
      else
      {
        ntwb = (d << 10) & 0xF800;
      }

      /* Window Plane Name Table Base changed during HBLANK */
      if ((v_counter < bitmap.viewport.h) && (reg[1] & 0x40) && (cycles <= (mcycles_vdp + 860)))
      {
        /* render entire line */
        render_line(v_counter);
      }
      break;
    }

    case 4: /* Plane B Name Table Base */
    {
      reg[4] = d;
      ntbb = (d << 13) & 0xE000;

      /* Plane B Name Table Base changed during HBLANK (Adventures of Batman & Robin) */
      if ((v_counter < bitmap.viewport.h) && (reg[1] & 0x40) && (cycles <= (mcycles_vdp + 860)))
      {
        /* render entire line */
        render_line(v_counter);
      }

      break;
    }

    case 5: /* Sprite Attribute Table Base */
    {
      reg[5] = d;
      satb = (d << 9) & sat_base_mask;
      break;
    }

    case 7: /* Backdrop color */
    {
      reg[7] = d;

      /* Check if backdrop color changed */
      d &= 0x3F;

      if (d != border)
      {
        /* Update backdrop color */
        border = d;

        /* Reset palette entry */
        if (reg[1] & 4)
        {
          /* Mode 5 */
          color_update_m5(0x00, *(uint16 *)&cram[d << 1]);
        }
        else
        {
          /* Mode 4 */
          color_update_m4(0x40, *(uint16 *)&cram[(0x10 | (d & 0x0F)) << 1]);
        }

        /* Backdrop color modified during HBLANK (Road Rash 1,2,3)*/
        if ((v_counter < bitmap.viewport.h) && (cycles <= (mcycles_vdp + 860)))
        {
          /* remap entire line */
          remap_line(v_counter);
        }
      }
      break;
    }

    case 8:   /* Horizontal Scroll (Mode 4 only) */
    {
      /* Hscroll is latched at HCount 0xF3, HCount 0xF6 on MD */
      /* Line starts at HCount 0xF4, HCount 0xF6 on MD */
      if (system_hw < SYSTEM_MD)
      {
        cycles = cycles + 15;
      }

      /* Make sure Hscroll has not already been latched */
      int line = (lines_per_frame + (cycles / MCYCLES_PER_LINE) - 1) % lines_per_frame;
      if ((line > v_counter) && (line < bitmap.viewport.h) && !(work_ram[0x1ffb] & cart.special))
      {
        v_counter = line;
        render_line(line);
      }

      reg[8] = d;
      break;
    }

    case 11:  /* CTRL #3 */
    {
      reg[11] = d;

      /* Horizontal scrolling mode */
      hscroll_mask = hscroll_mask_table[d & 0x03];

      /* Vertical Scrolling mode */
      if (d & 0x04)
      {
        render_bg = im2_flag ? render_bg_m5_im2_vs : render_bg_m5_vs;
      }
      else
      {
        render_bg = im2_flag ? render_bg_m5_im2 : render_bg_m5;
      }
      break;
    }

    case 12:  /* CTRL #4 */
    {
      /* Look for changed bits */
      r = d ^ reg[12];
      reg[12] = d;

      /* Shadow & Highlight mode */
      if (r & 0x08)
      {
        /* Reset color palette */
        int i;
        color_update_m5(0x00, *(uint16 *)&cram[border << 1]);
        for (i = 1; i < 0x40; i++)
        {
          color_update_m5(i, *(uint16 *)&cram[i << 1]);
        }

        /* Update sprite rendering function */
        if (d & 0x08)
        {
          render_obj = im2_flag ? render_obj_m5_im2_ste : render_obj_m5_ste;
        }
        else
        {
          render_obj = im2_flag ? render_obj_m5_im2 : render_obj_m5;
        }
      }

      /* Interlaced modes */
      if (r & 0x06)
      {
        /* changes should be applied on next frame */
        bitmap.viewport.changed |= 2;
      }

      /* Active display width */
      if (r & 0x01)
      {
        if (d & 0x01)
        {
          /* Update display-dependant registers */
          ntwb = (reg[3] << 10) & 0xF000;
          satb = (reg[5] << 9) & 0xFC00;
          sat_base_mask = 0xFC00;
          sat_addr_mask = 0x03FF;

          /* Update HC table */
          hctab = cycle2hc40;

          /* Update clipping */
          window_clip(reg[17], 1);

          /* Update fifo timings */
          fifo_latency = 190;
        }
        else
        {
          /* Update display-dependant registers */
          ntwb = (reg[3] << 10) & 0xF800;
          satb = (reg[5] << 9) & 0xFE00;
          sat_base_mask = 0xFE00;
          sat_addr_mask = 0x01FF;

          /* Update HC table */
          hctab = cycle2hc32;

          /* Update clipping */
          window_clip(reg[17], 0);

          /* Update FIFO timings */
          fifo_latency = 214;
        }

        /* Adjust FIFO timings for VRAM writes */
        fifo_latency <<= ((code & 0x0F) == 0x01);

        /* Active display width modified during HBLANK (Bugs Bunny Double Trouble) */
        if ((v_counter < bitmap.viewport.h) && (cycles <= (mcycles_vdp + 860)))
        {
          /* Update active display width */
          bitmap.viewport.w = 256 + ((d & 1) << 6);

          /* Redraw entire line */
          render_line(v_counter);
        }
        else
        {
          /* Changes should be applied on next frame (Golden Axe III intro) */
          /* NB: This is not 100% accurate but is required by GCN/Wii port (GX texture direct mapping) */
          /* and isn't noticeable anyway since display is generally disabled when active width is modified */
          bitmap.viewport.changed |= 2;
        }
      }
      break;
    }

    case 13: /* HScroll Base Address */
    {
      reg[13] = d;
      hscb = (d << 10) & 0xFC00;
      break;
    }

    case 16: /* Playfield size */
    {
      reg[16] = d;
      playfield_shift = shift_table[(d & 3)];
      playfield_col_mask = col_mask_table[(d & 3)];
      playfield_row_mask = row_mask_table[(d >> 4) & 3];
      break;
    }

    case 17: /* Window/Plane A vertical clipping */
    {
      reg[17] = d;
      window_clip(d, reg[12] & 1);
      break;
    }

    default:
    {
      reg[r] = d;
      break;
    }
  }
}


/*--------------------------------------------------------------------------*/
/* FIFO update function (Genesis mode only)                                 */
/*--------------------------------------------------------------------------*/

static void vdp_fifo_update(unsigned int cycles)
{
  if (fifo_write_cnt > 0)
  {
    /* Get number of FIFO reads */
    int fifo_read = ((cycles - fifo_lastwrite) / fifo_latency);

    if (fifo_read > 0)
    {
      /* Process FIFO entries */
      fifo_write_cnt -= fifo_read;

      /* Clear FIFO full flag */
      status &= 0xFEFF;

      /* Check remaining FIFO entries */
      if (fifo_write_cnt <= 0)
      {
        /* Set FIFO empty flag */
        status |= 0x200; 
        fifo_write_cnt = 0;
      }

      /* Update FIFO cycle count */
      fifo_lastwrite += (fifo_read * fifo_latency);
    }
  }
}


/*--------------------------------------------------------------------------*/
/* Internal 16-bit data bus access function (Mode 5 only)                   */
/*--------------------------------------------------------------------------*/

static void vdp_bus_w(unsigned int data)
{
  /* Check destination code */
  switch (code & 0x0F)
  {
    case 0x01:  /* VRAM */
    {
#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] VRAM 0x%x write -> 0x%x (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, addr, data, m68k_get_reg(M68K_REG_PC));
#endif
      /* Byte-swap data if A0 is set */
      if (addr & 1)
      {
        data = ((data >> 8) | (data << 8)) & 0xFFFF;
      }

      /* VRAM address */
      int index = addr & 0xFFFE;

      /* Pointer to VRAM */
      uint16 *p = (uint16 *)&vram[index];

      /* Intercept writes to Sprite Attribute Table */
      if ((index & sat_base_mask) == satb)
      {
        /* Update internal SAT */
        *(uint16 *) &sat[index & sat_addr_mask] = data;
      }

      /* Only write unique data to VRAM */
      if (data != *p)
      {
        /* Write data to VRAM */
        *p = data;

        /* Update pattern cache */
        int name;
        MARK_BG_DIRTY (index);
      }
      break;
    }

    case 0x03:  /* CRAM */
    {
#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] CRAM 0x%x write -> 0x%x (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, addr, data, m68k_get_reg(M68K_REG_PC));
#endif
      /* Pointer to CRAM 9-bit word */
      uint16 *p = (uint16 *)&cram[addr & 0x7E];

      /* Pack 16-bit bus data (BBB0GGG0RRR0) to 9-bit CRAM data (BBBGGGRRR) */
      data = ((data & 0xE00) >> 3) | ((data & 0x0E0) >> 2) | ((data & 0x00E) >> 1);

      /* Check if CRAM data is being modified */
      if (data != *p)
      {
        /* Write CRAM data */
        *p = data;

        /* CRAM index (64 words) */
        int index = (addr >> 1) & 0x3F;

        /* Color entry 0 of each palette is never displayed (transparent pixel) */
        if (index & 0x0F)
        {
          /* Update color palette */
          color_update_m5(index, data);
        }

        /* Update backdrop color */
        if (index == border)
        {
          color_update_m5(0x00, data);
        }

        /* CRAM modified during HBLANK (Striker, Zero the Kamikaze, etc) */
        if ((v_counter < bitmap.viewport.h) && (reg[1]& 0x40) && (mcycles_68k <= (mcycles_vdp + 860)))
        {
          /* Remap current line */
          remap_line(v_counter);
        }
      }
      break;
    }

    case 0x05:  /* VSRAM */
    {
#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] VSRAM 0x%x write -> 0x%x (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, addr, data, m68k_get_reg(M68K_REG_PC));
#endif
      *(uint16 *)&vsram[addr & 0x7E] = data;

      /* 2-cell Vscroll mode */
      if (reg[11] & 0x04)
      {
        /* VSRAM writes during HBLANK (Adventures of Batman & Robin) */
        if ((v_counter < bitmap.viewport.h) && (reg[1]& 0x40) && (mcycles_68k <= (mcycles_vdp + 860)))
        {
          /* Remap current line */
          render_line(v_counter);
        }
      }
      break;
    }

#ifdef LOGERROR
    default:
    {
      error("[%d(%d)][%d(%d)] Unknown (%d) 0x%x write -> 0x%x (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, code, addr, data, m68k_get_reg(M68K_REG_PC));
      break;
    }
#endif
  }

  /* Increment address register (TODO: see how address is incremented in Mode 4) */
  addr += reg[15];
}


/*--------------------------------------------------------------------------*/
/* 68k data port access functions (Genesis mode)                            */
/*--------------------------------------------------------------------------*/

static void vdp_68k_data_w_m4(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  /* Restricted VDP writes during active display */
  if (!(status & 8) && (reg[1] & 0x40))
  {
    /* Update VDP FIFO */
    vdp_fifo_update(mcycles_68k);

    /* Clear FIFO empty flag */
    status &= 0xFDFF;

    /* 4 words can be stored */
    if (fifo_write_cnt < 4)
    {
      /* Increment FIFO counter */
      fifo_write_cnt++;

      /* Set FIFO full flag if 4 words are stored */
      status |= ((fifo_write_cnt & 4) << 6);
    }
    else
    {
      /* CPU is locked until last FIFO entry has been processed (Chaos Engine, Soldiers of Fortune, Double Clutch) */
      fifo_lastwrite += fifo_latency;
      mcycles_68k = fifo_lastwrite;
    }
  }

  /* Check destination code */
  if (code & 0x02)
  {
    /* CRAM index (32 words) */
    int index = addr & 0x1F;

    /* Pointer to CRAM 9-bit word */
    uint16 *p = (uint16 *)&cram[index << 1];

    /* Pack 16-bit data (xxx000BBGGRR) to 9-bit CRAM data (xxxBBGGRR) */
    data = ((data & 0xE00) >> 3) | (data & 0x3F);

    /* Check if CRAM data is being modified */
    if (data != *p)
    {
      /* Write CRAM data */
      *p = data;

      /* Update color palette */
      color_update_m4(index, data);

      /* Update backdrop color */
      if (index == (0x10 | (border & 0x0F)))
      {
        color_update_m4(0x40, data);
      }
    }
  }
  else
  {
    /* Byte-swap data if A0 is set */
    if (addr & 1)
    {
      data = ((data >> 8) | (data << 8)) & 0xFFFF;
    }

    /* VRAM address (interleaved format) */
    int index = ((addr << 1) & 0x3FC) | ((addr & 0x200) >> 8) | (addr & 0x3C00);

    /* Pointer to VRAM */
    uint16 *p = (uint16 *)&vram[index];

    /* Only write unique data to VRAM */
    if (data != *p)
    {
      /* Write data to VRAM */
      *p = data;

      /* Update the pattern cache */
      int name;
      MARK_BG_DIRTY (index);
    }
  }

  /* Increment address register */
  addr += (reg[15] + 1);
}

static void vdp_68k_data_w_m5(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  /* Restricted VDP writes during active display */
  if (!(status & 8) && (reg[1] & 0x40))
  {
    /* Update VDP FIFO */
    vdp_fifo_update(mcycles_68k);

    /* Clear FIFO empty flag */
    status &= 0xFDFF;

    /* 4 words can be stored */
    if (fifo_write_cnt < 4)
    {
      /* Increment FIFO counter */
      fifo_write_cnt++;

      /* Set FIFO full flag if 4 words are stored */
      status |= ((fifo_write_cnt & 4) << 6);
    }
    else
    {
      /* CPU is locked until last FIFO entry has been processed (Chaos Engine, Soldiers of Fortune, Double Clutch) */
      fifo_lastwrite += fifo_latency;
      mcycles_68k = fifo_lastwrite;
    }
  }
  
  /* Write data */
  vdp_bus_w(data);

  /* DMA Fill */
  if (dmafill & 0x100)
  {
    /* Fill data (DMA fill flag is cleared) */
    dmafill = data >> 8;

    /* DMA length */
    dma_length = (reg[20] << 8) | reg[19];

    /* Zero DMA length */
    if (!dma_length)
    {
      dma_length = 0x10000;
    }

    /* Perform DMA Fill*/
    dma_type = 2;
    vdp_dma_update(mcycles_68k);
  }
}

static unsigned int vdp_68k_data_r_m4(void)
{
  /* Clear pending flag */
  pending = 0;

  /* VRAM address (interleaved format) */
  int index = ((addr << 1) & 0x3FC) | ((addr & 0x200) >> 8) | (addr & 0x3C00);

  /* Increment address register */
  addr += (reg[15] + 1);

  /* Read VRAM data */
  return *(uint16 *) &vram[index];
}

static unsigned int vdp_68k_data_r_m5(void)
{
  uint16 data = 0;

  /* Clear pending flag */
  pending = 0;

  switch (code & 0x0F)
  {
    case 0x00: /* VRAM */
    {
      /* Read data */
      data = *(uint16 *)&vram[addr & 0xFFFE];

#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] VRAM 0x%x read -> 0x%x (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, addr, data, m68k_get_reg(M68K_REG_PC));
#endif
      break;
    }

    case 0x04: /* VSRAM */
    {
      /* Read data */
      data = *(uint16 *)&vsram[addr & 0x7E];

#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] VSRAM 0x%x read -> 0x%x (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, addr, data, m68k_get_reg(M68K_REG_PC));
#endif
      break;
    }

    case 0x08: /* CRAM */
    {
      /* Read data */
      data = *(uint16 *)&cram[addr & 0x7E];

      /* Unpack 9-bit CRAM data (BBBGGGRRR) to 16-bit bus data (BBB0GGG0RRR0) */
      data = ((data & 0x1C0) << 3) | ((data & 0x038) << 2) | ((data & 0x007) << 1);

#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] CRAM 0x%x read -> 0x%x (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, addr, data, m68k_get_reg(M68K_REG_PC));
#endif
      break;
    }

    default:
    {
      /* Invalid code value */
#ifdef LOGERROR
      error("[%d(%d)][%d(%d)] Invalid (%d) 0x%x read (%x)\n", v_counter, mcycles_68k/MCYCLES_PER_LINE-1, mcycles_68k, mcycles_68k%MCYCLES_PER_LINE, code, addr, m68k_get_reg(M68K_REG_PC));
#endif
      break;
    }
  }

  /* Increment address register */
  addr += reg[15];

  /* Return data */
  return data;
}


/*--------------------------------------------------------------------------*/
/* Z80 data port access functions (Master System compatibilty mode)         */
/*--------------------------------------------------------------------------*/

static void vdp_z80_data_w_m4(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  /* Check destination code */
  if (code & 0x02)
  {
    /* CRAM index (32 words) */
    int index = addr & 0x1F;

    /* Pointer to CRAM word */
    uint16 *p = (uint16 *)&cram[index << 1];

    /* Check if CRAM data is being modified */
    if (data != *p)
    {
      /* Write CRAM data */
      *p = data;

      /* Update color palette */
      color_update_m4(index, data);

      /* Update backdrop color */
      if (index == (0x10 | (border & 0x0F)))
      {
        color_update_m4(0x40, data);
      }
    }
  }
  else
  {
    /* VRAM address */
    int index = addr & 0x3FFF;

    /* Only write unique data to VRAM */
    if (data != vram[index])
    {
      /* Write data */
      vram[index] = data;

      /* Update pattern cache */
      int name;
      MARK_BG_DIRTY(index);
    }
  }

  /* Increment address register  */
  addr += (reg[15] + 1);
}

static void vdp_z80_data_w_m5(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  /* Check destination code */
  switch (code & 0x0F)
  {
    case 0x01:  /* VRAM */
    {
      /* VRAM address (write low byte to even address & high byte to odd address) */
      int index = addr ^ 1;

      /* Intercept writes to Sprite Attribute Table */
      if ((index & sat_base_mask) == satb)
      {
        /* Update internal SAT */
        WRITE_BYTE(sat, index & sat_addr_mask, data);
      }

      /* Only write unique data to VRAM */
      if (data != READ_BYTE(vram, index))
      {
        /* Write data */
        WRITE_BYTE(vram, index, data);

        /* Update pattern cache */
        int name;
        MARK_BG_DIRTY (index);
      }
      break;
    }

    case 0x03:  /* CRAM */
    {
      /* Pointer to CRAM word */
      uint16 *p = (uint16 *)&cram[addr & 0x7E];

      /* Pack 8-bit value into 9-bit CRAM data */
      if (addr & 1)
      {
        /* Write high byte (0000BBB0 -> BBBxxxxxx) */
        data = (*p & 0x3F) | ((data & 0x0E) << 5);
      }
      else
      {
        /* Write low byte (GGG0RRR0 -> xxxGGGRRR) */
        data = (*p & 0x1C0) | ((data & 0x0E) >> 1)| ((data & 0xE0) >> 2);
      }

      /* Check if CRAM data is being modified */
      if (data != *p)
      {
        /* Write CRAM data */
        *p = data;

        /* CRAM index (64 words) */
        int index = (addr >> 1) & 0x3F;

        /* Color entry 0 of each palette is never displayed (transparent pixel) */
        if (index & 0x0F)
        {
          /* Update color palette */
          color_update_m5(index, data);
        }

        /* Update backdrop color */
        if (index == border)
        {
          color_update_m5(0x00, data);
        }
      }
      break;
    }

    case 0x05: /* VSRAM */
    {
      /* Write low byte to even address & high byte to odd address */
      WRITE_BYTE(vsram, (addr & 0x7F) ^ 1, data);
      break;
    }
  }

  /* Increment address register  */
  addr += reg[15];

  /* DMA Fill */
  if (dmafill & 0x100)
  {
    /* Fill data (DMA fill flag is cleared) */
    dmafill = data;

    /* DMA length */
    dma_length = (reg[20] << 8) | reg[19];

    /* Zero DMA length */
    if (!dma_length)
    {
      dma_length = 0x10000;
    }

    /* Perform DMA Fill */
    dma_type = 2;
    vdp_dma_update(mcycles_z80);
  }
}

static unsigned int vdp_z80_data_r_m4(void)
{
  /* Clear pending flag */
  pending = 0;

  /* Read buffer */
  unsigned int data = fifo[0];

  /* Process next read */
  fifo[0] = vram[addr & 0x3FFF];

  /* Increment address register (register #15 can only be set in Mode 5) */
  addr += (reg[15] + 1);

  /* Return data */
  return data;
}

static unsigned int vdp_z80_data_r_m5(void)
{
  unsigned int data = 0;

  /* Clear pending flag */
  pending = 0;

  switch (code & 0x0F)
  {
    case 0x00: /* VRAM */
    {
      /* Return low byte from even address & high byte from odd address */
      data = READ_BYTE(vram, addr ^ 1);
      break;
    }

    case 0x04: /* VSRAM */
    {
      /* Return low byte from even address & high byte from odd address */
      data = READ_BYTE(vsram, (addr & 0x7F) ^ 1);
      break;
    }

    case 0x08: /* CRAM */
    {
      /* Read CRAM data */
      data = *(uint16 *)&cram[addr & 0x7E];

      /* Unpack 9-bit CRAM data (BBBGGGRRR) to 16-bit data (BBB0GGG0RRR0) */
      data = ((data & 0x1C0) << 3) | ((data & 0x038) << 2) | ((data & 0x007) << 1);

      /* Return low byte from even address & high byte from odd address */
      if (addr & 1)
      {
        data = data >> 8;
      }

      data &= 0xFF;
      break;
    }
  }

  /* Increment address register */
  addr += reg[15];

  /* Return data */
  return data;
}


/*-----------------------------------------------------------------------------*/
/* VDP specific data port access functions (Master System, Game Gear, SG-1000) */
/*-----------------------------------------------------------------------------*/

static void vdp_z80_data_w_ms(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  if (code < 3)
  {
    /* check if we are already on next line */
    int line = (lines_per_frame + (mcycles_z80 / MCYCLES_PER_LINE) - 1) % lines_per_frame;
    if ((line > v_counter) && (line < bitmap.viewport.h) && !(work_ram[0x1ffb] & cart.special))
    {
      v_counter = line;
      render_line(line);
    }

    /* VRAM address */
    int index = addr & 0x3FFF;

#ifdef LOGVDP
    error("[%d(%d)][%d(%d)] VRAM 0x%x write -> 0x%x (%x)\n", v_counter, mcycles_z80/MCYCLES_PER_LINE-1, mcycles_z80, mcycles_z80%MCYCLES_PER_LINE, index, data, Z80.pc.w.l);
#endif

    /* VRAM write */
    if(data != vram[index])
    {
      int name;
      vram[index] = data;
      MARK_BG_DIRTY(index);
    }
  }
  else
  {
#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] CRAM 0x%x write -> 0x%x (%x)\n", v_counter, mcycles_z80/MCYCLES_PER_LINE-1, mcycles_z80, mcycles_z80%MCYCLES_PER_LINE, addr, data, Z80.pc.w.l);
#endif
    /* CRAM address */
    int index = addr & 0x1F;

    /* Pointer to CRAM word */
    uint16 *p = (uint16 *)&cram[index << 1];

    /* Check if CRAM data is being modified */
    if (data != *p)
    {
      /* Write CRAM data */
      *p = data;

      /* Update color palette */
      color_update_m4(index, data);

      /* Update backdrop color */
      if (index == (0x10 | (border & 0x0F)))
      {
        color_update_m4(0x40, data);
      }
    }
  }

  /* Update read buffer */
  fifo[0] = data;

  /* Update address register */
  addr++;
}

static void vdp_z80_data_w_gg(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  if (code < 3)
  {
    /* check if we are already on next line*/
    int line = (lines_per_frame + (mcycles_z80 / MCYCLES_PER_LINE) - 1) % lines_per_frame;
    if ((line > v_counter) && (line < bitmap.viewport.h) && !(work_ram[0x1ffb] & cart.special))
    {
      v_counter = line;
      render_line(line);
    }

    /* VRAM address */
    int index = addr & 0x3FFF;

#ifdef LOGVDP
      error("[%d(%d)][%d(%d)] VRAM 0x%x write -> 0x%x (%x)\n", v_counter, mcycles_z80/MCYCLES_PER_LINE-1, mcycles_z80, mcycles_z80%MCYCLES_PER_LINE, index, data, Z80.pc.w.l);
#endif

    /* VRAM write */
    if(data != vram[index])
    {
      int name;
      vram[index] = data;
      MARK_BG_DIRTY(index);
    }
  }
  else
  {
    if(addr & 1)
    {
      /* 12-bit data word */
      data = (data << 8) | cached_write;

      /* Pointer to CRAM word */
      uint16 *p = (uint16 *)&cram[addr & 0x3E];

      /* Check if CRAM data is being modified */
      if (data != *p)
      {
        /* Write CRAM data */
        *p = data;

        /* Color index (0-31) */
        int index = (addr >> 1) & 0x1F;

        /* Update color palette */
        color_update_m4(index, data);

        /* Update backdrop color */
        if (index == (0x10 | (border & 0x0F)))
        {
          color_update_m4(0x40, data);
        }
      }
    }
    else
    {
      /* Latch LSB */
      cached_write = data;
    }
  }

  /* Update read buffer */
  fifo[0] = data;

  /* Update address register */
  addr++;
}

static void vdp_z80_data_w_sg(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  /* VRAM address */
  int index = addr & 0x3FFF;

  /* 4K address decoding (cf. tms9918a.txt) */
  if (!(reg[1] & 0x80))
  {
    index = (index & 0x203F) | ((index >> 6) & 0x40) | ((index << 1) & 0x1F80);
  }

  /* VRAM write */
  vram[index] = data;

  /* Update address register */
  addr++;
}

/*--------------------------------------------------------------------------*/
/* DMA operations                                                           */
/*--------------------------------------------------------------------------*/

/* 68K bus to VRAM, VSRAM or CRAM */
static void vdp_dma_vbus(int length)
{
  unsigned int data;
  unsigned int source = (reg[23] << 17 | reg[22] << 9 | reg[21] << 1) & 0xFFFFFE;
  unsigned int base = source;

  /* DMA source */
  if ((source >> 17) == 0x50)
  {
    /* Z80 & I/O area ($A00000-$A1FFFF) */
    do
    {
      /* Return $FFFF only when the Z80 isn't hogging the Z-bus.
        (e.g. Z80 isn't reset and 68000 has the bus) */
      if (source <= 0xA0FFFF)
      {
        data = ((zstate ^ 3) ? *(uint16 *)(work_ram + (source & 0xFFFF)) : 0xFFFF);
      }

      /* The I/O chip and work RAM try to drive the data bus which results 
          in both values being combined in random ways when read.
          We return the I/O chip values which seem to have precedence, */
      else if (source <= 0xA1001F)
      {
        data = io_68k_read((source >> 1) & 0x0F);
        data = (data << 8 | data);
      }

      /* All remaining locations access work RAM */
      else
      {
        data = *(uint16 *)(work_ram + (source & 0xFFFF));
      }

      /* Increment source address */
      source += 2;

      /* 128k DMA window (i.e reg #23 is not modified) */
      source = ((base & 0xFE0000) | (source & 0x1FFFF));

      /* Write data on internal bus */
      vdp_bus_w(data);
    }
    while (--length);
  }
  else
  {
    do
    {
      /* Read from mapped memory (ROM/RAM) */
      data = *(uint16 *)(m68k_memory_map[source>>16].base + (source & 0xFFFF));

      /* Increment source address */
      source += 2;

      /* 128k DMA window (i.e reg #23 is not modified) */
      source = ((base & 0xFE0000) | (source & 0x1FFFF));

      /* Write data on internal bus */
      vdp_bus_w(data);
    }
    while (--length);
  }

  /* Update source address registers (reg #23 has not been modified) */
  reg[21] = (source >> 1) & 0xFF;
  reg[22] = (source >> 9) & 0xFF;
}

/*  VRAM Copy (TODO: check if CRAM or VSRAM copy is possible) */
static void vdp_dma_copy(int length)
{
  int name;
  unsigned int temp;
  unsigned int source = (reg[22] << 8) | reg[21];

  do
  {
    /* Read byte from source address */
    temp = READ_BYTE(vram, source);

    /* Intercept writes to Sprite Attribute Table */
    if ((addr & sat_base_mask) == satb)
    {
      /* Update internal SAT */
      WRITE_BYTE(sat, addr & sat_addr_mask, temp);
    }

    /* Write byte to VRAM address */
    WRITE_BYTE(vram, addr, temp);

    /* Update pattern cache */
    MARK_BG_DIRTY(addr);

    /* Increment source address */
    source = (source + 1) & 0xFFFF;

    /* Increment VRAM address */
    addr += reg[15];
  }
  while (--length);

  /* Update source address registers */
  reg[21] = source & 0xFF;
  reg[22] = (source >> 8) & 0xFF; 
}

/* VRAM Fill (TODO: check if CRAM or VSRAM fill is possible) */
static void vdp_dma_fill(unsigned int data, int length)
{
  int name;

  do
  {
    /* Intercept writes to Sprite Attribute Table */
    if ((addr & sat_base_mask) == satb)
    {
      /* Update internal SAT */
      WRITE_BYTE(sat, (addr & sat_addr_mask) ^ 1, data);
    }

    /* Write byte to adjacent VRAM address */
    WRITE_BYTE(vram, addr ^ 1, data);

    /* Update pattern cache */
    MARK_BG_DIRTY (addr);

    /* Increment VRAM address */
    addr += reg[15];
  }
  while (--length);
}

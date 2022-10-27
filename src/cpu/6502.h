//
//  6502.h
//  6502
//
//  Created by Tamas Rudnai on 7/22/19.
//  Copyright © 2019, 2020 Tamas Rudnai. All rights reserved.
//
// This file is part of Steve ][ -- The Apple ][ Emulator.
//
// Steve ][ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Steve ][ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Steve ][.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __6502_H__
#define __6502_H__

#include <stdint.h>
#include "common.h"
#include "woz.h"


#ifdef DEBUGGER
#define INSTR INLINE static
#else
#define INSTR INLINE static
#endif

#define CRYSTAL_MHZ 14.31818                // NTSC version (original)
#define DEFAULT_MHZ_6502 (CRYSTAL_MHZ / 14) // 1.023 MHz


typedef enum cpuMode_e {
    cpuMode_normal = 0,
    cpuMode_eco,
    cpuMode_game,
} cpuMode_s;

typedef enum cpuState_e {
    cpuState_unknown = 0,
    cpuState_inited,
    cpuState_running,
    cpuState_executing,
    cpuState_halting,
    cpuState_halted,
} cpuState_s;

extern volatile cpuMode_s cpuMode;
extern volatile cpuState_s cpuState;

extern const double default_MHz_6502;
extern const double iigs_MHz_6502;
extern const double iicplus_MHz_6502;
extern double MHz_6502;
extern unsigned int clk_6502_per_frm;
extern unsigned int clk_6502_per_frm_set;
extern unsigned int clk_6502_per_frm_max;
extern unsigned int clk_6502_per_frm_max_sound;


typedef enum {
    NO_INT,
    HALT,
    IRQ,
    NMI,
    HARDRESET,
    SOFTRESET,
} interrupt_t;


typedef struct debugLevel_s {
    uint8_t trace       : 1;
    uint8_t step        : 1;
    uint8_t brk         : 1;
    uint8_t rts         : 1;
    uint8_t bra         : 1;
    uint8_t bra_true    : 1;
    uint8_t bra_false   : 1;
    uint8_t compile     : 1;
} debugLevel_t;


typedef union flags_u {
    struct {
        uint8_t C:1;    // Carry Flag
        uint8_t Z:1;    // Zero Flag
        uint8_t I:1;    // Interrupt Flag
        uint8_t D:1;    // Decimal Flag
        uint8_t B:1;    // B Flag
        uint8_t res:1;  // reserved -- should be always 1
        uint8_t V:1;    // Overflow Flag ???
        uint8_t N:1;    // Negative Flag
    };
    
    uint8_t SR;
} flags_t;


//#pragma pack(1)
typedef struct m6502_s {
    uint8_t  A;       //  0: Accumulator
    uint8_t  X;       //  1: X index register
    uint8_t  Y;       //  2: Y index register
//    union {
//        uint8_t  instr;         // Instruction
//        struct {
//            uint8_t cc:2;
//            uint8_t bbb:3;
//            uint8_t aaa:3;
//        };
//    };
    struct { // no bitfield faster processing
        uint8_t C;    //  3: Carry Flag
        uint8_t Z;    //  4: Zero Flag
        uint8_t I;    //  5: Interrupt Flag
        uint8_t D;    //  6: Decimal Flag
        uint8_t B;    //  7: B Flag
        uint8_t res;  //  8: reserved -- should be always 1
        uint8_t V;    //  9: Overflow Flag ???
        uint8_t N;    // 10: Negative Flag
    };
    
    uint16_t PC;      // 11: Program Counter
    uint8_t SP;       // 13: Stack Pointer ( stack addr = 0x01 + sp )

//    unsigned clk;           // Clock Counter
    uint64_t clktime; // 14:
    uint64_t clklast; // 22:
    uint32_t clkfrm;  // 30:
    
    uint64_t clk_wrenable;  // CPU clock when WRITE RAM is triggered
    
    uint64_t lastIO;  // Last time I/O accessed
    int ecoSpindown;  // spindown counter for eco mode

    debugLevel_t dbgLevel;  // 34:  0: No Debug, 1: Disassembly Only, 2: Run till BRK, 3: StepByStep
    
    union {
        unsigned int IF;             // interrut flag
        interrupt_t interrupt;
    };
    
} m6502_t;
//#pragma pack()

extern const int ecoSpindown;   // initial value of ECO Spingdown Counter


// Memory Config
typedef struct MEMcfg_s {
    unsigned RAM_16K     : 1;
    unsigned RAM_128K    : 1;
    unsigned RD_INT_RAM  : 1;
    unsigned WR_RAM      : 1;
    unsigned RAM_BANK_2  : 1;
    unsigned AUX_BANK    : 1;
    unsigned txt_page_2  : 1;    // 0: page 1   1: page 2 (aux video memory)
    unsigned int_Cx_ROM  : 1;    // 0: Slot Cx ROM   1: Internal Cx ROM
    unsigned slot_C3_ROM : 1;    // 0: Internal ROM  1: Slot 3 ROM
    unsigned is_80STORE  : 1;
    unsigned RD_AUX_MEM  : 1;
    unsigned WR_AUX_MEM  : 1;
    unsigned ALT_ZP      : 1;
    
    unsigned WR_RAM_cntr;       // min 2 I/O to enable mem write
} MEMcfg_t;


typedef union videoMode_u {
    struct {
        uint8_t text    : 1;    // 0: graphics 1: text
        uint8_t col80   : 1;    // 0: 40 col   1: 80 col
        uint8_t altChr  : 1;    // 0: normal   1: alternate character set
        uint8_t mixed   : 1;    // 0: no mix   1: mixed graphics and text when in graphics mode
        uint8_t hires   : 1;    // 0: loRes    1: hiRes
    };
    uint8_t mode;
} videoMode_t;

extern MEMcfg_t MEMcfg;

extern m6502_t m6502;
extern uint8_t * const AUX;         // Pointer to the auxiliary memory so we can use this from Swift
extern uint8_t * const RAM;
extern uint8_t * const MEM;         // Pointer to the Shadow Memory Map so we can use this from Swift//extern uint8_t * AUX_VID_RAM;
extern uint32_t * videoMemPtr;

extern double * pdl_valarr;
extern double * pdl_prevarr;
extern double * pdl_diffarr;

//extern void hires_Update(void);

extern double mips;
extern double mhz;

#define DEFAULT_FPS 60U
#define DEF_VIDEO_DIV 2U
#define DEF_SPKR_DIV 2U
#define DEF_DRV_LED_DIV 4U

#define ECO_VIDEO_DIV 4U

#define GAME_FPS 60U // 180U // 480U
#define GAME_VIDEO_DIV 1U // (GAME_FPS / DEFAULT_FPS)

extern unsigned int video_fps_divider;
extern unsigned int fps;

extern void rom_loadFile( const char * bundlePath, const char * filename );
extern void tst6502(void);
extern void m6502_ColdReset( const char * bundlePath, const char * romFilePath );
extern void m6502_Run(void);

extern void interrupt_IRQ(void);
extern void interrupt_NMI(void);
extern void hardReset(void);
extern void softReset(void);

extern void openLog(void);
extern void closeLog(void);



#endif /* __6502_H__ */

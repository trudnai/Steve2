//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
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

#ifndef __APPLE2_MMIO_H__
#define __APPLE2_MMIO_H__


#include <string.h>

#include "common.h"
#include "6502.h"
#include "disk.h"
#include "woz.h"
#include "speaker.h"
#include "paddle.h"
#include "6502_dis_utils.h"


typedef union address16_u {
    uint16_t addr;
    struct {
        uint8_t offs;
        uint8_t page;
    };
} address16_t;


extern videoMode_t videoMode; // 40 col text, page 1


extern uint8_t Apple2_Dummy_Page[ 1 * PG ];            // Dummy Page to discard data
extern uint8_t Apple2_Dummy_RAM[ 64 * KB ];            // Dummy RAM to discard data

extern uint8_t INT_64K_ROM[ 64 * KB ];        // ROM C0, C8, D0, D8, E0, E8, F0, F8
extern uint8_t EXP_64K_ROM[ 64 * KB ];        // ROM C0, C8, D0, D8, E0, E8, F0, F8

extern uint8_t Apple2_64K_AUX[ 64 * KB ];        // 64K Expansion Memory
extern uint8_t Apple2_64K_RAM[ 64 * KB ];        // Main Memory
extern uint8_t Apple2_64K_MEM[ 64 * KB ];        // Shadow Copy of Memory (or current memory content)

//uint8_t * AUX_VID_RAM = Apple2_VID_AUX;       // Pointer to Auxiliary Video Memory
extern uint8_t * const AUX;           // Pointer to the Auxiliary Memory so we can use this from Swift
extern uint8_t * const RAM;           // Pointer to the Main Memory so we can use this from Swift
extern uint8_t * const MEM;           // Pointer to the Shadow Memory Map so we can use this from Swift

extern uint8_t * const RDLOMEM;       // for Read $0000 - $BFFF (shadow memory)
extern uint8_t *       WRZEROPG;       // for Write $0000 - $0200 (shadow memory)
extern uint8_t *       WRLOMEM;       // for Write $0200 - $BFFF (shadow memory)
extern uint8_t * const RDHIMEM;       // for Read / Write $0000 - $BFFF (shadow memory)
extern uint8_t *       WRD0MEM;     // for writing $D000 - $DFFF
extern uint8_t *       WRHIMEM;     // for writing $E000 - $FFFF


extern uint8_t writeState;                  // 1 if $C08D was written


#define INIT_MEMCFG { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

extern const MEMcfg_t initMEMcfg;

extern MEMcfg_t MEMcfg;
extern MEMcfg_t newMEMcfg;

extern void kbdInput ( uint8_t code );
extern void kbdPaste ( uint8_t code );
extern void kbdUp (void);
extern uint8_t getIO ( uint16_t ioaddr );
extern void setIO ( uint16_t ioaddr, uint8_t val );

extern void save_RAM(void);

#ifndef DISASSEMBLER
extern uint8_t getMEM ( uint16_t ioaddr );
extern uint16_t getMEM16 ( uint16_t ioaddr );
extern uint32_t getMEM32 ( uint16_t ioaddr );

extern void setMEM ( uint16_t ioaddr, uint8_t val );
extern void setMEM16 ( uint16_t ioaddr, uint16_t val );
extern void setMEM32 ( uint16_t ioaddr, uint32_t val );
extern void setMEMarray ( uint16_t addr, uint8_t * arr, int len );

extern void CALL( uint16_t addr );
extern void JUMP( uint16_t addr );
#endif


#define DEF_RAM_PAGE(mem,pg) \
    (mem) + ((pg) << 8)

#define DEF_RAM_PAGE16(mem,pg) \
    DEF_RAM_PAGE(mem, (pg) + 0x00), \
    DEF_RAM_PAGE(mem, (pg) + 0x01), \
    DEF_RAM_PAGE(mem, (pg) + 0x02), \
    DEF_RAM_PAGE(mem, (pg) + 0x03), \
    DEF_RAM_PAGE(mem, (pg) + 0x04), \
    DEF_RAM_PAGE(mem, (pg) + 0x05), \
    DEF_RAM_PAGE(mem, (pg) + 0x06), \
    DEF_RAM_PAGE(mem, (pg) + 0x07), \
    DEF_RAM_PAGE(mem, (pg) + 0x08), \
    DEF_RAM_PAGE(mem, (pg) + 0x09), \
    DEF_RAM_PAGE(mem, (pg) + 0x0A), \
    DEF_RAM_PAGE(mem, (pg) + 0x0B), \
    DEF_RAM_PAGE(mem, (pg) + 0x0C), \
    DEF_RAM_PAGE(mem, (pg) + 0x0D), \
    DEF_RAM_PAGE(mem, (pg) + 0x0E), \
    DEF_RAM_PAGE(mem, (pg) + 0x0F)


#define SWITCH_VIDEO_RAM( tbl,tpg, mem,mpg ) \
    (tbl)[ (tpg) + 0x00 ] = DEF_RAM_PAGE(mem, (mpg) + 0x00); \
    (tbl)[ (tpg) + 0x01 ] = DEF_RAM_PAGE(mem, (mpg) + 0x01); \
    (tbl)[ (tpg) + 0x02 ] = DEF_RAM_PAGE(mem, (mpg) + 0x02); \
    (tbl)[ (tpg) + 0x03 ] = DEF_RAM_PAGE(mem, (mpg) + 0x03);


#define SWITCH_ROM_PAGE( tbl,tpg, mem,mpg ) \
    (tbl)[ (tpg) ] = DEF_RAM_PAGE( mem, (mpg) );


#define SWITCH_STACK_ZP( tbl, mem ) \
    (tbl)[ 0x00 ] = DEF_RAM_PAGE(mem, 0x00); \
    (tbl)[ 0x01 ] = DEF_RAM_PAGE(mem, 0x01);


#define SWITCH_CX_ROM( tbl,tpg, mem,mpg ) \
/*    (tbl)[ (tpg) + 0x00 ] = DEF_RAM_PAGE(mem, (mpg) + 0x00); */ \
    (tbl)[ (tpg) + 0x01 ] = DEF_RAM_PAGE(mem, (mpg) + 0x01); \
    (tbl)[ (tpg) + 0x02 ] = DEF_RAM_PAGE(mem, (mpg) + 0x02); \
    (tbl)[ (tpg) + 0x03 ] = DEF_RAM_PAGE(mem, (mpg) + 0x03); \
    (tbl)[ (tpg) + 0x04 ] = DEF_RAM_PAGE(mem, (mpg) + 0x04); \
    (tbl)[ (tpg) + 0x05 ] = DEF_RAM_PAGE(mem, (mpg) + 0x05); \
    (tbl)[ (tpg) + 0x06 ] = DEF_RAM_PAGE(mem, (mpg) + 0x06); \
    (tbl)[ (tpg) + 0x07 ] = DEF_RAM_PAGE(mem, (mpg) + 0x07); \
    (tbl)[ (tpg) + 0x08 ] = DEF_RAM_PAGE(mem, (mpg) + 0x08); \
    (tbl)[ (tpg) + 0x09 ] = DEF_RAM_PAGE(mem, (mpg) + 0x09); \
    (tbl)[ (tpg) + 0x0A ] = DEF_RAM_PAGE(mem, (mpg) + 0x0A); \
    (tbl)[ (tpg) + 0x0B ] = DEF_RAM_PAGE(mem, (mpg) + 0x0B); \
    (tbl)[ (tpg) + 0x0C ] = DEF_RAM_PAGE(mem, (mpg) + 0x0C); \
    (tbl)[ (tpg) + 0x0D ] = DEF_RAM_PAGE(mem, (mpg) + 0x0D); \
    (tbl)[ (tpg) + 0x0E ] = DEF_RAM_PAGE(mem, (mpg) + 0x0E); \
    (tbl)[ (tpg) + 0x0F ] = DEF_RAM_PAGE(mem, (mpg) + 0x0F);


#define SWITCH_RAM_PAGE16( tbl,tpg, mem,mpg ) \
    (tbl)[ (tpg) + 0x00 ] = DEF_RAM_PAGE(mem, (mpg) + 0x00); \
    (tbl)[ (tpg) + 0x01 ] = DEF_RAM_PAGE(mem, (mpg) + 0x01); \
    (tbl)[ (tpg) + 0x02 ] = DEF_RAM_PAGE(mem, (mpg) + 0x02); \
    (tbl)[ (tpg) + 0x03 ] = DEF_RAM_PAGE(mem, (mpg) + 0x03); \
    (tbl)[ (tpg) + 0x04 ] = DEF_RAM_PAGE(mem, (mpg) + 0x04); \
    (tbl)[ (tpg) + 0x05 ] = DEF_RAM_PAGE(mem, (mpg) + 0x05); \
    (tbl)[ (tpg) + 0x06 ] = DEF_RAM_PAGE(mem, (mpg) + 0x06); \
    (tbl)[ (tpg) + 0x07 ] = DEF_RAM_PAGE(mem, (mpg) + 0x07); \
    (tbl)[ (tpg) + 0x08 ] = DEF_RAM_PAGE(mem, (mpg) + 0x08); \
    (tbl)[ (tpg) + 0x09 ] = DEF_RAM_PAGE(mem, (mpg) + 0x09); \
    (tbl)[ (tpg) + 0x0A ] = DEF_RAM_PAGE(mem, (mpg) + 0x0A); \
    (tbl)[ (tpg) + 0x0B ] = DEF_RAM_PAGE(mem, (mpg) + 0x0B); \
    (tbl)[ (tpg) + 0x0C ] = DEF_RAM_PAGE(mem, (mpg) + 0x0C); \
    (tbl)[ (tpg) + 0x0D ] = DEF_RAM_PAGE(mem, (mpg) + 0x0D); \
    (tbl)[ (tpg) + 0x0E ] = DEF_RAM_PAGE(mem, (mpg) + 0x0E); \
    (tbl)[ (tpg) + 0x0F ] = DEF_RAM_PAGE(mem, (mpg) + 0x0F);


#define SWITCH_AUX_MEM( tbl, mem ) \
    (tbl)[ 0x02 ] = DEF_RAM_PAGE(mem, 0x02); \
    (tbl)[ 0x03 ] = DEF_RAM_PAGE(mem, 0x03); \
    (tbl)[ 0x04 ] = DEF_RAM_PAGE(mem, 0x04); \
    (tbl)[ 0x05 ] = DEF_RAM_PAGE(mem, 0x05); \
    (tbl)[ 0x06 ] = DEF_RAM_PAGE(mem, 0x06); \
    (tbl)[ 0x07 ] = DEF_RAM_PAGE(mem, 0x07); \
    (tbl)[ 0x08 ] = DEF_RAM_PAGE(mem, 0x08); \
    (tbl)[ 0x09 ] = DEF_RAM_PAGE(mem, 0x09); \
    (tbl)[ 0x0A ] = DEF_RAM_PAGE(mem, 0x0A); \
    (tbl)[ 0x0B ] = DEF_RAM_PAGE(mem, 0x0B); \
    (tbl)[ 0x0B ] = DEF_RAM_PAGE(mem, 0x0C); \
    (tbl)[ 0x0B ] = DEF_RAM_PAGE(mem, 0x0D); \
    (tbl)[ 0x0B ] = DEF_RAM_PAGE(mem, 0x0E); \
    (tbl)[ 0x0B ] = DEF_RAM_PAGE(mem, 0x0F); \
    SWITCH_RAM_PAGE16( tbl, 0x10, mem, 0x10 ); \
    SWITCH_RAM_PAGE16( tbl, 0x20, mem, 0x20 ); \
    SWITCH_RAM_PAGE16( tbl, 0x30, mem, 0x30 ); \
    SWITCH_RAM_PAGE16( tbl, 0x40, mem, 0x40 ); \
    SWITCH_RAM_PAGE16( tbl, 0x50, mem, 0x50 ); \
    SWITCH_RAM_PAGE16( tbl, 0x60, mem, 0x60 ); \
    SWITCH_RAM_PAGE16( tbl, 0x70, mem, 0x70 ); \
    SWITCH_RAM_PAGE16( tbl, 0x80, mem, 0x80 ); \
    SWITCH_RAM_PAGE16( tbl, 0x90, mem, 0x90 ); \
    SWITCH_RAM_PAGE16( tbl, 0xA0, mem, 0xA0 ); \
    SWITCH_RAM_PAGE16( tbl, 0xB0, mem, 0xB0 );


#define DEF_RAM_DUMMY16 \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page, \
    Apple2_Dummy_Page

#define DEF_RAM_NULL16 \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL


enum slot {
    SLOT0   = 0x00,
    SLOT1   = 0x10,
    SLOT2   = 0x20,
    SLOT3   = 0x30,
    SLOT4   = 0x40,
    SLOT5   = 0x50,
    SLOT6   = 0x60,
    SLOT7   = 0x70,
};


// https://www.kreativekorp.com/miscpages/a2info/iomemory.shtml
// Comp:  O = Apple II+  E = Apple IIe  C = Apple IIc  G = Apple IIgs
// Act:   R = Read       W = Write      7 = Bit 7      V = Value

enum mmio {
    // Keyboard
    io_KBD              = 0xC000,   // OECG  R   Last Key Pressed + 128
    io_KBDSTRB          = 0xC010,   // OECG WR   Keyboard Strobe

    // Audio
    io_TAPEOUT          = 0xC020,   // OE    R   Toggle Cassette Tape Output (not on IIe PDS Card)
    io_SPKR             = 0xC030,   // OECG  R   Toggle Speaker
    
    // Video
    io_80STOREOFF       = 0xC000,   //  ECG W    Use $C002-$C005 for Aux Memory
    io_80STOREON        = 0xC001,   //  ECG W    Use PAGE2 for Aux Memory
    
    io_RD80STORE        = 0xC018,   //  ECG  R7  Status of $C002-$C005/PAGE2 for Aux Mem
    io_VID_CLR80VID     = 0xC00C,   //  ECG W    40 Columns
    io_VID_SET80VID     = 0xC00D,   //  ECG W    80 Columns
    io_VID_CLRALTCHAR   = 0xC00E,   //  ECG W    Primary Character Set
    io_VID_SETALTCHAR   = 0xC00F,   //  ECG W    Alternate Character Set
    io_VID_RDVBL        = 0xC019,   //  E G  R7  Vertical Blanking (E:1=drawing G:0=drawing) VSYNC
                       // RSTVBL         C   R   Reset Vertical Blanking Interrupt
    io_VID_RDTEXT       = 0xC01A,   //  ECG  R7  Status of Text/Graphics
    io_VID_RDMIXED      = 0xC01B,   //  ECG  R7  Status of Full Screen/Mixed Graphics
    io_VID_RDPAGE2      = 0xC01C,   //  ECG  R7  Status of Page 1/Page 2
    io_VID_RDHIRES      = 0xC01D,   //  ECG  R7  Status of LoRes/HiRes
    io_VID_ALTCHAR      = 0xC01E,   //  ECG  R7  Status of Primary/Alternate Character Set
    io_VID_RD80VID      = 0xC01F,   //  ECG  R7  Status of 40/80 Columns
    io_VID_Text_OFF     = 0xC050,   // TODO: floating I/O --> actual character / byte displayed by video?
    io_VID_Text_ON      = 0xC051,   // TODO: floating I/O --> actual character / byte displayed by video?
    io_VID_Mixed_OFF    = 0xC052,   // TODO: floating I/O --> actual character / byte displayed by video?
    io_VID_Mixed_ON     = 0xC053,   // TODO: floating I/O --> actual character / byte displayed by video?
    io_VID_TXTPAGE1     = 0xC054,   // OECG WR   Display Page 1
    io_VID_TXTPAGE2     = 0xC055,   // OECG WR   If 80STORE Off: Display Page 2
                                    //  ECG WR   If 80STORE On: Read/Write Aux Display Mem
    io_VID_Hires_OFF    = 0xC056,
    io_VID_Hires_ON     = 0xC057,
    
    io_TAPEIN           = 0xC060,   // OE    R7  Read Cassette Input
                                    //   C   R7  Status of 80/40 Column Switch

    // Game Controller
    io_STROBE           = 0xC040,
    io_CLRAN0           = 0xC058,
    io_SETAN0           = 0xC059,
    io_CLRAN1           = 0xC05A,
    io_SETAN1           = 0xC05B,
    io_CLRAN2           = 0xC05C,
    io_SETAN2           = 0xC05D,
    io_CLRAN3           = 0xC05E,
    io_SETAN3           = 0xC05F,

    io_PDL0             = 0xC064,
    io_PDL1             = 0xC065,
    io_PDL2             = 0xC066,
    io_PDL3             = 0xC067,
    io_PDL_STROBE       = 0xC070,

    // Disk ][
    io_DISK_PHASE0_OFF  = 0xC080,
    io_DISK_PHASE0_ON   = 0xC081,
    io_DISK_PHASE1_OFF  = 0xC082,
    io_DISK_PHASE1_ON   = 0xC083,
    io_DISK_PHASE2_OFF  = 0xC084,
    io_DISK_PHASE2_ON   = 0xC085,
    io_DISK_PHASE3_OFF  = 0xC086,
    io_DISK_PHASE3_ON   = 0xC087,
    io_DISK_POWER_OFF   = 0xC088,
    io_DISK_POWER_ON    = 0xC089,
    io_DISK_SELECT_1    = 0xC08A,
    io_DISK_SELECT_2    = 0xC08B,
    io_DISK_READ        = 0xC08C,
    io_DISK_WRITE       = 0xC08D,
    io_DISK_CLEAR       = 0xC08E,
    io_DISK_SHIFT       = 0xC08F,

    // Memory
    io_RDMAINRAM        = 0xC002,   //  ECG W    If 80STORE Off: Read Main Mem $0200-$BFFF
    io_RDCARDRAM        = 0xC003,   //  ECG W    If 80STORE Off: Read Aux Mem $0200-$BFFF
    io_WRMAINRAM        = 0xC004,   //  ECG W    If 80STORE Off: Write Main Mem $0200-$BFFF
    io_WRCARDRAM        = 0xC005,   //  ECG W    If 80STORE Off: Write Aux Mem $0200-$BFFF
    io_SETSLOTCXROM     = 0xC006,   //  E G W    Disable Internal ROM / Enable Peripheral ROM ($C100-$CFFF)
    io_SETINTCXROM      = 0xC007,   //  E G W    Enable Internal ROM ($C100-$CFFF)
    io_SETSTDZP         = 0xC008,   //  ECG W    Main Stack and Zero Page
    io_SETALTZP         = 0xC009,   //  ECG W    Aux Stack and Zero Page
    io_SETINTC3ROM      = 0xC00A,   //  E G W    ROM in Slot 3
    io_SETSLOTC3ROM     = 0xC00B,   //  E G W    ROM in Aux Slot
    
    io_RDLCBNK2         = 0xC011,   //  ECG  R7  Status of Selected $Dx Bank
    io_RDLCRAM          = 0xC012,   //  ECG  R7  Status of $Dx ROM / $Dx RAM (LC RAM)
    io_RDRAMRD          = 0xC013,   //  ECG  R7  Status of Main/Aux RAM Reading (auxilliary 48K)
    io_RDRAMWR          = 0xC014,   //  ECG  R7  Status of Main/Aux RAM Writing (auxilliary 48K)
    
    io_RDCXROM          = 0xC015,   //  E G  R7  Read state of $C100-$CFFF soft switch -- Status of Periph/ROM Access
    io_RSTXINT          = 0xC015,   //   C   R   Reset Mouse X0 Interrupt
    io_RDALTZP          = 0xC016,   //  ECG  R7  Status of Main/Aux Stack and Zero Page
    io_RDC3ROM          = 0xC017,   //  E G  R7  Status of Slot 3/Aux Slot ROM
    io_RSTYINT          = 0xC017,   //   C   R   Reset Mouse Y0 Interrupt
    
    io_MEM_RDRAM_NOWR_2 = 0xC080,   //  OECG WR   Read RAM bank 2; no write
    io_MEM_RDROM_WRAM_2 = 0xC081,   //  OECG  RR  Read ROM; write RAM bank 2
    io_MEM_RDROM_NOWR_2 = 0xC082,   //  OECG WR   Read ROM; no write
    io_MEM_RDRAM_WRAM_2 = 0xC083,   //  OECG  RR  Read/write RAM bank 2
    
    io_MEM_RDRAM_NOWR_2_ = 0xC084,  //  OECG WR   Read RAM bank 2; no write
    io_MEM_RDROM_WRAM_2_ = 0xC085,  //  OECG  RR  Read ROM; write RAM bank 2
    io_MEM_RDROM_NOWR_2_ = 0xC086,  //  OECG WR   Read ROM; no write
    io_MEM_RDRAM_WRAM_2_ = 0xC087,  //  OECG  RR  Read/write RAM bank 2
    
    io_MEM_RDRAM_NOWR_1 = 0xC088,   //  OECG WR   Read RAM bank 1; no write
    io_MEM_RDROM_WRAM_1 = 0xC089,   //  OECG  RR  Read ROM; write RAM bank 1
    io_MEM_RDROM_NOWR_1 = 0xC08A,   //  OECG WR   Read ROM; no write
    io_MEM_RDRAM_WRAM_1 = 0xC08B,   //  OECG  RR  Read/write RAM bank 1

    io_MEM_RDRAM_NOWR_1_ = 0xC08C,  //  OECG WR   Read RAM bank 1; no write
    io_MEM_RDROM_WRAM_1_ = 0xC08D,  //  OECG  RR  Read ROM; write RAM bank 1
    io_MEM_RDROM_NOWR_1_ = 0xC08E,  //  OECG WR   Read ROM; no write
    io_MEM_RDRAM_WRAM_1_ = 0xC08F,  //  OECG  RR  Read/write RAM bank 1
};


#define PAGESIZE 256
#define PAGES 16

extern const uint8_t * const shadowLowMEM;
extern const uint8_t * currentLowRDMEM;
extern uint8_t * currentLowWRMEM;

extern uint8_t activeTextAuxPage;
extern uint8_t * activeTextPage;
extern uint8_t * shadowTextPage;
extern uint8_t * current_RAM_bank;

extern void auxMemorySelect( MEMcfg_t newMEMcfg );
//extern void (*auxMemorySelect)( MEMcfg_t newMEMcfg );

extern void C3MemorySelect( MEMcfg_t newMEMcfg );
extern void CxMemorySelect( MEMcfg_t newMEMcfg );
extern void resetMemory(void);
extern void initMemory(void);
extern inline uint8_t *extracted(void);
extern void textPageSelect(void);

INLINE void io_RAM_EXP( uint16_t addr );
INLINE uint8_t ioRead( uint16_t addr );
INLINE void ioWrite( uint16_t addr, uint8_t val );
INLINE uint8_t memread8_low( uint16_t addr );
INLINE uint8_t memread8_high( uint16_t addr );
INLINE uint8_t memread8( uint16_t addr );
INLINE uint16_t memread16_high( uint16_t addr );
INLINE uint16_t memread16_low( uint16_t addr );
INLINE uint16_t memread16( uint16_t addr );
INLINE uint8_t _memread( uint16_t addr );
INLINE uint8_t _memread_dbg( uint16_t addr );
INLINE uint8_t _memread_dis( uint16_t addr );
INLINE void _memwrite8_zp( uint16_t addr, uint8_t data );
INLINE void _memwrite8_low( uint16_t addr, uint8_t data );
INLINE void _memwrite8_bank( uint16_t addr, uint8_t data );
INLINE void _memwrite8_high( uint16_t addr, uint8_t data );
INLINE void _memwrite( uint16_t addr, uint8_t data );
INLINE uint8_t _fetch(void);
INLINE uint8_t _fetch_dis(void);
INLINE uint16_t _fetch16(void);
INLINE uint16_t _fetch16_dis(void);
INLINE uint16_t _addr_abs(void);
INLINE uint16_t _addr_abs_dbg(void);
INLINE uint16_t _addr_abs_dis(void);
INLINE uint8_t _src_abs(void);
INLINE uint8_t _src_abs_dbg(void);
INLINE uint8_t _src_abs_dis(void);
INLINE int8_t _rel_addr(void);
INLINE int8_t _rel_addr_dis(void);
INLINE uint16_t _abs_addr(void);
INLINE uint16_t _abs_addr_dis(void);
INLINE uint16_t _ind_addr(void);
INLINE uint16_t _ind_addr_dbg(void);
INLINE uint16_t _ind_addr_dis(void);
INLINE uint16_t _addr_abs_X(void);
INLINE uint16_t _addr_abs_X_dbg(void);
INLINE uint16_t _addr_abs_X_dis(void);
INLINE uint16_t _abs_addr_X(void);
INLINE uint16_t _abs_addr_X_dis(void);
INLINE uint8_t _src_abs_X(void);
INLINE uint8_t _src_abs_X_dbg(void);
INLINE uint8_t _src_abs_X_dis(void);
INLINE uint16_t _addr_abs_Y(void);
INLINE uint16_t _addr_abs_Y_dbg(void);
INLINE uint16_t _addr_abs_Y_dis(void);
INLINE uint8_t _src_abs_Y(void);
INLINE uint8_t _src_abs_Y_dbg(void);
INLINE uint8_t _src_abs_Y_dis(void);
INLINE uint8_t _imm(void);
INLINE uint8_t _imm_dis(void);
INLINE uint8_t _addr_zp(void);
INLINE uint8_t _addr_zp_dbg(void);
INLINE uint8_t _addr_zp_dis(void);
INLINE uint8_t _src_zp(void);
INLINE uint8_t _src_zp_dbg(void);
INLINE uint8_t _src_zp_dis(void);
INLINE uint16_t _addr_ind(void);
INLINE uint16_t _addr_ind_dbg(void);
INLINE uint16_t _addr_ind_dis(void);
INLINE uint8_t _src_ind(void);
INLINE uint8_t _src_ind_dbg(void);
INLINE uint8_t _src_ind_dis(void);
INLINE uint16_t _addr_ind_X(void);
INLINE uint16_t _addr_ind_X_dbg(void);
INLINE uint16_t _addr_ind_X_dis(void);
INLINE uint16_t _addr_ind_ind_X(void);
INLINE uint16_t _addr_ind_ind_X_dbg(void);
INLINE uint16_t _addr_ind_ind_X_dis(void);
INLINE uint8_t _src_X_ind(void);
INLINE uint8_t _src_X_ind_dbg(void);
INLINE uint8_t _src_X_ind_dis(void);
INLINE uint16_t _addr_ind_Y(void);
INLINE uint16_t _addr_ind_Y_dbg(void);
INLINE uint16_t _addr_ind_Y_dis(void);
INLINE uint8_t _src_ind_Y(void);
INLINE uint8_t _src_ind_Y_dbg(void);
INLINE uint8_t _src_ind_Y_dis(void);
INLINE uint8_t _addr_zp_X(void);
INLINE uint8_t _addr_zp_X_dbg(void);
INLINE uint8_t _addr_zp_X_dis(void);
INLINE uint8_t _src_zp_X(void);
INLINE uint8_t _src_zp_X_dbg(void);
INLINE uint8_t _src_zp_X_dis(void);
INLINE uint8_t _addr_zp_Y(void);
INLINE uint8_t _addr_zp_Y_dbg(void);
INLINE uint8_t _addr_zp_Y_dis(void);
INLINE uint8_t _src_zp_Y(void);
INLINE uint8_t _src_zp_Y_dbg(void);
INLINE uint8_t _src_zp_Y_dis(void);


#ifdef DISASSEMBLER

#define fetch() _fetch_dis()
#define fetch16() _fetch16_dis()
#define memread(addr) _memread_dis(addr)
#define memwrite8_zp(addr,data)     // do not write anything into the memory while disassembling
#define memwrite8_low(addr,data)    // do not write anything into the memory while disassembling
#define memwrite8_bank(addr,data)   // do not write anything into the memory while disassembling
#define memwrite8_high(addr,data)   // do not write anything into the memory while disassembling
#define memwrite(addr,data)         // do not write anything into the memory while disassembling
#define addr_abs() _addr_abs_dis()
#define src_abs() _src_abs_dis()
#define rel_addr() _rel_addr_dis()
#define abs_addr() _abs_addr_dis()
#define ind_addr() _ind_addr_dis()
#define addr_abs_X() _addr_abs_X_dis()
#define abs_addr_X() _abs_addr_X_dis()
#define src_abs_X() _src_abs_X_dis()
#define addr_abs_Y() _addr_abs_Y_dis()
#define src_abs_Y() _src_abs_Y_dis()
#define imm() _imm_dis()
#define addr_zp() _addr_zp_dis()
#define src_zp() _src_zp_dis()
#define addr_ind() _addr_ind_dis()
#define src_ind() _src_ind_dis()
#define addr_ind_X() _addr_ind_X_dis()
#define addr_ind_ind_X() _addr_ind_ind_X_dis()
#define src_X_ind() _src_X_ind_dis()
#define addr_ind_Y() _addr_ind_Y_dis()
#define src_ind_Y() _src_ind_Y_dis()
#define addr_zp_X() _addr_zp_X_dis()
#define src_zp_X() _src_zp_X_dis()
#define addr_zp_Y() _addr_zp_Y_dis()
#define src_zp_Y() _src_zp_Y_dis()

#elif defined(DEBUGGER) // DISASSEMBLER

#define fetch() _fetch()
#define fetch16() _fetch16()
#define memread(addr) _memread_dbg(addr)
#define memwrite8_zp(addr,data) _memwrite8_zp(addr,data)
#define memwrite8_low(addr,data) _memwrite8_low(addr,data)
#define memwrite8_bank(addr,data) _memwrite8_bank(addr,data)
#define memwrite8_high(addr,data) _memwrite8_high(addr,data)
#define memwrite(addr,data) _memwrite(addr,data);
#define addr_abs() _addr_abs_dbg()
#define src_abs() _src_abs_dbg()
#define rel_addr() _rel_addr()
#define abs_addr() _abs_addr()
#define ind_addr() _ind_addr_dbg()
#define addr_abs_X() _addr_abs_X_dbg()
#define abs_addr_X() _abs_addr_X()
#define src_abs_X() _src_abs_X_dbg()
#define addr_abs_Y() _addr_abs_Y_dbg()
#define src_abs_Y() _src_abs_Y_dbg()
#define imm() _imm()
#define addr_zp() _addr_zp_dbg()
#define src_zp() _src_zp_dbg()
#define addr_ind() _addr_ind_dbg()
#define src_ind() _src_ind_dbg()
#define addr_ind_X() _addr_ind_X_dbg()
#define addr_ind_ind_X() _addr_ind_ind_X_dbg()
#define src_X_ind() _src_X_ind_dbg()
#define addr_ind_Y() _addr_ind_Y_dbg()
#define src_ind_Y() _src_ind_Y_dbg()
#define addr_zp_X() _addr_zp_X_dbg()
#define src_zp_X() _src_zp_X_dbg()
#define addr_zp_Y() _addr_zp_Y_dbg()
#define src_zp_Y() _src_zp_Y_dbg()

#else // DEBUGGER

#define fetch() _fetch()
#define fetch16() _fetch16()
#define memread(addr) _memread(addr)
#define memwrite8_zp(addr,data) _memwrite8_zp(addr,data)
#define memwrite8_low(addr,data) _memwrite8_low(addr,data)
#define memwrite8_bank(addr,data) _memwrite8_bank(addr,data)
#define memwrite8_high(addr,data) _memwrite8_high(addr,data)
#define memwrite(addr,data) _memwrite(addr,data)
#define addr_abs() _addr_abs()
#define src_abs() _src_abs()
#define rel_addr() _rel_addr()
#define abs_addr() _abs_addr()
#define ind_addr() _ind_addr()
#define addr_abs_X() _addr_abs_X()
#define abs_addr_X() _abs_addr_X()
#define src_abs_X() _src_abs_X()
#define addr_abs_Y() _addr_abs_Y()
#define src_abs_Y() _src_abs_Y()
#define imm() _imm()
#define addr_zp() _addr_zp()
#define src_zp() _src_zp()
#define addr_ind() _addr_ind()
#define src_ind() _src_ind()
#define addr_ind_X() _addr_ind_X()
#define addr_ind_ind_X() _addr_ind_ind_X()
#define src_X_ind() _src_X_ind()
#define addr_ind_Y() _addr_ind_Y()
#define src_ind_Y() _src_ind_Y()
#define addr_zp_X() _addr_zp_X()
#define src_zp_X() _src_zp_X()
#define addr_zp_Y() _addr_zp_Y()
#define src_zp_Y() _src_zp_Y()

#endif // DISASSEMBLER & DEBUGGER


#endif // __APPLE2_MMIO_H__


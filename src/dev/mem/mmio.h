//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __APPLE2_MMIO_H__
#define __APPLE2_MMIO_H__

#include "common.h"
#include "6502.h"
#include "disk.h"
#include "woz.h"
#include "speaker.h"


typedef union address16_u {
    uint16_t addr;
    struct {
        uint8_t offs;
        uint8_t page;
    };
} address16_t;


videoMode_t videoMode = { 1 }; // 40 col text, page 1


uint8_t Apple2_Dummy_Page[ 1 * PG ];            // Dummy Page to discard data
uint8_t Apple2_Dummy_RAM[ 64 * KB ];            // Dummy RAM to discard data

uint8_t Apple2_16K_ROM[ 16 * KB ] = {0};        // ROM C0, C8, D0, D8, E0, E8, F0, F8

uint8_t Apple2_64K_AUX[ 64 * KB ] = {0};        // 64K Expansion Memory
uint8_t Apple2_64K_RAM[ 64 * KB ] = {0};        // Main Memory
uint8_t Apple2_64K_MEM[ 64 * KB ] = {0};        // Shadow Copy of Memory (or current memory content)

//uint8_t * AUX_VID_RAM = Apple2_VID_AUX;       // Pointer to Auxiliary Video Memory
uint8_t * const AUX = Apple2_64K_AUX;           // Pointer to the Auxiliary Memory so we can use this from Swift
uint8_t * const RAM = Apple2_64K_RAM;           // Pointer to the Main Memory so we can use this from Swift
uint8_t * const MEM = Apple2_64K_MEM;           // Pointer to the Shadow Memory Map so we can use this from Swift

uint8_t * const RDLOMEM = Apple2_64K_MEM;       // Pointer to the Shadow Memory Map so we can use this from Swift
uint8_t * const WRLOMEM = Apple2_64K_MEM;       // Pointer to the Shadow Memory Map so we can use this from Swift
uint8_t * const RDHIMEM = Apple2_64K_MEM;       // Pointer to the Shadow Memory Map so we can use this from Swift
uint8_t * const WRHIMEM = Apple2_Dummy_RAM;     // Pointer to the Shadow Memory Map so we can use this from Swift


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


MEMcfg_t MEMcfg = { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// https://www.kreativekorp.com/miscpages/a2info/iomemory.shtml
// Comp:  O = Apple II+  E = Apple IIe  C = Apple IIc  G = Apple IIgs
// Act:   R = Read       W = Write      7 = Bit 7      V = Value

enum mmio {
    // Keyboard
    io_KBD              = 0xC000,   // OECG  R   Last Key Pressed + 128
    io_KBDSTRB          = 0xC010,   // OECG WR   Keyboard Strobe

    // Audio
    io_SPKR             = 0xC030,   // OECG  R   Toggle Speaker
    
    // Video
    io_80STOREOFF       = 0xC000,   //  ECG W    Use $C002-$C005 for Aux Memory
    io_80STOREON        = 0xC001,   //  ECG W    Use PAGE2 for Aux Memory
    io_RD80STORE        = 0xC018,   //  ECG  R7  Status of $C002-$C005/PAGE2 for Aux Mem
    io_VID_CLR80VID     = 0xC00C,   //  ECG W    40 Columns
    io_VID_SET80VID     = 0xC00D,   //  ECG W    80 Columns
    io_VID_CLRALTCHAR   = 0xC00E,   //  ECG W    Primary Character Set
    io_VID_SETALTCHAR   = 0xC00F,   //  ECG W    Alternate Character Set
    io_VID_RDTEXT       = 0xC01A,   //  ECG  R7  Status of Text/Graphics
    io_VID_ALTCHAR      = 0xC01E,   //  ECG  R7  Status of Primary/Alternate Character Set
    io_VID_RD80VID      = 0xC01F,   //  ECG  R7  Status of 40/80 Columns
    io_VID_Text_OFF     = 0xC050,
    io_VID_Text_ON      = 0xC051,
    io_VID_Mixed_OFF    = 0xC052,
    io_VID_Mixed_ON     = 0xC053,
    io_VID_TXTPAGE1     = 0xC054,   // OECG WR   Display Page 1
    io_VID_TXTPAGE2     = 0xC055,   // OECG WR   If 80STORE Off: Display Page 2
                                    //  ECG WR   If 80STORE On: Read/Write Aux Display Mem
    io_VID_Hires_OFF    = 0xC056,
    io_VID_Hires_ON     = 0xC057,
    
    io_TAPEIN           = 0xC060,   // OE    R7  Read Cassette Input
                                    //   C   R7  Status of 80/40 Column Switch

    // Game Controller
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
    io_RDCXROM          = 0xC015,   //  E G  R7  Read state of $C100-$CFFF soft switch -- Status of Periph/ROM Access
    io_RSTXINT          = 0xC015,   //   C   R   Reset Mouse X0 Interrupt
    io_RDALTZP          = 0xC016,   //  ECG  R7  Status of Main/Aux Stack and Zero Page
    io_RDC3ROM          = 0xC017,   //  E G  R7  Status of Slot 3/Aux Slot ROM
    io_RSTYINT          = 0xC017,   //   C   R   Reset Mouse Y0 Interrupt
    io_MEM_RDRAM_NOWR_2 = 0xC080,
    io_MEM_RDROM_WRAM_2 = 0xC081,
    io_MEM_RDROM_NOWR_2 = 0xC082,
    io_MEM_RDRAM_WRAM_2 = 0xC083,
    io_MEM_RDRAM_NOWR_1 = 0xC088,
    io_MEM_RDROM_WRAM_1 = 0xC089,
    io_MEM_RDROM_NOWR_1 = 0xC08A,
    io_MEM_RDRAM_WRAM_1 = 0xC08B,
};


#define PAGESIZE 256
#define PAGES 16


void resetMemory() {
    
    // Reset memory configuration
    MEMcfg.RAM_16K      = 0;
    MEMcfg.RAM_128K     = 1;
    MEMcfg.RD_RAM       = 0;
    MEMcfg.WR_RAM       = 0;
    MEMcfg.RAM_BANK_2   = 0;
    MEMcfg.AUX_BANK     = 0;
    MEMcfg.is_80STORE   = 0;
    MEMcfg.RD_AUX_MEM   = 0;
    MEMcfg.WR_AUX_MEM   = 0;
    MEMcfg.int_Cx_ROM   = 0;
    MEMcfg.slot_C3_ROM  = 0;
    MEMcfg.ALT_ZP       = 0;
    MEMcfg.txt_page_2   = 0;
    

    // Aux Video Memory
    memset( Apple2_64K_AUX, 0, sizeof(Apple2_64K_AUX) );
    // 64K Main Memory Area
    memset( Apple2_64K_RAM, 0, sizeof(Apple2_64K_RAM) );
    memset( Apple2_64K_MEM, 0, sizeof(Apple2_64K_MEM) );
    // text memory should be filled by spaces
    memset( Apple2_64K_AUX + 0x400, 0xA0, 0x800 );
    memset( Apple2_64K_RAM + 0x400, 0xA0, 0x800 );
    memset( Apple2_64K_MEM + 0x400, 0xA0, 0x800 );
    // I/O area should be 0 -- just in case we decide to init RAM with a different pattern...
    memset( Apple2_64K_RAM + 0xC000, 0, 0x1000 );
}


static uint8_t page2 = 0;
static uint8_t * activePage = Apple2_64K_RAM + 0x400;

void textPageSelect() {
    uint8_t * shadow = Apple2_64K_MEM + 0x400;
    
    if ( page2 != MEMcfg.txt_page_2 ) {
        page2 = MEMcfg.txt_page_2;
        static uint8_t * newPage = NULL;

        if ( MEMcfg.is_80STORE && MEMcfg.txt_page_2 ) {
            newPage = Apple2_64K_AUX + 0x400;
        }
        else {
            newPage = Apple2_64K_RAM + 0x400;
        }

        if ( activePage != newPage ) {
            if ( activePage ) {
                // save the content of Shadow Memory
                memcpy(activePage, shadow, 0x400);
            }
            
            // load the content of Video Page 2
            memcpy(shadow, newPage, 0x400);
            
            activePage = newPage;
        }
    }
    
}


void auxMemorySelect() {
    uint8_t * shadow = Apple2_64K_MEM + 0x200;
    uint8_t * memory;

    if ( MEMcfg.is_80STORE ) {
        if ( MEMcfg.RD_AUX_MEM ) {
            memory = Apple2_64K_AUX + 0x200;
            
            // save the content of Shadow Memory
            memcpy(Apple2_64K_RAM + 0x200, shadow, 0xA00);
            
            // load the content of Aux Memory
            memcpy(Apple2_64K_MEM + 0x200, Apple2_64K_AUX, 0xA00);
        }
        else {
            memory = Apple2_64K_RAM + 0x200;
            
            // save the content of Shadow Memory
            memcpy(Apple2_64K_AUX + 0x200, shadow, 0xA00);
            
            // load the content of Int Memory
            memcpy(Apple2_64K_MEM + 0x200, Apple2_64K_RAM, 0xA00);
        }
        
        if ( MEMcfg.WR_AUX_MEM ) {
            // TODO: set write table for AUX
        }
        else {
            // TODO: set write table for RAM
        }
    }
    else {
        // TODO: set read table for RAM
    }
    
    // load new content to shadow memory
    memcpy(shadow, memory, 0xA00);
}


INLINE uint8_t ioRead( uint16_t addr ) {
//    if (outdev) fprintf(outdev, "ioRead:%04X\n", addr);
//    printf("ioRead:%04X (PC:%04X)\n", addr, m6502.PC);

    uint8_t currentMagnet = 0;
    
    switch ( (uint8_t)addr ) {
        case (uint8_t)io_KBD:
//            if ( RAM[io_KBD] > 0x7F ) printf("io_KBD:%04X\n", addr);
            return Apple2_64K_RAM[io_KBD];

        case (uint8_t)io_KBDSTRB:
            // TODO: This is very slow!
//            printf("io_KBDSTRB\n");
            Apple2_64K_RAM[io_KBD] &= 0x7F;
            return Apple2_64K_RAM[io_KBDSTRB];

        case (uint8_t)io_SPKR:
            spkr_toggle();
            return Apple2_64K_RAM[io_SPKR];

        case (uint8_t)io_VID_RDTEXT:
            return videoMode.text << 7;
            
        case (uint8_t)io_VID_ALTCHAR:
            return videoMode.altChr << 7;
            
        case (uint8_t)io_VID_RD80VID:
            return videoMode.col80 << 7;
            
        case (uint8_t)io_TAPEIN:
            return MEMcfg.txt_page_2 << 7;
            
        case (uint8_t)io_RDCXROM:
            return MEMcfg.int_Cx_ROM << 7;
            
        case (uint8_t)io_RDALTZP:
            return MEMcfg.ALT_ZP << 7;
            
        case (uint8_t)io_RDC3ROM:
            return MEMcfg.slot_C3_ROM << 7;
            
        case (uint8_t)io_RD80STORE:
            return MEMcfg.is_80STORE << 7;
            
        case (uint8_t)io_VID_TXTPAGE1:
//            printf("io_VID_TXTPAGE1\n");
            MEMcfg.txt_page_2 = 0;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_TXTPAGE2:
//            printf("io_VID_TXTPAGE2\n");
            MEMcfg.txt_page_2 = 1;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_Text_OFF:
            videoMode.text = 0;
            break;
            
        case (uint8_t)io_VID_Text_ON:
            videoMode.text = 1;
            break;
            
        case (uint8_t)io_VID_Mixed_OFF:
            videoMode.mixed = 0;
            break;
            
        case (uint8_t)io_VID_Mixed_ON:
            videoMode.mixed = 1;
            break;
            
        case (uint8_t)io_VID_Hires_OFF:
            videoMode.hires = 0;
            break;
            
        case (uint8_t)io_VID_Hires_ON:
            videoMode.hires = 1;
            break;
            
            
        case (uint8_t)io_PDL0:
        case (uint8_t)io_PDL1:
        case (uint8_t)io_PDL2:
        case (uint8_t)io_PDL3:
//            if ( RAM[addr] > 127 ) {
//                RAM[addr]--;
//            }
            return Apple2_64K_RAM[addr];
            
        case (uint8_t)io_MEM_RDRAM_NOWR_2:
        case (uint8_t)io_MEM_RDROM_WRAM_2:
        case (uint8_t)io_MEM_RDROM_NOWR_2:
        case (uint8_t)io_MEM_RDRAM_WRAM_2:
        case (uint8_t)io_MEM_RDRAM_NOWR_1:
        case (uint8_t)io_MEM_RDROM_WRAM_1:
        case (uint8_t)io_MEM_RDROM_NOWR_1:
        case (uint8_t)io_MEM_RDRAM_WRAM_1:
            if ( MEMcfg.RAM_16K || MEMcfg.RAM_128K ) {
                uint8_t * RAM_BANK = Apple2_64K_AUX + 0xC000;
                
                // RAM Bank 1 or 2?
                switch ((uint8_t)addr) {
                    case (uint8_t)io_MEM_RDRAM_NOWR_2:
                    case (uint8_t)io_MEM_RDROM_WRAM_2:
                    case (uint8_t)io_MEM_RDROM_NOWR_2:
                    case (uint8_t)io_MEM_RDRAM_WRAM_2:
                        MEMcfg.RAM_BANK_2 = 1;
                        RAM_BANK = Apple2_64K_AUX + 0xD000;
                        break;
                        
                    default:
                        MEMcfg.RAM_BANK_2 = 0;
                        RAM_BANK = Apple2_64K_AUX + 0xC000;
                        break;
                }

                // is RAM to read or ROM?
                switch ((uint8_t)addr) {
                    case (uint8_t)io_MEM_RDRAM_NOWR_2:
                    case (uint8_t)io_MEM_RDRAM_WRAM_2:
                    case (uint8_t)io_MEM_RDRAM_NOWR_1:
                    case (uint8_t)io_MEM_RDRAM_WRAM_1:
                        MEMcfg.RD_RAM = 1;
                        
                        uint8_t * shadow = Apple2_64K_MEM + 0xD000;
                        // save the content of Shadow Memory
                        memcpy(Apple2_64K_RAM + 0xD000, shadow, 0x3000);
                        // load the content of Aux Memory
                        memcpy(Apple2_64K_MEM + 0xD000, Apple2_64K_AUX, 0x3000);

                        // set the RAM extension to read on the upper memory area
                        break;
                        
                    default:
                        MEMcfg.RD_RAM = 0;
                        
                        shadow = Apple2_64K_MEM + 0xD000;
                        // save the content of Shadow Memory
                        memcpy(Apple2_64K_AUX + 0xD000, shadow, 0x3000);
                        // load the content of ROM Memory
                        memcpy(Apple2_64K_MEM + 0xD000, Apple2_16K_ROM + 0x1000, 0x3000);

                        // set the ROM to read on the upper memory area
                        break;
                }

                // is RAM Writeable?
                switch ((uint8_t)addr) {
                    case (uint8_t)io_MEM_RDROM_WRAM_2:
                    case (uint8_t)io_MEM_RDRAM_WRAM_2:
                    case (uint8_t)io_MEM_RDROM_WRAM_1:
                    case (uint8_t)io_MEM_RDRAM_WRAM_1:
                        MEMcfg.WR_RAM = 1;
                        // set the RAM extension to read from the upper memory area
                        break;
                        
                    default:
                        MEMcfg.WR_RAM = 0;
                        // set the ROM to read on the upper memory area
                        break;
                }
                
            } // if there is RAM expansion card installed
            break;

        // TODO: Make code "card insertable to slot" / aka slot independent and dynamically add/remove
        case (uint8_t)io_DISK_PHASE0_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE1_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE2_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE3_OFF + SLOT6:
            currentMagnet = (addr - io_DISK_PHASE0_OFF - SLOT6) / 2;
            disk.phase.magnet &= ~(1 << currentMagnet);
            printf("io_DISK_PHASE%u_OFF (S%u, ps:%X) ", currentMagnet, 6, disk.phase.magnet);

            disk_phase();
            return 0;

        case (uint8_t)io_DISK_PHASE0_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE1_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE2_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE3_ON + SLOT6: {
            currentMagnet = (addr - io_DISK_PHASE0_ON - SLOT6) / 2;
            disk.phase.magnet |= 1 << currentMagnet;
            printf("io_DISK_PHASE%u_ON (S%u, ps:%X) ", currentMagnet, 6, disk.phase.magnet);

            disk_phase();
            return 0;
        }

        case (uint8_t)io_DISK_POWER_OFF + SLOT6:
            dbgPrintf2("io_DISK_POWER_OFF (S%u)\n", 6);
            return 0;

        case (uint8_t)io_DISK_POWER_ON + SLOT6:
            dbgPrintf2("io_DISK_POWER_ON (S%u)\n", 6);
            return 0;

        case (uint8_t)io_DISK_SELECT_1 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_1 (S%u)\n", 6);
            return 0;

        case (uint8_t)io_DISK_SELECT_2 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_2 (S%u)\n", 6);
            return 0;

        case (uint8_t)io_DISK_READ + SLOT6:
            return disk_read();
          
            
        case (uint8_t)io_DISK_WRITE + SLOT6:
            dbgPrintf2("io_DISK_WRITE (S%u)\n", 6);
            return 0;

        case (uint8_t)io_DISK_CLEAR + SLOT6:
            dbgPrintf2("io_DISK_CLEAR (S%u)\n", 6);
            return 0;

        case (uint8_t)io_DISK_SHIFT + SLOT6:
            dbgPrintf2("io_DISK_SHIFT (S%u)\n", 6);
            return 0;

            
            
        default:
            //printf("mmio read:%04X\n", addr);
            break;
    }
    
    return Apple2_64K_RAM[addr];
}

// TODO:
void setIO ( uint16_t ioaddr, uint8_t val ) {
    Apple2_64K_RAM[ioaddr] = val;
}


void kbdInput ( uint8_t code ) {
//    printf("kbdInput: %02X ('%c')\n", code, isprint(code) ? code : ' ');
    switch ( code ) {
        case '\n':
            code = 0x0D;
            break;

        case 0x7F: // BackSlash
            code = 0x08;
            break;
            
        default:
            break;
    }
    
    code |= 0x80;
    
    // timeout with linearly increasing sleep
    for( int i = 1; i < 100 && ( RAM[io_KBD] > 0x7F ); i++ ) {
        usleep( i * 2 );
    }

    RAM[io_KBD] = RAM[io_KBDSTRB] = code;
}


void kbdUp () {
    RAM[io_KBDSTRB] &= 0x7F;
}



INLINE void ioWrite( uint16_t addr, uint8_t val ) {
//    if (outdev) fprintf(outdev, "ioWrite:%04X (A:%02X)\n", addr, m6502.A);
    
    switch (addr) {
        case io_KBDSTRB:
            Apple2_64K_RAM[io_KBD] &= 0x7F;
            break;
            
        case io_SPKR:
            spkr_toggle();
            break;
            
        case io_RDMAINRAM:
//            printf("io_RDMAINRAM\n");
            MEMcfg.RD_AUX_MEM = 0;
            auxMemorySelect();
            break;

        case io_RDCARDRAM:
//            printf("io_RDCARDRAM\n");
            MEMcfg.RD_AUX_MEM = 1;
            auxMemorySelect();
            break;

        case io_WRMAINRAM:
//            printf("io_WRMAINRAM\n");
            MEMcfg.WR_AUX_MEM = 0;
            auxMemorySelect();
            break;

        case io_WRCARDRAM:
//            printf("io_WRCARDRAM\n");
            MEMcfg.WR_AUX_MEM = 1;
            auxMemorySelect();
            break;

        case io_SETSTDZP:
            MEMcfg.ALT_ZP = 0;
            // TODO: set zero page table to RAM
            break;
            
        case io_SETALTZP:
            MEMcfg.ALT_ZP = 1;
            // TODO: set zero page table to AUX
            break;

        case io_SETSLOTCXROM:
//            printf("io_SETSLOTCXROM\n");
            MEMcfg.int_Cx_ROM = 0;
            // TODO: set Cx00 ROM area table to SLOT
            break;

        case io_SETINTCXROM:
//            printf("io_SETINTCXROM\n");
            MEMcfg.int_Cx_ROM = 1;
            // TODO: set Cx00 ROM area table to INT
            break;

        case io_SETSLOTC3ROM:
//            printf("io_SETSLOTC3ROM\n");
            MEMcfg.slot_C3_ROM = 1;
            // TODO: set C300 ROM area table to SLOT
            break;

        case io_SETINTC3ROM:
//            printf("io_SETINTC3ROM\n");
            MEMcfg.slot_C3_ROM = 0;
            // TODO: set C300 ROM area table to INT
            break;

        case io_VID_CLR80VID:
//            printf("io_VID_CLR80VID\n");
            videoMode.col80 = 0;
            break;
            
        case io_VID_SET80VID:
            videoMode.col80 = 1;
            break;
            
        case io_VID_CLRALTCHAR:
            videoMode.altChr = 0;
            break;
            
        case io_VID_SETALTCHAR:
            videoMode.altChr = 1;
            break;
            
        case io_80STOREOFF:
//            printf("io_80STOREOFF\n");
            MEMcfg.is_80STORE = 0;
            textPageSelect();
            break;
            
        case io_80STOREON:
//            printf("io_80STOREON\n");
            MEMcfg.is_80STORE = 1;
            textPageSelect();
            break;
            
        case io_VID_TXTPAGE1:
//            printf("io_VID_TXTPAGE1\n");
            MEMcfg.txt_page_2 = 0;
            textPageSelect();
            break;
            
        case io_VID_TXTPAGE2:
//            printf("io_VID_TXTPAGE2\n");
            MEMcfg.txt_page_2 = 1;
            textPageSelect();
            break;
            
        default:
            break;
    }
    return;
}

/**
 Naive implementation of RAM read from address
 **/

/**
 Naive implementation of RAM read from address
 **/
INLINE uint8_t memread8_low( uint16_t addr ) {
    return Apple2_64K_MEM[addr];
}
INLINE uint8_t memread8_high( uint16_t addr ) {
    return RDHIMEM[addr];
}
INLINE uint8_t memread8( uint16_t addr ) {
    if (addr >= 0xC000) {
        return memread8_high(addr);
    }

    return memread8_low(addr);
}
/**
 Naive implementation of RAM read from address
 **/
INLINE uint16_t memread16_low( uint16_t addr ) {
    return * (uint16_t*) ( Apple2_64K_MEM + addr );
}
INLINE uint16_t memread16_high( uint16_t addr ) {
    return * (uint16_t*) ( RDHIMEM + addr );
}
INLINE uint16_t memread16( uint16_t addr ) {

//    if (addr >= 0xC000) {
//        return memread16_high(addr);
//    }

    return memread16_low(addr);
}

INLINE uint8_t memread( uint16_t addr ) {
    if (addr >= 0xC000) {
        if (addr < 0xC100) {
            return ioRead(addr);
        }

//        return memread8_paged(addr);
        return memread8_high(addr);
    }

//    return memread8_paged(addr);
    return memread8_low(addr);

//    return memread8(addr);
}

/**
 Naive implementation of RAM read from address
 **/
//INLINE uint16_t memioread16( uint16_t addr ) {
//    return (uint16_t)mmio_read[ addr ](addr);
//}


/**
 Naive implementation of RAM write to address
 **/
//static  void memwrite_zp( uint8_t addr, uint8_t byte ) {
//    RAM[ addr ] = byte;
//}


/**
 Naive implementation of RAM write to address
 **/

INLINE void memwrite8_low( uint16_t addr, uint8_t data ) {
    WRLOMEM[addr] = data;
}
INLINE void memwrite8_high( uint16_t addr, uint8_t data ) {
    WRHIMEM[addr] = data;
}
INLINE void memwrite( uint16_t addr, uint8_t data ) {
    if (addr >= 0xC000) {
        if (addr < 0xC100) {
            ioWrite(addr, data);
        }
        else {
           memwrite8_high(addr, data);
        }
    }
    else {
        memwrite8_low(addr, data);
    }
    
}

/**
 Fetching 1 byte from memory address pc (program counter)
 increase pc by one
 **/
INLINE uint8_t fetch() {
    disHexB( disassembly.pOpcode, memread(m6502.PC) );
#ifdef CLK_ABSOLUTE_PRECISE
    if ( (m6502.PC & 0xFF) >= 0xFF ) {
        m6502.clktime++;
    }
#endif
    return memread8_low( m6502.PC++ );
}

/**
 Fetching 2 bytes as a 16 bit number from memory address pc (program counter)
 increase pc by one
 **/
INLINE uint16_t fetch16() {
    uint16_t word = memread16( m6502.PC );
    // disPrintf(disassembly.comment, "fetch16:%04X", word);
    #ifdef CLK_ABSOLUTE_PRECISE
    if ( (m6502.PC & 0xFF) >= 0xFE ) {
        m6502.clktime++;
    }
    #endif
    m6502.PC += 2;
    disHexW( disassembly.pOpcode, word );
    return word;
}


INLINE uint8_t * dest( uint8_t * mem, uint16_t addr ) {
    return mem + addr;
}


/**
 abs        ....    absolute         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
INLINE uint16_t addr_abs() {
    dbgPrintf("abs:%04X(%02X) ", *((uint16_t*)&RAM[m6502.PC]), RAM[*((uint16_t*)&RAM[m6502.PC])]);
    disPrintf(disassembly.oper, "$%04X", memread16(m6502.PC))
    return fetch16();
}
INLINE uint8_t src_abs() {
    return memread( addr_abs() );
}
INLINE uint8_t * dest_abs() {
    return WRLOMEM + addr_abs();
}


INLINE int8_t rel_addr() {
    disPrintf(disassembly.oper, "$%04X", m6502.PC + 1 + (int8_t)memread8(m6502.PC))
    return fetch();
}
INLINE uint16_t abs_addr() {
    disPrintf(disassembly.oper, "$%04X", memread16(m6502.PC))
    return fetch16();
}
INLINE uint16_t ind_addr() {
    disPrintf(disassembly.oper, "($%04X)", memread16(m6502.PC))
    disPrintf(disassembly.comment, "ind_addr:%04X", memread16(memread16(m6502.PC)))
    return memread16( fetch16() );
}

/**
 abs,X        ....    absolute, X-indexed         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
INLINE uint16_t addr_abs_X() {
    dbgPrintf("abs,X:%04X(%02X) ", *((uint16_t*)&RAM[m6502.PC]) + m6502.X, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.X]);
    disPrintf(disassembly.oper, "$%04X,X", memread16(m6502.PC));
    return fetch16() + m6502.X;
}
INLINE uint8_t src_abs_X() {
    return memread( addr_abs_X() );
}
INLINE uint8_t * dest_abs_X() {
    return WRLOMEM + addr_abs_X();
}


/**
 abs,Y        ....    absolute, Y-indexed         OPC $LLHH,Y
 operand is address; effective address is address incremented by Y with carry **
 **/
INLINE uint16_t addr_abs_Y() {
    dbgPrintf("abs,Y:%04X(%02X) ", *((uint16_t*)&RAM[m6502.PC]) + m6502.Y, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.Y]);
    disPrintf(disassembly.oper, "$%04X,Y", memread16(m6502.PC))
    return fetch16() + m6502.Y;
}
INLINE uint8_t src_abs_Y() {
    return memread(addr_abs_Y());
}
INLINE uint8_t * dest_abs_Y() {
    return WRLOMEM + addr_abs_Y();
}

INLINE uint8_t imm() {
    disPrintf(disassembly.oper, "#$%02X", memread8(m6502.PC))
    return fetch();
}


/**
 zpg        ....    zeropage         OPC $LL
 operand is zeropage address (hi-byte is zero, address = $00LL)
 **/
INLINE uint8_t addr_zp() {
    dbgPrintf("zp:%02X(%02X) ", RAM[m6502.PC], RAM[ RAM[m6502.PC]] );
    disPrintf(disassembly.oper, "$%02X", memread8(m6502.PC))
    return fetch();
}
INLINE uint8_t src_zp() {
    return memread8_low(addr_zp());
}
INLINE uint8_t * dest_zp() {
    return WRLOMEM + addr_zp();
}

/**
 get a 16 bit address from the zp:zp+1
 **/
INLINE uint16_t addr_zp_ind( uint8_t addr ) {
    dbgPrintf("zpi:%02X:%04X(%02X) ", RAM[m6502.PC], *((uint16_t*)&RAM[m6502.PC]), RAM[*((uint16_t*)&RAM[m6502.PC])]);
    disPrintf(disassembly.oper, "($%02X)", memread8(m6502.PC) );
    disPrintf(disassembly.comment, "ind_addr:%04X", memread16( memread8(m6502.PC) ) );
    return memread16(addr);
}

/**
 X,ind        ....    X-indexed, indirect         OPC ($LL,X)
 operand is zeropage address;
 effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
 **/
INLINE uint16_t addr_X_ind() {
    dbgPrintf("zpXi:%02X:%04X(%02X) ", RAM[m6502.PC], *((uint16_t*)&RAM[m6502.PC]) + m6502.X, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.X]);
    disPrintf(disassembly.oper, "($%02X,X)", memread8(m6502.PC) )
    disPrintf(disassembly.comment, "ind_addr:%04X", memread16( memread8(m6502.PC) + m6502.X) );
    return memread16( fetch() + m6502.X );
}
INLINE uint8_t src_X_ind() {
    return memread( addr_X_ind() );
}
INLINE uint8_t * dest_X_ind() {
    return WRLOMEM + addr_X_ind();
}

/**
 ind,Y        ....    indirect, Y-indexed         OPC ($LL),Y
 operand is zeropage address;
 effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
 **/
INLINE uint16_t addr_ind_Y() {
    //    uint8_t a = fetch();
    //    dbgPrintf("addr_ind_Y: %04X + %02X = %04X ", addr_zpg_ind( a ), m6502.Y, addr_zpg_ind( a ) + m6502.Y);
    disPrintf(disassembly.oper, "($%02X),Y", memread8(m6502.PC) )
    disPrintf(disassembly.comment, "ind_addr:%04X", memread16( memread8(m6502.PC) ) + m6502.Y );
    return memread16( fetch() ) + m6502.Y;
}
INLINE uint8_t src_ind_Y() {
    return memread( addr_ind_Y() );
}
INLINE uint8_t * dest_ind_Y() {
    return WRLOMEM + addr_ind_Y();
}

/**
 zpg,X        ....    zeropage, X-indexed         OPC $LL,X
 operand is zeropage address;
 effective address is address incremented by X without carry **
 **/
INLINE uint8_t addr_zp_X() {
    disPrintf(disassembly.oper, "$%02X,X", memread8(m6502.PC))
    return fetch() + m6502.X;
}
INLINE uint8_t src_zp_X() {
    return memread8_low(addr_zp_X());
}
INLINE uint8_t * dest_zp_X() {
    return WRLOMEM + addr_zp_X();
}

/**
 zpg,Y        ....    zeropage, Y-indexed         OPC $LL,Y
 operand is zeropage address;
 effective address is address incremented by Y without carry **
 **/
INLINE uint8_t addr_zp_Y() {
    disPrintf(disassembly.oper, "$%02X,Y", memread8(m6502.PC))
    return fetch() + m6502.Y;
}
INLINE uint8_t src_zp_Y() {
    return memread8_low(addr_zp_Y());
}
INLINE uint8_t * dest_zp_Y() {
    return WRLOMEM + addr_zp_Y();
}


#endif // __APPLE2_MMIO_H__


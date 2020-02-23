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


typedef union address16_u {
    uint16_t addr;
    struct {
        uint8_t offs;
        uint8_t page;
    };
} address16_t;


videoMode_t videoMode = { 1 }; // 40 col text, page 1


uint8_t Apple2_Dummy_Page[ 1 * PG ];        // Dummy Page for discarding data
uint8_t Apple2_512_AUX[  2 * PG ] = {0};    // Auxiliary bank for page 0 and 1
uint8_t Apple2_12K_ROM[ 12 * KB ] = {0};    // ROM D0, D8, E0, E8, F0, F8
uint8_t Apple2_16K_RAM[ 16 * KB ] = {0};    // 16K Memory Expansion Card
uint8_t Apple2_64K_RAM[ 64 * KB ] = {0};    // Main Memory
uint8_t * RAM = Apple2_64K_RAM;             // Pointer to the main memory so we can use this from Swift


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

uint8_t * RAM_PG_RD_TBL[256] = {
    // 48K main memory
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x00),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x10),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x20),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x30),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x40),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x50),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x60),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x70),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x80),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x90),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0xA0),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0xB0),
    // I/O Addresses
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0xC0),
    // Reading from the ROM
    DEF_RAM_PAGE16( Apple2_12K_ROM, 0x00),  // D0
    DEF_RAM_PAGE16( Apple2_12K_ROM, 0x10),  // E0
    DEF_RAM_PAGE16( Apple2_12K_ROM, 0x20)   // F0
};

uint8_t * RAM_PG_WR_TBL[256] = {
    // 48K main memory
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x00),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x10),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x20),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x30),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x40),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x50),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x60),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x70),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x80),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0x90),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0xA0),
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0xB0),
    // I/O Addresses
    DEF_RAM_PAGE16( Apple2_64K_RAM, 0xC0),
    // NO Writing to the ROM
    DEF_RAM_DUMMY16,
    DEF_RAM_DUMMY16,
    DEF_RAM_DUMMY16,
};



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


// Memory Config
struct MEMcfg_s {
    uint8_t RAM_16K     : 1;
    uint8_t RAM_128K    : 1;
    uint8_t RD_RAM      : 1;
    uint8_t WR_RAM      : 1;
    uint8_t RAM_BANK_2  : 1;
    uint8_t AUX_BANK    : 1;
} MEMcfg = { 1, 0, 0, 0, 0 };

enum mmio {
    // Keyboard
    io_KBD              = 0xC000,
    io_KBDSTRB          = 0xC010,

    // Audio
    io_SPKR             = 0xC030,
    
    // Video
    io_VID_80col_OFF    = 0xC00C,
    io_VID_80col_ON     = 0xC00D,
    io_VID_AltChar_OFF  = 0xC00E,
    io_VID_AltChar_ON   = 0xC00F,
    io_VID_Text_OFF     = 0xC050,
    io_VID_Text_ON      = 0xC051,
    io_VID_Mixed_OFF    = 0xC052,
    io_VID_Mixed_ON     = 0xC053,
    io_VID_Page2_OFF    = 0xC054,
    io_VID_Page2_ON     = 0xC055,
    io_VID_Hires_OFF    = 0xC056,
    io_VID_Hires_ON     = 0xC057,

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



INLINE uint8_t ioRead( uint16_t addr ) {
    dbgPrintf("mmio read:%04X\n", addr);
    
    uint8_t currentMagnet = 0;
    
    switch (addr) {
        case io_KBD:
//            if ( RAM[io_KBD] > 0x7F ) printf("io_KBD:%04X\n", addr);
            return RAM[io_KBD];

        case io_KBDSTRB:
            // TODO: This is very slow!
//            printf("io_KBDSTRB\n");
            return RAM[io_KBD] &= 0x7F;

        case io_SPKR:
            // TODO: This is very slow!
//            printf("io_KBDSTRB\n");
            
            //ViewController_spk_up_play();
            
            return RAM[io_SPKR];

        case io_VID_80col_OFF:
            videoMode.col80 = 0;
            break;
            
        case io_VID_80col_ON:
            videoMode.col80 = 1;
            break;
            
        case io_VID_AltChar_OFF:
            videoMode.altChr = 0;
            break;
            
        case io_VID_AltChar_ON:
            videoMode.altChr = 1;
            break;
            
        case io_VID_Text_OFF:
            videoMode.text = 0;
            break;
            
        case io_VID_Text_ON:
            videoMode.text = 1;
            break;
            
        case io_VID_Mixed_OFF:
            videoMode.mixed = 0;
            break;
            
        case io_VID_Mixed_ON:
            videoMode.mixed = 1;
            break;
            
        case io_VID_Page2_OFF:
            videoMode.page = 0;
            break;
            
        case io_VID_Page2_ON:
            videoMode.page = 1;
            break;
            
        case io_VID_Hires_OFF:
            videoMode.hires = 0;
            break;
            
        case io_VID_Hires_ON:
            videoMode.hires = 1;
            break;
            
            
        case io_PDL0:
        case io_PDL1:
        case io_PDL2:
        case io_PDL3:
//            if ( RAM[addr] > 127 ) {
//                RAM[addr]--;
//            }
            return RAM[addr];
            
        case io_MEM_RDRAM_NOWR_2:
        case io_MEM_RDROM_WRAM_2:
        case io_MEM_RDROM_NOWR_2:
        case io_MEM_RDRAM_WRAM_2:
        case io_MEM_RDRAM_NOWR_1:
        case io_MEM_RDROM_WRAM_1:
        case io_MEM_RDROM_NOWR_1:
        case io_MEM_RDRAM_WRAM_1:
            if ( MEMcfg.RAM_16K || MEMcfg.RAM_128K ) {
                uint8_t * RAM_BANK = Apple2_16K_RAM;
                
                // RAM Bank 1 or 2?
                switch (addr) {
                    case io_MEM_RDRAM_NOWR_2:
                    case io_MEM_RDROM_WRAM_2:
                    case io_MEM_RDROM_NOWR_2:
                    case io_MEM_RDRAM_WRAM_2:
                        MEMcfg.RAM_BANK_2 = 1;
                        RAM_BANK = Apple2_16K_RAM + 0x30;
                        break;
                        
                    default:
                        MEMcfg.RAM_BANK_2 = 0;
                        RAM_BANK = Apple2_16K_RAM;
                        break;
                }

                // is RAM to read or ROM?
                switch (addr) {
                    case io_MEM_RDRAM_NOWR_2:
                    case io_MEM_RDRAM_WRAM_2:
                    case io_MEM_RDRAM_NOWR_1:
                    case io_MEM_RDRAM_WRAM_1:
                        MEMcfg.RD_RAM = 1;
                        // set the RAM extension to read on the upper memory area
                        SWITCH_RAM_PAGE16( RAM_PG_RD_TBL, 0xD0, RAM_BANK,       0x00 );
                        SWITCH_RAM_PAGE16( RAM_PG_RD_TBL, 0xE0, Apple2_16K_RAM, 0x10 );
                        SWITCH_RAM_PAGE16( RAM_PG_RD_TBL, 0xF0, Apple2_16K_RAM, 0x20 );
                        break;
                        
                    default:
                        MEMcfg.RD_RAM = 0;
                        // set the ROM to read on the upper memory area
                        SWITCH_RAM_PAGE16( RAM_PG_RD_TBL, 0xD0, Apple2_12K_ROM, 0x00 );
                        SWITCH_RAM_PAGE16( RAM_PG_RD_TBL, 0xE0, Apple2_12K_ROM, 0x10 );
                        SWITCH_RAM_PAGE16( RAM_PG_RD_TBL, 0xF0, Apple2_12K_ROM, 0x20 );
                        break;
                }

                // is RAM Writeable?
                switch (addr) {
                    case io_MEM_RDROM_WRAM_2:
                    case io_MEM_RDRAM_WRAM_2:
                    case io_MEM_RDROM_WRAM_1:
                    case io_MEM_RDRAM_WRAM_1:
                        MEMcfg.WR_RAM = 1;
                        // set the RAM extension to read from the upper memory area
                        SWITCH_RAM_PAGE16( RAM_PG_WR_TBL, 0xD0, RAM_BANK,       0x00 );
                        SWITCH_RAM_PAGE16( RAM_PG_WR_TBL, 0xE0, Apple2_16K_RAM, 0x10 );
                        SWITCH_RAM_PAGE16( RAM_PG_WR_TBL, 0xF0, Apple2_16K_RAM, 0x20 );
                        break;
                        
                    default:
                        MEMcfg.WR_RAM = 0;
                        // set the ROM to read on the upper memory area
                        SWITCH_RAM_PAGE16( RAM_PG_WR_TBL, 0xD0, Apple2_Dummy_Page, 0 );
                        SWITCH_RAM_PAGE16( RAM_PG_WR_TBL, 0xE0, Apple2_Dummy_Page, 0 );
                        SWITCH_RAM_PAGE16( RAM_PG_WR_TBL, 0xF0, Apple2_Dummy_Page, 0 );
                        break;
                }
                
            } // if there is RAM expansion card installed
            break;

        // TODO: Make code "card insertable to slot" / aka slot independent and dynamically add/remove
        case io_DISK_PHASE0_OFF + SLOT6:
        case io_DISK_PHASE1_OFF + SLOT6:
        case io_DISK_PHASE2_OFF + SLOT6:
        case io_DISK_PHASE3_OFF + SLOT6:
            currentMagnet = (addr - io_DISK_PHASE0_OFF - SLOT6) / 2;
            disk.phase.magnet &= ~(1 << currentMagnet);
            printf("io_DISK_PHASE%u_OFF (S%u, ps:%X) ", currentMagnet, 6, disk.phase.magnet);

            disk_phase();
            return 0;

        case io_DISK_PHASE0_ON + SLOT6:
        case io_DISK_PHASE1_ON + SLOT6:
        case io_DISK_PHASE2_ON + SLOT6:
        case io_DISK_PHASE3_ON + SLOT6: {
            currentMagnet = (addr - io_DISK_PHASE0_ON - SLOT6) / 2;
            disk.phase.magnet |= 1 << currentMagnet;
            printf("io_DISK_PHASE%u_ON (S%u, ps:%X) ", currentMagnet, 6, disk.phase.magnet);

            disk_phase();
            return 0;
        }

        case io_DISK_POWER_OFF + SLOT6:
            dbgPrintf2("io_DISK_POWER_OFF (S%u)\n", 6);
            return 0;

        case io_DISK_POWER_ON + SLOT6:
            dbgPrintf2("io_DISK_POWER_ON (S%u)\n", 6);
            return 0;

        case io_DISK_SELECT_1 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_1 (S%u)\n", 6);
            return 0;

        case io_DISK_SELECT_2 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_2 (S%u)\n", 6);
            return 0;

        case io_DISK_READ + SLOT6:
            return disk_read();
          
            
        case io_DISK_WRITE + SLOT6:
            dbgPrintf2("io_DISK_WRITE (S%u)\n", 6);
            return 0;

        case io_DISK_CLEAR + SLOT6:
            dbgPrintf2("io_DISK_CLEAR (S%u)\n", 6);
            return 0;

        case io_DISK_SHIFT + SLOT6:
            dbgPrintf2("io_DISK_SHIFT (S%u)\n", 6);
            return 0;

            
            
        default:
            //printf("mmio read:%04X\n", addr);
            break;
    }
    
    return RAM[addr];
}


void setIO ( uint16_t ioaddr, uint8_t val ) {
    RAM[ioaddr] = val;
}


void kbdInput ( uint8_t code ) {
//    printf("kbdInput: %02X ('%c')\n", code, isprint(code) ? code : ' ');
    switch ( code ) {
//        case '\n':
//            code = 0x0D;
//            break;
//
        case 0x7F: // BackSlash
            code = 0x08;
            break;
            
        default:
            break;
    }
    
    code |= 0x80;
    
    for( int i = 1000; i && ( RAM[io_KBD] > 0x7F ); --i ) {
        usleep(10);
    }

    RAM[io_KBD] = code;
}


INLINE void ioWrite( uint16_t addr, uint8_t val ) {
    //    printf("mmio:%04X\n", addr);
    switch (addr) {
        case io_KBD:
            break;
            
        case io_KBDSTRB:
            RAM[io_KBD] &= 0x7F;
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
INLINE uint8_t memread8( uint16_t addr ) {
    return * ( RAM_PG_RD_TBL[addr >> 8] + (addr & 0xFF) );
//    return RAM[ addr ];
}
/**
 Naive implementation of RAM read from address
 **/
INLINE uint16_t memread16( uint16_t addr ) {
    return * (uint16_t*) ( RAM_PG_RD_TBL[addr >> 8] + (addr & 0xFF) );
//    return * (uint16_t*) (& RAM[ addr ]);
}

INLINE uint8_t memread( uint16_t addr ) {
    if ( (addr >= 0xC000) && (addr <= 0xC0FF) ) {
        return ioRead(addr);
    }
    
    return memread8(addr);
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
static  void memwrite( uint16_t addr, uint8_t byte ) {
//    if ( addr >= 0xD000 ) {
//        // ROM
//        return;
//    }
//    if ( addr >= 0xC000 ) {
//        return mmioWrite(addr);
//    }
//
    
    RAM[ addr ] = byte;
}


/**
 Fetching 1 byte from memory address pc (program counter)
 increase pc by one
 **/
INLINE uint8_t fetch() {
    disHexB( disassembly.pOpcode, RAM[m6502.PC] );
#ifdef CLK_ABSOLUTE_PRECISE
    if ( (m6502.PC & 0xFF) >= 0xFF ) {
        m6502.clktime++;
    }
#endif
    return memread( m6502.PC++ );
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
    uint16_t addr = addr_abs();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
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
    uint16_t addr = addr_abs_X();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
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
    uint16_t addr = addr_abs_Y();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
}

INLINE uint16_t imm() {
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
    return memread8(addr_zp());
}
INLINE uint8_t * dest_zp() {
    uint16_t addr = addr_zp();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
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
    uint16_t addr = addr_X_ind();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
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
    uint16_t addr = addr_ind_Y();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
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
    return memread8(addr_zp_X());
}
INLINE uint8_t * dest_zp_X() {
    uint16_t addr = addr_zp_X();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
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
    return memread8(addr_zp_Y());
}
INLINE uint8_t * dest_zp_Y() {
    uint16_t addr = addr_zp_Y();
    return ( RAM_PG_WR_TBL[addr >> 8] + (addr & 0xFF) );
}


#endif // __APPLE2_MMIO_H__


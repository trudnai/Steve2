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


uint8_t Apple2_64K_RAM[ 64 * KB ] = {0};
uint8_t * RAM = Apple2_64K_RAM;

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


enum mmio {
    io_KBD              = 0xC000,
    io_KBDSTRB          = 0xC010,
    
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
    
};


#define PAGESIZE 256
#define PAGES 16

//uint8_t ram_0[PAGESIZE];
//uint8_t ram_1[PAGESIZE];
//uint8_t ram_2[PAGESIZE];
//uint8_t ram_3[PAGESIZE];
//uint8_t ram_4[PAGESIZE];
//uint8_t ram_5[PAGESIZE];
//uint8_t ram_6[PAGESIZE];
//uint8_t ram_7[PAGESIZE];
//uint8_t ram_8[PAGESIZE];
//uint8_t ram_9[PAGESIZE];
//uint8_t ram_A[PAGESIZE];
//uint8_t ram_B[PAGESIZE];
//uint8_t aui_C[PAGESIZE];
//uint8_t rom_D[PAGESIZE];
//uint8_t rom_E[PAGESIZE];
//uint8_t rom_F[PAGESIZE];
//
//uint8_t * ram[PAGES] = {
//    ram_0,
//    ram_1,
//    ram_2,
//    ram_3,
//    ram_4,
//    ram_5,
//    ram_6,
//    ram_7,
//    ram_8,
//    ram_9,
//    ram_A,
//    ram_B,
//    aui_C,
//    rom_D,
//    rom_E,
//    rom_F,
//};

//uint8_t ( * mmio_read [ 64 * KB ] )( uint16_t addr );

typedef union address16_u {
    uint16_t addr;
    struct {
        uint8_t offs;
        uint8_t page;
    };
} address16_t;


#define CASE_DISKII(x) \
    case io_DISK_PHASE0_OFF + SLOT##x: \
        printf("io_DISK_PHASE0_OFF (S%u)\n", x); \
        return 0; \
    case io_DISK_PHASE0_ON + SLOT##x: \
        printf("io_DISK_PHASE0_ON (S%u)\n", x); \
        return 0; \
    case io_DISK_PHASE1_OFF + SLOT##x: \
        printf("io_DISK_PHASE1_OFF (S%u)\n", x); \
        return 0; \
    case io_DISK_PHASE1_ON + SLOT##x: \
        printf("io_DISK_PHASE1_ON (S%u)\n", x); \
        return 0; \
    case io_DISK_PHASE2_OFF + SLOT##x: \
        printf("io_DISK_PHASE2_OFF (S%u)\n", x); \
        return 0; \
    case io_DISK_PHASE2_ON + SLOT##x: \
        printf("io_DISK_PHASE2_ON (S%u)\n", x); \
        return 0; \
    case io_DISK_PHASE3_OFF + SLOT##x: \
        printf("io_DISK_PHASE3_OFF (S%u)\n", x); \
        return 0; \
    case io_DISK_PHASE3_ON + SLOT##x: \
        printf("io_DISK_PHASE3_ON (S%u)\n", x); \
        return 0; \
    case io_DISK_POWER_OFF + SLOT##x: \
        printf("io_DISK_POWER_OFF (S%u)\n", x); \
        return 0; \
    case io_DISK_POWER_ON + SLOT##x: \
        printf("io_DISK_POWER_ON (S%u)\n", x); \
        return 0; \
    case io_DISK_SELECT_1 + SLOT##x: \
        printf("io_DISK_SELECT_1 (S%u)\n", x); \
        return 0; \
    case io_DISK_SELECT_2 + SLOT##x: \
        printf("io_DISK_SELECT_2 (S%u)\n", x); \
        return 0; \
    case io_DISK_READ + SLOT##x: \
        printf("io_DISK_READ (S%u)\n", x); \
        return 0; \
    case io_DISK_WRITE + SLOT##x: \
        printf("io_DISK_WRITE (S%u)\n", x); \
        return 0; \
    case io_DISK_CLEAR + SLOT##x: \
        printf("io_DISK_CLEAR (S%u)\n", x); \
        return 0; \
    case io_DISK_SHIFT + SLOT##x: \
        printf("io_DISK_SHIFT (S%u)\n", x); \
        return 0;


static const minDiskPhaseNum = 0;
static const maxDiskPhaseNum = 80;
static int trackPhase = maxDiskPhaseNum;

struct phase_t {
    uint8_t current : 2;
    uint8_t last    : 2;
} phase = {0};

static const int8_t phaseTransition[4][4] = {
    {  0, -1,  0, +1 },
    { +1,  0, -1,  0 },
    {  0, +1,  0, -1 },
    { -1,  0, +1,  0 },
};




INLINE uint8_t ioRead( uint16_t addr ) {
    dbgPrintf("mmio read:%04X\n", addr);
    
    switch (addr) {
        case io_KBD:
//            if ( RAM[io_KBD] > 0x7F ) printf("io_KBD:%04X\n", addr);
            return RAM[io_KBD];

        case io_KBDSTRB:
            // TODO: This is very slow!
//            printf("io_KBDSTRB\n");
            return RAM[io_KBD] &= 0x7F;

//        CASE_DISKII(6)

        // TODO: Make code "card insertable to slot" / aka slot independent and dynamically add/remove
        case io_DISK_PHASE0_OFF + SLOT6:
        case io_DISK_PHASE1_OFF + SLOT6:
        case io_DISK_PHASE2_OFF + SLOT6:
        case io_DISK_PHASE3_OFF + SLOT6:
            dbgPrintf2("io_DISK_PHASE%u_OFF (S%u)\n", phase.current, 6);
            phase.last = phase.current;
            return 0;
            
        case io_DISK_PHASE0_ON + SLOT6:
        case io_DISK_PHASE1_ON + SLOT6:
        case io_DISK_PHASE2_ON + SLOT6:
        case io_DISK_PHASE3_ON + SLOT6: {
            phase.current = (addr - io_DISK_PHASE0_ON - SLOT6) / 2;
            trackPhase += phaseTransition[phase.current][phase.last];
            if ( trackPhase < minDiskPhaseNum ) {
                trackPhase = minDiskPhaseNum;
            }
            if ( trackPhase > maxDiskPhaseNum ) {
                trackPhase = maxDiskPhaseNum;
            }
            dbgPrintf2("io_DISK_PHASE%u_ON (S%u, trk:%u)\n", phase.current, 6, trackPhase);
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
            dbgPrintf("io_DISK_READ (S%u)\n", 6);
            int track = woz_tmap.phase[trackPhase];
            if (trackOffset >= WOZ_TRACK_BYTE_COUNT ) {
                trackOffset = 0;
            }
            printf("offs:%u\n", trackOffset);
            return woz_trks[track].data[trackOffset++];
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
            return RAM[addr];
    }
    
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
    
    while ( RAM[io_KBD] > 0x7F ) {
        usleep(10);
    }

    RAM[io_KBD] = code;
}


INLINE void ioWrite( uint16_t addr, uint8_t val ) {
    //    printf("mmio:%04X\n", addr);
    switch (addr) {
        case io_KBD:
            return;
            
        default:
            break;
    }
    return;
}

/**
 Naive implementation of RAM read from address
 **/

INLINE uint8_t memread_zp( uint8_t addr ) {
    return RAM[ addr ];
}

/**
 Naive implementation of RAM read from address
 **/
INLINE uint8_t memread8( uint16_t addr ) {
//    if ( addr == 0xD2AD ) {
//        dbgPrintf("OUT OF MEMORY!\n");
//    }
    

    return RAM[ addr ];
}
/**
 Naive implementation of RAM read from address
 **/
INLINE uint16_t memread16( uint16_t addr ) {
    return * (uint16_t*) (& RAM[ addr ]);
}

INLINE uint8_t memread( uint16_t addr ) {
//    switch ( ((address16_t)addr).page ) {
//        case 0xC0:
//        case 0xC1:
//        case 0xC2:
//        case 0xC3:
//        case 0xC4:
//        case 0xC5:
//        case 0xC6:
//        case 0xC7:
//        case 0xC8:
//        case 0xC9:
//        case 0xCA:
//        case 0xCB:
//        case 0xCC:
//        case 0xCD:
//        case 0xCE:
//        case 0xCF:
//            return ioRead(addr);
//
//        defaut:
//            break;
//    }

    if ( (addr >= 0xC000) && (addr < 0xC0FF) ) {
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
static  void memwrite_zp( uint8_t addr, uint8_t byte ) {
    RAM[ addr ] = byte;
}


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
    return memread( m6502.PC++ );
}

/**
 Fetching 2 bytes as a 16 bit number from memory address pc (program counter)
 increase pc by one
 **/
INLINE uint16_t fetch16() {
    uint16_t word = memread16( m6502.PC );
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
    return & RAM[ addr_abs() ];
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
    return memread16( fetch16() );
}

/**
 abs,X        ....    absolute, X-indexed         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
INLINE uint16_t addr_abs_X() {
    dbgPrintf("abs,X:%04X(%02X) ", *((uint16_t*)&RAM[m6502.PC]) + m6502.X, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.X]);
    disPrintf(disassembly.oper, "$%04X,X", memread16(m6502.PC))
    return fetch16() + m6502.X;
}
INLINE uint8_t src_abs_X() {
    return memread( addr_abs_X() );
}
INLINE uint8_t * dest_abs_X() {
    return & RAM[ addr_abs_X() ];
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
    return & RAM[ addr_abs_Y() ];
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
    return memread_zp(addr_zp());
}
INLINE uint8_t * dest_zp() {
    return & RAM[ addr_zp() ];
}

/**
 get a 16 bit address from the zp:zp+1
 **/
INLINE uint16_t addr_zp_ind( uint8_t addr ) {
    dbgPrintf("zpi:%02X:%04X(%02X) ", RAM[m6502.PC], *((uint16_t*)&RAM[m6502.PC]), RAM[*((uint16_t*)&RAM[m6502.PC])]);
    disPrintf(disassembly.oper, "($%02X) {$%04X}", memread8(m6502.PC), memread16( memread8(m6502.PC) ) )
    return memread16(addr);
}

/**
 X,ind        ....    X-indexed, indirect         OPC ($LL,X)
 operand is zeropage address;
 effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
 **/
INLINE uint16_t addr_X_ind() {
    dbgPrintf("zpXi:%02X:%04X(%02X) ", RAM[m6502.PC], *((uint16_t*)&RAM[m6502.PC]) + m6502.X, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.X]);
    disPrintf(disassembly.oper, "($%02X,X) {$%04X}", memread8(m6502.PC), memread16( memread8(m6502.PC) + m6502.X) )
    return memread16( fetch() + m6502.X );
}
INLINE uint8_t src_X_ind() {
    return memread( addr_X_ind() );
}
INLINE uint8_t * dest_X_ind() {
    return & RAM[ addr_X_ind() ];
}

/**
 ind,Y        ....    indirect, Y-indexed         OPC ($LL),Y
 operand is zeropage address;
 effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
 **/
INLINE uint16_t addr_ind_Y() {
    //    uint8_t a = fetch();
    //    dbgPrintf("addr_ind_Y: %04X + %02X = %04X ", addr_zpg_ind( a ), m6502.Y, addr_zpg_ind( a ) + m6502.Y);
    disPrintf(disassembly.oper, "($%02X),Y {$%04X}", memread8(m6502.PC), memread16( memread8(m6502.PC) ) + m6502.Y)
    return memread16( fetch() ) + m6502.Y;
}
INLINE uint8_t src_ind_Y() {
    return memread( addr_ind_Y() );
}
INLINE uint8_t * dest_ind_Y() {
    uint16_t addr = addr_ind_Y();
//    if ( (addr >= 0xC000) && (addr <= 0xC0FF) ) {
//        addr = 0xC111;
//    }
    //    return & RAM[ addr_abs_Y() ];
    return & RAM[ addr ];
//    return & RAM[ addr_ind_Y() ];
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
    return memread_zp(addr_zp_X());
}
INLINE uint8_t * dest_zp_X() {
    return & RAM[ addr_zp_X() ];
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
    return memread_zp(addr_zp_Y());
}
INLINE uint8_t * dest_zp_Y() {
    return & RAM[ addr_zp_Y() ];
}


#endif // __APPLE2_MMIO_H__


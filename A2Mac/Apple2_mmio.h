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


enum mmio {
    io_KBD = 0xC000,
    io_KBDSTRB = 0xC010,
};


uint8_t RAM[ 64 * KB ] = {0};

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


static inline uint8_t ioRead( uint16_t addr ) {
//    printf("mmio:%04X\n", addr);
    switch (addr) {
        case io_KBD:
            return RAM[addr];
            
        case io_KBDSTRB:
            return RAM[io_KBD] &= 0x7F;

        default:
            break;
    }
    return 0;
}

static inline void ioWrite( uint16_t addr ) {
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

static inline uint8_t memread_zp( uint8_t addr ) {
    return RAM[ addr ];
}

static inline uint8_t memread( uint16_t addr ) {
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

    if ( (addr >= 0xC000) && (addr < 0xD000) ) {
        ioRead(addr);
    }
    
    return RAM[ addr ];
}

/**
 Naive implementation of RAM read from address
 **/
static inline uint16_t memread16( uint16_t addr ) {
//    if ( ( addr >= 0xC000 ) && ( addr < 0xD000 ) ) {
//        return mmioRead(addr);
//    }

//    dbgPrintf("%04X ", * (uint16_t*) (& RAM[ addr ]));
    return * (uint16_t*) (& RAM[ addr ]);
}

/**
 Naive implementation of RAM read from address
 **/
//static inline uint16_t memioread16( uint16_t addr ) {
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
static inline uint8_t fetch() {
    dbgPrintf("%02X ", RAM[m6502.pc]);
    return memread( m6502.pc++ );
}

/**
 Fetching 2 bytes as a 16 bit number from memory address pc (program counter)
 increase pc by one
 **/
static inline uint16_t fetch16() {
    dbgPrintf("%04X ", memread16(m6502.pc));
    uint16_t word = memread16( m6502.pc );
    m6502.pc += 2;
    return word;
}

/**
 get a 16 bit address from the zp:zp+1
 **/
static inline uint16_t addr_zp_ind( uint8_t addr ) {
    return memread16(addr);
}

/**
 X,ind        ....    X-indexed, indirect         OPC ($LL,X)
 operand is zeropage address;
 effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
 **/
static inline uint16_t addr_X_ind() {
    return addr_zp_ind( fetch() + m6502.X );
}

/**
 ind,Y        ....    indirect, Y-indexed         OPC ($LL),Y
 operand is zeropage address;
 effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
 **/
static inline uint16_t addr_ind_Y() {
    uint8_t a = fetch();
//    dbgPrintf("addr_ind_Y: %04X + %02X = %04X ", addr_zpg_ind( a ), m6502.Y, addr_zpg_ind( a ) + m6502.Y);
    return addr_zp_ind( a ) + m6502.Y;
}

/**
 abs,X        ....    absolute, X-indexed         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
static inline uint16_t addr_abs_X() {
    return fetch16() + m6502.X;
}

/**
 abs,Y        ....    absolute, Y-indexed         OPC $LLHH,Y
 operand is address; effective address is address incremented by Y with carry **
 **/
static inline uint16_t addr_abs_Y() {
    return fetch16() + m6502.Y;
}

/**
 zpg        ....    zeropage         OPC $LL
 operand is zeropage address (hi-byte is zero, address = $00LL)
 **/
static inline uint16_t addr_zp() {
    return fetch();
}

/**
 zpg,X        ....    zeropage, X-indexed         OPC $LL,X
 operand is zeropage address;
 effective address is address incremented by X without carry **
 **/
static inline uint16_t addr_zp_X() {
    return addr_zp() + m6502.X;
}

/**
 zpg,Y        ....    zeropage, Y-indexed         OPC $LL,Y
 operand is zeropage address;
 effective address is address incremented by Y without carry **
 **/
static inline uint16_t addr_zp_Y() {
    return addr_zp() + m6502.Y;
}


#endif // __APPLE2_MMIO_H__


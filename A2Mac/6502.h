//
//  6502.h
//  6502
//
//  Created by Tamas Rudnai on 7/22/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_H__
#define __6502_H__

#import "stdint.h"

#ifdef DEBUG
#define dbgPrintf(format, ...) printf (format, ## __VA_ARGS__)
#else
#define dbgPrintf(format, ...)
#endif


typedef struct m6502_s {
    uint8_t  A;             // Accumulator
    uint8_t  X;             // X index register
    uint8_t  Y;             // Y index register
    union {
        uint8_t  instr;         // Instruction
        struct {
            uint8_t aaa:3;
            uint8_t bbb:3;
            uint8_t cc:2;
        };
    };
    union {
        uint8_t sr;         // Status Register as 1 byte
        struct {
            uint8_t N:1;    // Negative Flag
            uint8_t V:1;    // Overflow Flag ???
            uint8_t B:2;    // B Flag
            uint8_t D:1;    // Decimal Flag
            uint8_t I:1;    // Interrupt Flag
            uint8_t Z:1;    // Zero Flag
            uint8_t C:1;    // Carry Flag
        } flags;            // Status Register
    };
    uint16_t pc;            // Program Counter
    uint16_t sp;            // Stack Pointer
    unsigned clk;           // Clock Counter
} m6502_s;


extern m6502_s m6502;
extern uint8_t RAM[ 64 * 1024 ];

extern void tst6502();


#endif /* __6502_H__ */

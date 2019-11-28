//
//  6502.h
//  6502
//
//  Created by Tamas Rudnai on 7/22/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_H__
#define __6502_H__

#import <stdint.h>

#ifdef DEBUG
//#define dbgPrintf(format, ...) printf (format, ## __VA_ARGS__)
#define dbgPrintf(format, ...)
#define dbgPrintf2(format, ...) printf (format, ## __VA_ARGS__)
#else
#define dbgPrintf(format, ...)
#define dbgPrintf2(format, ...)
#endif

typedef enum {
    NO_INT,
    HLT,
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

typedef struct m6502_s {
    uint8_t  A;             // Accumulator
    uint8_t  X;             // X index register
    uint8_t  Y;             // Y index register
//    union {
//        uint8_t  instr;         // Instruction
//        struct {
//            uint8_t cc:2;
//            uint8_t bbb:3;
//            uint8_t aaa:3;
//        };
//    };
    union {
        uint8_t SR;         // Status Register
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
    };
    uint16_t PC;            // Program Counter
    uint8_t SP;             // Stack Pointer ( stack addr = 0x01 + sp )
    unsigned clk;           // Clock Counter
    debugLevel_t dbgLevel;  // 0: No Debug, 1: Disassembly Only, 2: Run till BRK, 3: StepByStep
    
    union {
        unsigned int IF;             // interrut flag
        interrupt_t interrupt;
    };
} m6502_t;


typedef struct disassembly_s {
    char addr[5];                   // 4 digits + \0
    char opcode[4 * 3 + 1];         // max 4 bytes * (2 digits + 1 space) + \0
    char * pOpcode;                 // pointer for opcode string builder
    char inst[6 + 1];               // 3 char (unknown instr? -- give it 6 chars) + \0
    char oper[14 + 2 + 1 + 1 + 1];   // 4 digits + 2 brackets + 1 comma + 1 index + \0
    char comment[256];              // to be able to add some comments
} disassembly_t;


extern m6502_t m6502;
extern uint8_t * RAM;
extern uint32_t * videoMemPtr;

extern void hires_Update(void);

extern double mips;
extern double mhz;
extern const unsigned int fps;

extern void tst6502(void);
extern void m6502_ColdReset(void);
extern void m6502_Run(void);
extern void kbdInput ( uint8_t code );

#endif /* __6502_H__ */

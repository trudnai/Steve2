//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//


#ifndef __6502_INSTR_LOAD_STORE_H__
#define __6502_INSTR_LOAD_STORE_H__

#include <ctype.h>

/**
 LDA  Load Accumulator with Memory
 
 M -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     LDA #oper     A9    2     2
 zeropage      LDA oper      A5    2     3
 zeropage,X    LDA oper,X    B5    2     4
 absolute      LDA oper      AD    3     4
 absolute,X    LDA oper,X    BD    3     4*
 absolute,Y    LDA oper,Y    B9    3     4*
 (indirect,X)  LDA (oper,X)  A1    2     6
 (indirect),Y  LDA (oper),Y  B1    2     5*
**/
static inline void LDA( uint8_t imm ) {
    dbgPrintf("LDA ");
    m6502.A = imm;
    set_flags_NZ(imm);
}

/**
 LDX  Load Index X with Memory
 
 M -> X                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     LDX #oper     A2    2     2
 zeropage      LDX oper      A6    2     3
 zeropage,Y    LDX oper,Y    B6    2     4
 absolute      LDX oper      AE    3     4
 absolute,Y    LDX oper,Y    BE    3     4*
**/
static inline void LDX( uint8_t imm ) {
    dbgPrintf("LDX ");
    m6502.X = imm;
    set_flags_NZ(imm);
}

/**
 LDY  Load Index Y with Memory
 
 M -> Y                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     LDY #oper     A0    2     2
 zeropage      LDY oper      A4    2     3
 zeropage,X    LDY oper,X    B4    2     4
 absolute      LDY oper      AC    3     4
 absolute,X    LDY oper,X    BC    3     4*
**/
static inline void LDY( uint8_t imm ) {
    dbgPrintf("LDY ");
    m6502.Y = imm;
    set_flags_NZ(imm);
}


char * charConv =
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?"
;


/**
 STR   Store Data in Destination
 
 (not a real instruction, only a helper function)
**/
static inline void STR( uint8_t * dst, uint8_t imm ) {
    *dst = imm;

//    uint16_t v = dst - RAM;
    
//    if ( ( v >= 0x400 ) && ( v < 0x800 ) ) {
//        char c = charConv[imm];
//        if (( imm > ' ' ) && ( c < 0x7F ))
//            printf("%04X: t:%02X '%c'\n", v, imm, isprint(c) ? c : ' ');
//    }
}

/**
 STA  Store Accumulator in Memory
 
 A -> M                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      STA oper      85    2     3
 zeropage,X    STA oper,X    95    2     4
 absolute      STA oper      8D    3     4
 absolute,X    STA oper,X    9D    3     5
 absolute,Y    STA oper,Y    99    3     5
 (indirect,X)  STA (oper,X)  81    2     6
 (indirect),Y  STA (oper),Y  91    2     6
**/
static inline void STA( uint8_t * dst ) {
    dbgPrintf("STA ");
    STR(dst, m6502.A);
}

/**
 STX  Store Index X in Memory
 
 X -> M                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      STX oper      86    2     3
 zeropage,Y    STX oper,Y    96    2     4
 absolute      STX oper      8E    3     4
 **/
static inline void STX( uint8_t * dst ) {
    dbgPrintf("STX ");
    STR(dst, m6502.X);
}

/**
 STY  Sore Index Y in Memory
 
 Y -> M                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      STY oper      84    2     3
 zeropage,X    STY oper,X    94    2     4
 absolute      STY oper      8C    3     4
 **/
static inline void STY( uint8_t * dst ) {
    dbgPrintf("STY ");
    STR(dst, m6502.Y);
}

#endif // __6502_INSTR_LOAD_STORE_H__


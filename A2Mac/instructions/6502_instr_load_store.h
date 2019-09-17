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
INLINE void LDA( uint8_t src ) {
    dbgPrintf("LDA(%02X) ", src);
    set_flags_NZ(m6502.A = src);
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
INLINE void LDX( uint8_t src ) {
    dbgPrintf("LDX(%02X) ", src);
    set_flags_NZ(m6502.X = src);
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
INLINE void LDY( uint8_t src ) {
    dbgPrintf("LDY(%02X) ", src);
    set_flags_NZ(m6502.Y = src);
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
INLINE void STR( uint8_t * dst, uint8_t src ) {
    dbgPrintf("STR [%04X], %02X ", (int)(dst - RAM), src );
    *dst = src;

//    uint16_t addr = dst - RAM;
//    if ( ( addr >= 0x400 ) && ( addr < 0x800 ) ) {
//        char c = charConv[src];
////        if ( c == '?' ) {
////            printf("? SYNTAX ERROR\n");
////        }
//
//        if (( src > ' ' ) && ( c < 0x7F ))
//            printf("*** PRINT: %04X: t:%02X '%c'\n", addr, src, isprint(c) ? c : ' ');
//    }
//    else if ( ( addr >= 0xC000 ) && ( addr < 0xD000 ) ) {
//        printf("mmio write:[%04X] = %02X\n", addr, src);
//    }
//
//
//    else switch ( addr ) {
//        case 0x36:
//        case 0x37:
//            dbgPrintf("*** OUTROUT %04X: %02X\n", addr, src);
//            break;
//
//        case 0x9B:
//        case 0x9C:
//            dbgPrintf("*** LOWTR %04X: %02X\n", addr, src);
//            break;
//
//        case 0x6F: // FRETOP
//        case 0x70:
//            dbgPrintf("*** FRETOP %04X: %02X\n", addr, src);
//            break;
//
//        case 0x73: // MEMSIZ
//        case 0x74:
//            dbgPrintf("*** MEMSIZ %04X: %02X\n", addr, src);
//            break;
//
//        case 0x5E:
//            dbgPrintf("*** ??? %04X: %02X\n", addr, src);
//            break;
//            
//        default:
//            break;
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
INLINE void STA( uint8_t * dst ) {
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
INLINE void STX( uint8_t * dst ) {
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
INLINE void STY( uint8_t * dst ) {
    dbgPrintf("STY ");
    STR(dst, m6502.Y);
}

#endif // __6502_INSTR_LOAD_STORE_H__


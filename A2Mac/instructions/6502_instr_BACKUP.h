//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>

#include <time.h>

#define K 1000ULL
#define M (K * K)
#define G (M * K)
#define T (G * K)




unsigned long long int clktime = 0;


enum mmio {
    ioSomething = 0xC000,
};


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

m6502_s m6502 = {0};

uint8_t mem[64*1024] = {0};
uint8_t ( * mmio_read [64*1024] )( uint16_t addr );


typedef
union {
    uint8_t bits;
    struct {
        uint8_t b7:1;
        uint8_t b6:1;
        uint8_t b5:1;
        uint8_t b4:1;
        uint8_t b3:1;
        uint8_t b2:1;
        uint8_t b1:1;
        uint8_t b0:1;
    };
} bits_t;

#define BITTEST(n,x) ((bits_t)(n)).b##x;

static inline void set_flags_NZ( uint8_t imm ) {
    m6502.flags.N = BITTEST(imm, 7);
    m6502.flags.Z = imm == 0;
}

static inline void set_flags_NVZ( uint8_t imm ) {
    set_flags_NZ(imm);
    m6502.flags.V = BITTEST(imm, 6);
}

static inline void set_flags_NZC( int test ) {
    m6502.flags.N = test  < 0;
    m6502.flags.Z = test == 0;
    m6502.flags.C = test  > 0xFF;
}

static inline uint8_t mmioRead( uint16_t addr ) {
    switch (addr) {
        case ioSomething:
            return 123;
            
        default:
            break;
    }
    return 0;
}

/**
 Naive implementation of RAM read from address
 **/
static inline uint8_t memread( uint16_t addr ) {

    if ( ( addr >= 0xC000 ) && ( addr < 0xD000 ) ) {
        return mmioRead(addr);
    }
    
//    if ( addr < 0xC000 )
//        return mem[ addr ];
//    if ( addr < 0xD000 )
//        return mmioRead(addr);

//    return ROMRead(addr);
    return mem[ addr ];
}
//#define memread(a) mem[a]


/**
 Naive implementation of RAM read from address
 **/
static inline uint16_t memread16( uint16_t addr ) {
    if ( addr < 0xC000 )
        return * (uint16_t*) (& mem[ addr ]);
    if ( addr < 0xD000 )
        return * (uint16_t*) (& mem[ addr ]);

    return * (uint16_t*) (& mem[ addr ]);
}
//#define memread16(a) * (uint16_t*) (& mem[a])

/**
 Naive implementation of RAM read from address
 **/
//static inline uint16_t memioread16( uint16_t addr ) {
//    return (uint16_t)mmio_read[ addr ](addr);
//}

/**
 Naive implementation of RAM write to address
 **/
static  void memwrite( uint16_t addr, uint8_t byte ) {
    mem[ addr ] = byte;
}

/**
 Fetching 1 byte from memory address pc (program counter)
 increase pc by one
 **/
static inline uint8_t fetch() {
    //    printf("fetching 0x%04X\n", m6502.pc);
    return memread( m6502.pc++ );
}
//#define fetch(a) memread(a)

/**
 Fetching 2 bytes as a 16 bit number from memory address pc (program counter)
 increase pc by one
 **/
static inline uint16_t fetch16() {
//    return ( (uint16_t)fetch() << 8 ) + fetch();
    uint16_t word = memread16( m6502.pc );
    m6502.pc += 2;
    return word;
}
//#define fetch16(a) memread16(a)

/**
 get a 16 bit address from the zp:zp+1
 **/
static inline uint16_t addr_zpg_ind( uint8_t zpg ) {
    return memread16(zpg);
}

/**
 X,ind        ....    X-indexed, indirect         OPC ($LL,X)
 operand is zeropage address;
 effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
 **/
static inline uint16_t addr_X_ind() {
    return addr_zpg_ind( fetch() + m6502.X );
}

/**
 ind,Y        ....    indirect, Y-indexed         OPC ($LL),Y
 operand is zeropage address;
 effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
 **/
static inline uint16_t addr_ind_Y() {
    return addr_zpg_ind( fetch() ) + m6502.Y;
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
static inline uint16_t addr_zpg() {
    return fetch();
}

/**
 zpg,X        ....    zeropage, X-indexed         OPC $LL,X
 operand is zeropage address;
 effective address is address incremented by X without carry **
 **/
static inline uint16_t addr_zpg_X() {
    return addr_zpg() + m6502.X;
}

/**
 zpg,Y        ....    zeropage, Y-indexed         OPC $LL,Y
 operand is zeropage address;
 effective address is address incremented by Y without carry **
 **/
static inline uint16_t addr_zpg_Y() {
    return addr_zpg() + m6502.Y;
}


static inline void PUSH( uint8_t n ) {
    memwrite( m6502.sp--, n );
}

static inline uint8_t POP() {
    return memread( ++m6502.sp );
}

static inline void PUSH_addr( uint16_t addr ) {
    PUSH( (uint8_t)addr );
    PUSH( (uint8_t)(addr >> 8) );
}

static inline uint16_t POP_addr() {
    return ( POP() << 8 ) + POP();
}


/**
 BRK  Force Break
 
 interrupt,                       N Z C I D V
 push PC+2, push SR               - - - 1 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       BRK           00    1     7
**/
static inline void BRK() {
//    printf("BRK\n");
}

/**
 NOP  No Operation
 
 ---                              N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       NOP           EA    1     2
 **/
static inline void NOP() {
    //    printf("BRK\n");
}


static inline void STR( uint8_t * dst, uint8_t imm ) {
    *dst = imm;
}


/**
 INC  Increment Memory by One
 
 M + 1 -> M                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      INC oper      E6    2     5
 zeropage,X    INC oper,X    F6    2     6
 absolute      INC oper      EE    3     6
 absolute,X    INC oper,X    FE    3     7
**/
static inline void INC( uint8_t * dst ) {
    (*dst)++;
    set_flags_NZ(*dst);
}

/**
 INX  Increment Index X by One
 
 X + 1 -> X                       N Z C I D V
 + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       INX           E8    1     2
**/
static inline void INX() {
    m6502.X++;
    set_flags_NZ(m6502.X);
}

/**
 INY  Increment Index Y by One
 
 Y + 1 -> Y                       N Z C I D V
 + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       INY           C8    1     2
**/
static inline void INY() {
    m6502.Y++;
    set_flags_NZ(m6502.Y);
}

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
    m6502.A = imm;
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
    m6502.X = imm;
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
    m6502.Y = imm;
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
    STR(dst, m6502.Y);
}

/**
 ORA  OR Memory with Accumulator
 
 A OR M -> A                      N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     ORA #oper     09    2     2
 zeropage      ORA oper      05    2     3
 zeropage,X    ORA oper,X    15    2     4
 absolute      ORA oper      0D    3     4
 absolute,X    ORA oper,X    1D    3     4*
 absolute,Y    ORA oper,Y    19    3     4*
 (indirect,X)  ORA (oper,X)  01    2     6
 (indirect),Y  ORA (oper),Y  11    2     5*
**/
static inline void ORA( uint8_t imm ) {
    m6502.A |= imm;
    set_flags_NZ( m6502.A );
}

/**
 ADC  Add Memory to Accumulator with Carry
 
 A + M + C -> A, C                N Z C I D V
                                  + + + - - +
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     ADC #oper     69    2     2
 zeropage      ADC oper      65    2     3
 zeropage,X    ADC oper,X    75    2     4
 absolute      ADC oper      6D    3     4
 absolute,X    ADC oper,X    7D    3     4*
 absolute,Y    ADC oper,Y    79    3     4*
 (indirect,X)  ADC (oper,X)  61    2     6
 (indirect),Y  ADC (oper),Y  71    2     5*
**/
static inline void ADC( uint8_t imm ) {
    m6502.A += imm + m6502.flags.C;
    set_flags_NVZ( m6502.A );
}

/**
 SBC  Subtract Memory from Accumulator with Borrow
 
 A - M - C -> A                   N Z C I D V
                                  + + + - - +
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     SBC #oper     E9    2     2
 zeropage      SBC oper      E5    2     3
 zeropage,X    SBC oper,X    F5    2     4
 absolute      SBC oper      ED    3     4
 absolute,X    SBC oper,X    FD    3     4*
 absolute,Y    SBC oper,Y    F9    3     4*
 (indirect,X)  SBC (oper,X)  E1    2     6
 (indirect),Y  SBC (oper),Y  F1    2     5*
**/
static inline void SBC( uint8_t imm ) {
    int tmp = (int)m6502.A - imm - m6502.flags.C;
    m6502.A = (uint8_t)tmp;
    set_flags_NVZ( tmp );
}

/**
 AND  AND Memory with Accumulator
 
 A AND M -> A                     N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     AND #oper     29    2     2
 zeropage      AND oper      25    2     3
 zeropage,X    AND oper,X    35    2     4
 absolute      AND oper      2D    3     4
 absolute,X    AND oper,X    3D    3     4*
 absolute,Y    AND oper,Y    39    3     4*
 (indirect,X)  AND (oper,X)  21    2     6
 (indirect),Y  AND (oper),Y  31    2     5*
 **/
static inline void AND( uint8_t imm ) {
    m6502.A &= imm;
    set_flags_NZ( m6502.A );
}

/**
 EOR  Exclusive-OR Memory with Accumulator
 
 A EOR M -> A                     N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     EOR #oper     49    2     2
 zeropage      EOR oper      45    2     3
 zeropage,X    EOR oper,X    55    2     4
 absolute      EOR oper      4D    3     4
 absolute,X    EOR oper,X    5D    3     4*
 absolute,Y    EOR oper,Y    59    3     4*
 (indirect,X)  EOR (oper,X)  41    2     6
 (indirect),Y  EOR (oper),Y  51    2     5*
**/
static inline void EOR( uint8_t imm ) {
    m6502.A ^= imm;
    set_flags_NZ( m6502.A );
}


/**
 ASL  Shift Left One Bit (Memory or Accumulator)
 
 C <- [76543210] <- 0             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ASL A         0A    1     2
 zeropage      ASL oper      06    2     5
 zeropage,X    ASL oper,X    16    2     6
 absolute      ASL oper      0E    3     6
 absolute,X    ASL oper,X    1E    3     7
**/
static inline void ASL( uint8_t * dst ) {
    m6502.flags.C = (*dst & (1<<7)) >> 7;
    *dst <<= 1;
    set_flags_NZ( *dst );
}

/**
 LSR  Shift One Bit Right (Memory or Accumulator)
 
 0 -> [76543210] -> C             N Z C I D V
                                  0 + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   LSR A         4A    1     2
 zeropage      LSR oper      46    2     5
 zeropage,X    LSR oper,X    56    2     6
 absolute      LSR oper      4E    3     6
 absolute,X    LSR oper,X    5E    3     7
**/
static inline void LSR( uint8_t * dst ) {
    m6502.flags.C = *dst & 1;
    *dst >>= 1;
    set_flags_NZ( *dst );
}

/**
 PHA  Push Accumulator on Stack
 
 push A                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHA           48    1     3
**/
static inline void PHA() {
    PUSH( m6502.A );
}

/**
 PLA  Pull Accumulator from Stack
 
 pull A                           N Z C I D V
 + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLA           68    1     4
 **/
static inline void PLA() {
     m6502.A = POP();
}

/**
 PHP  Push Processor Status on Stack
 
 push SR                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHP           08    1     3
**/
static inline void PHP() {
    PUSH( m6502.sr );
}

/**
 PLP  Pull Processor Status from Stack
 
 pull SR                          N Z C I D V
                                  from stack
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLP           28    1     4
**/
static inline void PLP() {
    m6502.sr = POP();
}


static inline void BR( int8_t reladdr ) {
    m6502.pc += reladdr;
}


/**
 BNE  Branch on Result not Zero
 
 branch on Z = 0                  N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BNE oper      D0    2     2**
**/
static inline void BNE( int8_t reladdr ) {
    if ( m6502.flags.Z == 0 )
        BR( reladdr );
}

/**
 BEQ  Branch on Result Zero
 
 branch on Z = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BEQ oper      F0    2     2**
**/
static inline void BEQ( int8_t reladdr ) {
    if ( m6502.flags.Z == 1 )
        BR( reladdr );
}

/**
 BPL  Branch on Result Plus
 
 branch on N = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BPL oper      10    2     2**
**/
static inline void BPL( int8_t reladdr ) {
    if ( m6502.flags.N == 0 )
        BR( reladdr );
}

/**
 BMI  Branch on Result Minus
 
 branch on N = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BMI oper      30    2     2**
**/
static inline void BMI( int8_t reladdr ) {
    if ( m6502.flags.N == 1 )
        BR( reladdr );
}

/**
 BVC  Branch on Overflow Clear
 
 branch on V = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BVC oper      50    2     2**
**/
static inline void BVC( int8_t reladdr ) {
    if ( m6502.flags.V == 0 )
        BR( reladdr );
}

/**
 BVS  Branch on Overflow Set
 
 branch on V = 1                  N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BVC oper      70    2     2**
**/
static inline void BVS( int8_t reladdr ) {
    if ( m6502.flags.V == 1 )
        BR( reladdr );
}

/**
 BCC  Branch on Carry Clear
 
 branch on C = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BCC oper      90    2     2**
**/
static inline void BCC( int8_t reladdr ) {
    if ( m6502.flags.C == 0 )
        BR( reladdr );
}

/**
 BCS  Branch on Carry Set
 
 branch on C = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BCS oper      B0    2     2**
**/
static inline void BCS( int8_t reladdr ) {
    if ( m6502.flags.C == 1 )
        BR( reladdr );
}

/**
 CLC  Clear Carry Flag
 
 0 -> C                           N Z C I D V
                                  - - 0 - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLC           18    1     2
**/
static inline void CLC() {
    m6502.flags.C = 0;
}

/**
 SEC  Set Carry Flag
 
 1 -> C                           N Z C I D V
                                  - - 1 - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEC           38    1     2
**/
static inline void SEC() {
    m6502.flags.C = 1;
}

/**
 CLD  Clear Decimal Mode
 
 0 -> D                           N Z C I D V
                                  - - - - 0 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLD           D8    1     2
 **/
static inline void CLD() {
    m6502.flags.D = 0;
}

/**
 SED  Set Decimal Flag
 
 1 -> D                           N Z C I D V
                                  - - - - 1 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SED           F8    1     2
**/
static inline void SED() {
    m6502.flags.D = 1;
}

/**
 CLI  Clear Interrupt Disable Bit
 
 0 -> I                           N Z C I D V
                                  - - - 0 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLI           58    1     2
 **/
static inline void CLI() {
    m6502.flags.I = 0;
}

/**
 SEI  Set Interrupt Disable Status
 
 1 -> I                           N Z C I D V
 - - - 1 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEI           78    1     2
 **/
static inline void SEI() {
    m6502.flags.I = 1;
}

/**
 CLV  Clear Overflow Flag
 
 0 -> V                           N Z C I D V
                                  - - - - - 0
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLV           B8    1     2
**/
static inline void CLV() {
    m6502.flags.V = 0;
}

/**
 CMP  Compare Memory with Accumulator
 
 A - M                            N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     CMP #oper     C9    2     2
 zeropage      CMP oper      C5    2     3
 zeropage,X    CMP oper,X    D5    2     4
 absolute      CMP oper      CD    3     4
 absolute,X    CMP oper,X    DD    3     4*
 absolute,Y    CMP oper,Y    D9    3     4*
 (indirect,X)  CMP (oper,X)  C1    2     6
 (indirect),Y  CMP (oper),Y  D1    2     5*
**/
static inline void CMP( uint8_t imm ) {
    set_flags_NZC( (int)m6502.A - imm );
}

/**
 CPX  Compare Memory and Index X
 
 X - M                            N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     CPX #oper     E0    2     2
 zeropage      CPX oper      E4    2     3
 absolute      CPX oper      EC    3     4
**/
static inline void CPX( uint8_t imm ) {
    set_flags_NZC( (int)m6502.X - imm );
}

/**
 CPY  Compare Memory and Index Y
 
 Y - M                            N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     CPY #oper     C0    2     2
 zeropage      CPY oper      C4    2     3
 absolute      CPY oper      CC    3     4
**/
static inline void CPY( uint8_t imm ) {
    set_flags_NZC( (int)m6502.Y - imm );
}

/**
 JSR  Jump to New Location Saving Return Address
 
 push (PC+2),                     N Z C I D V
 (PC+1) -> PCL                    - - - - - -
 (PC+2) -> PCH
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 absolute      JSR oper      20    3     6
**/
static inline void JSR( uint16_t addr ) {
    PUSH_addr(m6502.pc + 2);
    m6502.pc = addr;
}

/**
 JMP  Jump to New Location
 
 (PC+1) -> PCL                    N Z C I D V
 (PC+2) -> PCH                    - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 absolute      JMP oper      4C    3     3
 indirect      JMP (oper)    6C    3     5
 **/
static inline void JMP( uint16_t addr ) {
    m6502.pc = addr;
}

/**
 RTS  Return from Subroutine
 
 pull PC, PC+1 -> PC              N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       RTS           60    1     6
 **/
static inline void RTS() {
    m6502.pc = POP_addr();
}

/**
 RTI  Return from Interrupt
 
 pull SR, pull PC                 N Z C I D V
                                  from stack
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       RTI           40    1     6
**/
static inline void RTI() {
    m6502.sr = POP();
    RTS();
}

/**
 BIT  Test Bits in Memory with Accumulator
 
 bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
 the zeroflag is set to the result of operand AND accumulator.
 
 A AND M, M7 -> N, M6 -> V        N Z C I D V
                                 M7 + - - - M6
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      BIT oper      24    2     3
 absolute      BIT oper      2C    3     4

**/
static inline void BIT( uint8_t imm ) {
    m6502.flags.N = BITTEST(imm, 7);
    m6502.flags.V = BITTEST(imm, 6);
    m6502.flags.Z = ( m6502.A & imm ) == 0;
}

/**
 ROL  Rotate One Bit Left (Memory or Accumulator)
 
 C <- [76543210] <- C             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ROL A         2A    1     2
 zeropage      ROL oper      26    2     5
 zeropage,X    ROL oper,X    36    2     6
 absolute      ROL oper      2E    3     6
 absolute,X    ROL oper,X    3E    3     7
**/
static inline void ROL( uint8_t * dst ) {
    uint8_t C = m6502.flags.C;
    m6502.flags.C = (*dst & (1<<7)) >> 7;
    *dst <<= 1;
    *dst |= C;
    set_flags_NZ( *dst );
}

/**
 ROR  Rotate One Bit Right (Memory or Accumulator)
 
 C -> [76543210] -> C             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ROR A         6A    1     2
 zeropage      ROR oper      66    2     5
 zeropage,X    ROR oper,X    76    2     6
 absolute      ROR oper      6E    3     6
 absolute,X    ROR oper,X    7E    3     7
**/
static inline void ROR( uint8_t * dst ) {
    uint8_t C = m6502.flags.C << 7;
    m6502.flags.C = *dst;
    *dst >>= 1;
    *dst |= C;
    set_flags_NZ( *dst );
}

/**
 DEC  Decrement Memory by One
 
 M - 1 -> M                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      DEC oper      C6    2     5
 zeropage,X    DEC oper,X    D6    2     6
 absolute      DEC oper      CE    3     3
 absolute,X    DEC oper,X    DE    3     7
**/
static inline void DEC( uint8_t * dst ) {
    (*dst)--;
    set_flags_NZ(*dst);
}

/**
 DEX  Decrement Index X by One
 
 X - 1 -> X                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       DEC           CA    1     2
**/
static inline void DEX() {
    m6502.X--;
    set_flags_NZ(m6502.X);
}

/**
 DEY  Decrement Index Y by One
 
 Y - 1 -> Y                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       DEC           88    1     2
 **/
static inline void DEY() {
    m6502.Y--;
    set_flags_NZ(m6502.Y);
}


/**
 TAX  Transfer Accumulator to Index X
 
 A -> X                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TAX           AA    1     2
**/
static inline void TAX() {
    m6502.X = m6502.A;
    set_flags_NZ(m6502.X);
}

/**
 TAY  Transfer Accumulator to Index Y
 
 A -> Y                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TAY           A8    1     2
**/
static inline void TAY() {
    m6502.Y = m6502.A;
    set_flags_NZ(m6502.Y);
}

/**
 TSX  Transfer Stack Pointer to Index X
 
 SP -> X                          N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TSX           BA    1     2
**/
static inline void TSX() {
    m6502.X = m6502.sp;
    set_flags_NZ(m6502.X);
}

/**
 TXA  Transfer Index X to Accumulator
 
 X -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TXA           8A    1     2
**/
static inline void TXA() {
    m6502.A = m6502.X;
    set_flags_NZ(m6502.A);
}


/**
 TXS  Transfer Index X to Stack Register
 
 X -> SP                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TXS           9A    1     2
 **/
static inline void TXS() {
    m6502.sp = m6502.X;
}


/**
 TYA  Transfer Index Y to Accumulator
 
 Y -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TYA           98    1     2
 **/
static inline void TYA() {
    m6502.A = m6502.Y;
    set_flags_NZ(m6502.A);
}

/////



static inline int m6502_step() {

//    m6502.instr = fetch();
//    switch ( m6502.instr ) {
    switch ( fetch() ) {
//        case 0x00: { uint16_t addr = fetch16(); BIT( (* mmio_read[addr])(addr) ); return 4;   }           // BIT abs BRK(); return 2;                                    // BRK
        case 0x00: BRK(); return 2;                                    // BRK
        case 0x01: ORA( memread( addr_X_ind() ) ); return 6;           // ORA X,ind
//        case 0x02: // t jams
//        case 0x03: // SLO* (undocumented)
//        case 0x04: // NOP* (undocumented)
        case 0x05: ORA( memread( fetch() ) ); return 3;                // ORA zpg
        case 0x06: ASL( & mem[ fetch() ] ); return 5;                  // ASL zpg
//        case 0x07: // SLO* (undocumented)
        case 0x08: PHP(); return 3;                                    // PHP
        case 0x09: ORA( fetch() ); return 2;                           // ORA imm
        case 0x0A: ASL( & m6502.A ); return 2;                         // ASL A
//        case 0x0B: // ANC** (undocumented)
//        case 0x0C: // NOP* (undocumented)
        case 0x0D: ORA( memread( fetch16() ) ); return 4;              // ORA abs
        case 0x0E: ASL( & mem[ fetch16() ] ); return 6;                // ASL abs
//            case 0x0F: // SLO* (undocumented)
        case 0x10: BPL( (int8_t)fetch() ); return 2;                   // BPL rel
        case 0x11: ORA( memread( addr_ind_Y() ) ); return 5;           // ORA ind,Y
//        case 0x12: // t jams
//        case 0x13: // SLO* (undocumented)
//        case 0x14: // NOP* (undocumented)
        case 0x15: ORA( memread( fetch() + m6502.X ) ); return 4;      // ORA zpg,X
        case 0x16: ASL( & mem[ fetch() + m6502.X ] ); return 6;        // ASL zpg,X
//            case 0x17: // SLO* (undocumented)
        case 0x18: CLC(); return 2;                                    // CLC
        case 0x19: ORA( memread( fetch16() + m6502.Y ) ); return 4;    // ORA abs,Y
//        case 0x1A: // NOP* (undocumented)
//        case 0x1B: // SLO* (undocumented)
//        case 0x1C: // NOP* (undocumented)
        case 0x1D: ORA( memread( fetch16() + m6502.X ) ); return 4;    // ORA abs,X
        case 0x1E: ASL( & mem[ fetch16() + m6502.X ] ); return 7;      // ASL abs,X
//        case 0x1F: // SLO* (undocumented)
        case 0x20: JSR( fetch16() ); return 6;                         // JSR abs
        case 0x21: AND( memread( addr_X_ind() ) ); return 6;           // AND X,ind
//        case 0x22:
//        case 0x23:
        case 0x24: BIT( memread( fetch() ) ); return 3;                // BIT zpg
        case 0x25: AND( memread( fetch() ) ); return 3;                // AND zpg
        case 0x26: ROL( & mem[ fetch() ] ); return 5;                  // ROL zpg
//        case 0x27:
        case 0x28: PLP(); return 4;                                    // PLP
        case 0x29: AND( fetch() ); return 2;                           // AND imm
        case 0x2A: ROL( & m6502.A ); return 2;                         // ROL A
//        case 0x2B:
        case 0x2C: BIT( memread( fetch16() ) ); return 4;              // BIT abs
        case 0x2D: AND( fetch16() ); return 4;                         // AND abs
        case 0x2E: ROL( & mem[ fetch16() ] ); return 6;                // ROL abs
//        case 0x2F:
        case 0x30: BMI( (int8_t)fetch() ); return 2;                   // BMI rel
        case 0x31: AND( memread( addr_ind_Y() ) ); return 5;           // AND ind,Y
//        case 0x32:
//        case 0x33:
//        case 0x34:
        case 0x35: AND( memread( addr_zpg_X() ) ); return 4;           // AND zpg,X
        case 0x36: ROL( & mem[ addr_zpg_X() ] ); return 6;             // ROL zpg,X
//        case 0x37:
        case 0x38: SEC(); return 2;                                    // SEC
        case 0x39: AND( memread( addr_abs_Y() ) ); return 4;           // AND abs,Y
//        case 0x3A:
//        case 0x3B:
//        case 0x3C:
        case 0x3D: AND( memread( addr_abs_X() ) ); return 4;           // AND abs,X
        case 0x3E: ROL( & mem[ addr_abs_X() ] ); return 7;             // ROL abs,X
//            case 0x3F:
        case 0x40: RTI(); return 6;                                    // RTI
        case 0x41: EOR( memread( addr_X_ind() ) ); return 6;           // EOR X,ind
//        case 0x42:
//        case 0x43:
//        case 0x44:
        case 0x45: EOR( memread( fetch() ) ); return 3;                // EOR zpg
        case 0x46: LSR( & mem[ fetch() ] ); return 5;                  // LSR zpg
//        case 0x47:
        case 0x48: PHA(); return 3;                                    // PHA
        case 0x49: EOR( fetch() ); return 2;                           // EOR imm
        case 0x4A: LSR( & m6502.A ); return 2;                         // LSR A
//        case 0x4B:
        case 0x4C: JMP( fetch16() ); return 3;                         // JMP abs
        case 0x4D: EOR( fetch16() ); return 4;                         // EOR abs
        case 0x4E: LSR( & mem[ fetch16() ] ); return 6;                // LSR abs
//        case 0x4F:
        case 0x50: BVC( (int8_t)fetch() ); return 2;                   // BVC rel
        case 0x51: EOR( memread( addr_ind_Y() ) ); return 5;           // EOR ind,Y
//        case 0x52:
//        case 0x53:
//        case 0x54:
        case 0x55: EOR( memread( addr_zpg_X() ) ); return 4;           // AND zpg,X
        case 0x56: LSR( & mem[ addr_zpg_X() ] ); return 6;             // LSR zpg,X
//            case 0x57:
        case 0x58: CLI(); return 2;                                    // CLI
        case 0x59: EOR( memread( addr_abs_Y() ) ); return 4;           // EOR abs,Y
//        case 0x5A:
//        case 0x5B:
//        case 0x5C:
        case 0x5D: EOR( memread( addr_abs_X() ) ); return 4;           // EOR abs,X
        case 0x5E: LSR( & mem[ addr_abs_X() ] ); return 7;             // LSR abs,X
//            case 0x5F:
        case 0x60: RTS(); return 6;                                    // RTS
        case 0x61: ADC( memread( addr_X_ind() ) ); return 6;           // ADC X,ind
//        case 0x62:
//        case 0x63:
//        case 0x64:
        case 0x65: ADC( memread( fetch() ) ); return 3;                // ADC zpg
        case 0x66: ROR( & mem[ fetch() ] ); return 5;                  // ROR zpg
//        case 0x67:
        case 0x68: PLA(); break;                                    // PLA
        case 0x69: ADC( fetch() ); return 2;                           // ADC imm
        case 0x6A: ROR( & m6502.A ); return 2;                         // ROR A
//        case 0x6B:
        case 0x6C: JMP( memread( fetch16() ) ); return 5;              // JMP ind
        case 0x6D: ADC( memread( fetch16() ) ); return 4;              // ADC abs
        case 0x6E: ROR( & mem[ fetch16() ] ); return 6;                // ROR abs
//        case 0x6F:
        case 0x70: BVS( (int8_t)fetch() ); break;                   // BVS rel
        case 0x71: ADC( memread( addr_ind_Y() ) ); return 5;           // ADC ind,Y
//        case 0x72:
//        case 0x73:
//        case 0x74:
        case 0x75: ADC( memread( addr_zpg_X() ) ); return 4;           // ADC zpg,X
        case 0x76: ROR( & mem[ addr_zpg_X() ] ); return 6;             // ROR zpg,X
//            case 0x77:
        case 0x78: SEI(); break;                                    // SEI
        case 0x79: ADC( memread( addr_abs_Y() ) ); return 4;           // ADC abs,Y
//        case 0x7A:
//        case 0x7B:
//        case 0x7C:
        case 0x7D: ADC( memread( addr_abs_X() ) ); return 4;           // ADC abs,X
        case 0x7E: ROR( & mem[ addr_abs_X() ] ); return 7;             // ROR abs,X
//        case 0x7F:
//        case 0x80:
        case 0x81: STA( & mem[ addr_X_ind() ] ) ; break;            // STA X,ind
//        case 0x82:
//        case 0x83:
        case 0x84: STY( & mem[ fetch() ] ); break;                  // STY zpg
        case 0x85: STA( & mem[ fetch() ] ); break;                  // STA zpg
        case 0x86: STX( & mem[ fetch() ] ); break;                  // STX zpg
//        case 0x87:
        case 0x88: DEY(); break;                                    // DEY
//        case 0x89:
        case 0x8A: TXA(); break;                                    // TXA
//        case 0x8B:
        case 0x8C: STY( & mem[ fetch16() ] ); break;                // STY abs
        case 0x8D: STA( & mem[ fetch16() ] ); break;                // STA abs
        case 0x8E: STX( & mem[ fetch16() ] ); break;                // STX abs
//        case 0x8F:
        case 0x90: BCC( (int8_t)fetch() ); break;                   // BCC rel
        case 0x91: STA( & mem[ addr_ind_Y() ] ); break;             // STA ind,Y
//        case 0x92:
//        case 0x93:
        case 0x94: STY( & mem[ addr_zpg_X() ] ); break;             // STY zpg,X
        case 0x95: STA( & mem[ addr_zpg_X() ] ); break;             // STA zpg,X
        case 0x96: STX( & mem[ addr_zpg_Y() ] ); break;             // STX zpg,Y
//        case 0x97:
        case 0x98: TYA(); break;                                    // TYA
        case 0x99: STA( & mem[ addr_abs_Y() ] ); break;             // STA abs,Y
        case 0x9A: TXS(); break;                                    // TXS
//        case 0x9B:
//        case 0x9C:
        case 0x9D: STA( & mem[ addr_abs_X() ] ); break;             // STA abs,X
//        case 0x9E:
//        case 0x9F:
        case 0xA0: LDY( fetch() ); break;                           // LDY imm
        case 0xA1: LDA( memread( addr_X_ind() ) ) ; break;          // LDA X,ind
        case 0xA2: LDX( fetch() ); break;                           // LDX imm
//        case 0xA3:
        case 0xA4: LDY( memread( fetch() ) ); break;                // LDY zpg
        case 0xA5: LDA( memread( fetch() ) ); break;                // LDA zpg
        case 0xA6: LDX( memread( fetch() ) ); break;                // LDX zpg
//        case 0xA7:
        case 0xA8: TAY(); break;                                    // TAY
        case 0xA9: LDA( fetch() ); break;                           // LDA imm
        case 0xAA: TAX(); break;                                    // TAX
//        case 0xAB:
        case 0xAC: LDY( memread( fetch16() ) ); break;              // LDY abs
        case 0xAD: LDA( memread( fetch16() ) ); break;              // LDA abs
        case 0xAE: LDX( memread( fetch16() ) ); break;              // LDX abs
//        case 0xAF:
        case 0xB0: BCS( (int8_t)fetch() ); break;                   // BCS rel
        case 0xB1: LDA( memread( addr_ind_Y() ) ); break;           // LDA ind,Y
//        case 0xB2:
//        case 0xB3:
        case 0xB4: LDY( memread( addr_zpg_X() ) ); break;           // LDY zpg,X
        case 0xB5: LDA( memread( addr_zpg_X() ) ); break;           // LDA zpg,X
        case 0xB6: LDX( memread( addr_zpg_Y() ) ); break;           // LDX zpg,Y
//        case 0xB7:
        case 0xB8: CLV(); break;                                    // CLV
        case 0xB9: LDA( memread( addr_abs_Y() ) ); break;           // LDA abs,Y
        case 0xBA: TSX(); break;                                    // TSX
//        case 0xBB:
        case 0xBC: LDY( memread( addr_abs_X() ) ); break;             // LDY abs,X
        case 0xBD: LDA( memread( addr_abs_X() ) ); break;             // LDA abs,X
        case 0xBE: LDX( memread( addr_abs_X() ) ); break;             // LDX abs,X
//        case 0xBF:
        case 0xC0: CPY( fetch() ); break;                           // CPY imm
        case 0xC1: CMP( memread( addr_X_ind() ) ) ; break;          // LDA X,ind
//        case 0xC2:
//        case 0xC3:
        case 0xC4: CPY( memread( fetch() ) ); break;                // CPY zpg
        case 0xC5: CMP( memread( fetch() ) ); break;                // CMP zpg
        case 0xC6: DEC( & mem[ fetch() ] ); break;                  // DEC zpg
//        case 0xC7:
        case 0xC8: INY(); break;                                    // INY
        case 0xC9: CMP( fetch() ); break;                           // CMP imm
        case 0xCA: DEX(); break;                                    // DEX
//        case 0xCB:
        case 0xCC: CPY( memread( fetch16() ) ); break;              // CPY abs
        case 0xCD: CMP( fetch16() ); break;                         // CMP abs
        case 0xCE: DEC( & mem[ fetch16() ] ); break;                // DEC abs
//        case 0xCF:
        case 0xD0: BNE( (int8_t)fetch() ); break;                   // BNE rel
        case 0xD1: CMP( memread( addr_ind_Y() ) ); break;           // CMP ind,Y
//        case 0xD2:
//        case 0xD3:
//        case 0xD4:
        case 0xD5: CMP( memread( addr_zpg_X() ) ); break;           // CMP zpg,X
        case 0xD6: DEC( & mem[ addr_zpg_X() ] ); break;             // DEC zpg,X
//        case 0xD7:
        case 0xD8: CLD(); break;                                    // CLD
        case 0xD9: CMP( memread( addr_abs_Y() ) ); break;           // CMP abs,Y
//        case 0xDA:
//        case 0xDB:
//        case 0xDC:
        case 0xDD: CMP( memread( addr_abs_X() ) ); break;           // CMP abs,X
        case 0xDE: DEC( & mem[ addr_abs_X() ] ); break;             // DEC abs,X
//        case 0xDF:
        case 0xE0: CPX( fetch() ); break;                           // CPX imm
        case 0xE1: SBC( memread( addr_X_ind() ) ) ; break;          // SBC X,ind
//        case 0xE2:
//        case 0xE3:
        case 0xE4: CPX( memread( fetch() ) ); break;                // CPX zpg
        case 0xE5: SBC( memread( fetch() ) ); break;                // SBC zpg
        case 0xE6: INC( & mem[ fetch() ] ); break;                  // INC zpg
//        case 0xE7:
        case 0xE8: INX(); break;                                    // INX
        case 0xE9: SBC( fetch() ); break;                           // SBC imm
        case 0xEA: NOP(); break;                                    // NOP
//        case 0xEB:
        case 0xEC: CPX( memread( fetch16() ) ); break;              // CPX abs
        case 0xED: SBC( fetch16() ); break;                         // SBC abs
        case 0xEE: INC( & mem[ fetch16() ] ); break;                // INC abs
//        case 0xEF:
        case 0xF0: BEQ( (int8_t)fetch() ); break;                   // BEQ rel
        case 0xF1: SBC( memread( addr_ind_Y() ) ); break;           // SBC ind,Y
//        case 0xF2:
//        case 0xF3:
//        case 0xF4:
        case 0xF5: SBC( memread( addr_zpg_X() ) ); break;           // SBC zpg,X
        case 0xF6: INC( & mem[ addr_zpg_X() ] ); break;             // INC zpg,X
//        case 0xF7:
        case 0xF8: SED(); break;                                    // SED
        case 0xF9: SBC( memread( addr_abs_Y() ) ); break;           // SBC abs,Y
//        case 0xFA:
//        case 0xFB:
//        case 0xFC:
        case 0xFD: SBC( memread( addr_abs_X() ) ); break;           // SBC abs,X
        case 0xFE: INC( & mem[ addr_abs_X() ] ); break;             // INC abs,X
//            case 0xFF:

        default:
            printf("Unimplemented Instruction 0x%02X\n", m6502.instr);
            break;
    }
    
    return 2;
}

const unsigned long long int iterations = M;

unsigned long long TICK_PER_SEC = G;
unsigned long long TICK_6502_PER_SEC = 0;
unsigned long long MHz_6502 = 1.024 * M;

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi) );
    return ( (unsigned long long)lo) | ( ((unsigned long long)hi) << 32 );
}


static inline void m6502_run() {
    uint8_t clk = 0;
    // init time
    unsigned long long s = rdtsc();
    unsigned long long e = (unsigned long long)-1LL;

    for ( unsigned long long int i = 0; i < iterations ; i++ ) {
        clk = m6502_step();
        clktime += clk;
        e = TICK_6502_PER_SEC * clktime;
        // query time + wait
//        usleep(1);

        // tight loop gives us the most precise wait time
        while ( rdtsc() - s < e ) {}
    }
}

void init() {
//    for ( int i = 0; i < 64*1024; i++ ) {
//        mmio_read[i] = memread;
//    }
    
    unsigned long long s = rdtsc();
    sleep(1);
    unsigned long long e = rdtsc();
    TICK_PER_SEC = e - s;
    TICK_6502_PER_SEC = TICK_PER_SEC / MHz_6502;
}


int main(int argc, const char * argv[]) {
    // insert code here...
    printf("6502\n");
    
    init();
    
//    clock_t start = clock();
    unsigned long long s = rdtsc();
    m6502_run();
//    clock_t end = clock();
//    double execution_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    unsigned long long e = rdtsc();
    unsigned long long t = e - s;
    double execution_time = (double)t / TICK_PER_SEC;

    double mips = iterations / (execution_time * M);
    double mhz = clktime / (execution_time * M);
    printf("Elpased time: (%llu / %llu / %llu), %.3lfs (%.3lf MIPS, %.3lf MHz)\n", TICK_PER_SEC, MHz_6502, TICK_6502_PER_SEC, execution_time, mips, mhz);

    return 0;
}


//
//  6502.s
//  A2Mac
//
//  Created by Tamas Rudnai on 4/25/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//

// #if defined(__arm__)
//     #define target "arm"

#ifdef __x86_64__

#define ARG_1       rdi
#define ARG_2       rsi
#define ARG_3       rdx
#define ARG_3_32    edx
#define ARG_3_16    dx
#define ARG_3_8     dl
#define ARG_4       rcx
#define ARG_4_32    cdx
#define ARG_4_16    cx
#define ARG_4_8     cl

#define REG_A       al
#define FLG_D       ah
#define REG_A_FLG_D ax

#define REG_X       bl
#define REG_Y       bh
#define REG_XY      bx


#define m6502       r8
#define reg_A       0
#define reg_X       1
#define reg_Y       2

#define flag_C      3
#define flag_Z      4
#define flag_D      6
#define flag_O      9
#define flag_N      10

#define s_clkfrm    30


.file "src/cpu/jit/x64/6502.s"
.intel_syntax noprefix
//.section rodata
.text

.globl _jit_prologue2, _jit_prologue2_addr_m6502, _jit_prologue2_end
_jit_prologue2:
//        push    rax
        push    rbx
//        push    rcx
//        push    rdx
//        push    rdi

        mov m6502, ARG_4                       // m6502 struct

_jit_prologue2_addr_m6502:

        mov     REG_A, byte ptr [m6502 + flag_D]     // m6502.D - Decimal flag
        mov     FLG_D, REG_A
        mov     REG_A, byte ptr [m6502 + reg_A]     // m6502.A
        mov     REG_XY, word ptr [m6502 + reg_X]    // m6502.X & m6502.Y

        xor     cx, cx                          // temp x86 flags

        cmp     byte ptr [m6502 + flag_C], 0         // m6502.C - Carry flag
        jz      .c0
        or      cx, 1 << 0
.c0:

        cmp     byte ptr [m6502 + flag_Z], 0         // m6502.Z - Zero flag
        jz      .z0
        or      cx, 1 << 6
.z0:

        cmp     byte ptr [m6502 + flag_N], 0        // m6502.N - Negative flag
        jz      .n0
        or      cx, 1 << 7
.n0:

        cmp     byte ptr [m6502 + flag_O], 0         // m6502.O - Overflow flag
        jz      .o0
        or      cx, 1 << 11
.o0:

        push    cx                              // commit converted x86 flags
        popfw

        mov     ecx, dword ptr [m6502 + s_clkfrm]     // m6502.clkfrm

        jmp     ARG_3
_jit_prologue2_end:


#else

.globl _jit_prologue2, _jit_prologue2_addr_m6502, _jit_prologue2_end
_jit_prologue2:
_jit_prologue2_addr_m6502:
        ret
_jit_prologue2_end:

#endif



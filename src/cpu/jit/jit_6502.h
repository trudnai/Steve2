//
//  jit_6502.h
//  Steve ][
//
//  Created by Tamas Rudnai on 4/14/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//
// This file is part of Steve ][ -- The Apple ][ Emulator.
//
// Steve ][ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Steve ][ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Steve ][.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef jit_6502_h
#define jit_6502_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "common.h"
#include "6502.h"
#include "woz.h"

void jit_emit_code(void* mem, const void* code, const size_t size);
void run_from_rwx(long i);
unsigned long long test_asm(void);


#endif /* jit_6502_h */

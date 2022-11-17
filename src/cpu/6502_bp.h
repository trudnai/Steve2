//
//  6502_bp.h
//  A2Mac
//
//  Created by Tamas Rudnai on 10/26/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

#ifndef _6502_bp_h
#define _6502_bp_h

#include <stdint.h>

#define DEBUG_MAX_BREAKPOINTS   256
extern uint16_t * breakpoints;
extern uint16_t * mem_read_breakpoints;
extern uint16_t * mem_write_breakpoints;

#define LAST_IDX(bp) (bp)[0]

extern int m6502_dbg_bp_add(uint16_t * bp, uint16_t addr);
extern int m6502_dbg_bp_del(uint16_t * bp, uint16_t addr);
extern void m6502_dbg_bp_del_all(uint16_t * bp);
extern _Bool m6502_dbg_bp_exists(uint16_t * bp, uint16_t addr);


#endif /* _6502_bp_h */

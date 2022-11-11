//
//  6502_debugger.h
//  A2Mac
//
//  Created by Tamas Rudnai on 10/26/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

#ifndef _6502_bp_h
#define _6502_bp_h

#include <stdint.h>

#define DEBUG_MAX_BREAKPOINTS   256
extern uint16_t breakpoints[DEBUG_MAX_BREAKPOINTS];

extern void m6502_dbg_init(void);
extern int m6502_dbg_bp_add(uint16_t addr);
extern void m6502_dbg_bp_del(uint16_t addr);
extern void m6502_dbg_bp_del_all(void);
extern _Bool m6502_dbg_bp_is_exists(uint16_t addr);


#endif /* _6502_bp_h */

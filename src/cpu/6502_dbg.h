//
//  6502_dbg.h
//  A2Mac
//
//  Created by Tamas Rudnai on 10/26/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

#ifndef _6502_dbg_h
#define _6502_dbg_h


extern uint16_t disass_addr;
extern void m6502_Debug(void);
extern void m6502_dbg_on(void);
extern void m6502_dbg_off(void);
extern void m6502_dbg_init(void);


#endif /* _6502_dbg_h */

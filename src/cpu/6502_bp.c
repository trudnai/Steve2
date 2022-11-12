//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019, 2020 Tamas Rudnai. All rights reserved.
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
// Documentations:
//
// http://nesdev.com/6502_cpu.txt
// http://www.oxyron.de/html/opcodes02.html
// https://macgui.com/kb/article/46
// https://www.masswerk.at/6502/6502_instruction_set.html
//

#include <string.h>
#include <stdlib.h>
#include "6502_bp.h"


uint16_t breakpoints[DEBUG_MAX_BREAKPOINTS];
int bp_last_idx = 0;
int bp_idx = 0;


static void swap(uint16_t * a, uint16_t * b) {
    uint16_t temp = *a;
    *a = *b;
    *b = temp;
}


void m6502_dbg_bp_sort( uint16_t arr[], int first, int last ) {
    if ( first < last ) {
        uint16_t pivot = first; // (first + last) / 2;
        int i = first;
        int j = last;

        while ( i < j ) {
            while ( arr[i] <= arr[pivot] && i < last ) {
                i++;
            }
            while ( arr[j] > arr[pivot] && j > first ) {
                j--;
            }

            if ( i < j ) {
                swap(breakpoints + i, breakpoints + j);
            }
        }

        swap(breakpoints + pivot, breakpoints + j);


        if (j > first) {
            m6502_dbg_bp_sort( arr, first, j - 1 );
        }
        if (j < last) {
            m6502_dbg_bp_sort( arr, j + 1, last );
        }
    }
}


/// A binary search function. It returns
/// location of addr in given array arr[l..r] is present,
/// otherwise -1
int m6502_dbg_bp_search(uint16_t arr[], int l, int r, uint16_t addr) {
    while ( r >= l ) {
        int mid = (l + r) / 2;

        // found it
        if (arr[mid] == addr) {
            return mid;
        }

        // maybe in the left side?
        else if (arr[mid] > addr) {
            r = mid - 1;
        }

        // maybe in the right side?
        else {
            l = mid + 1;
        }
    }

    // addr not found
    return -1;
}


/// m6502_dbg_bp_get_empty
/// Get an empty slot in the bp astorage
/// @return Index of the empty breakpoint or -1 if error
int m6502_dbg_bp_get_last(int i) {
    for(; i >= 0; i--) {
        if ( breakpoints[i] ) {
            return i;
        }
    }
    // no empty slots
    return -1;
}


/// m6502_dbg_bp_get_first
/// Get first valid BP
/// @return addr of BP or 0 if non
uint16_t m6502_dbg_bp_get_next() {
    while ( bp_idx < bp_last_idx ) {
        uint16_t addr = breakpoints[++bp_idx];
        if (addr) {
            return addr;
        }
    }
    // no empty slots
    return 0;
}


/// m6502_dbg_bp_get_first
/// Get first valid BP
/// @return addr of BP or 0 if non
uint16_t m6502_dbg_bp_get_first() {
    bp_idx = -1;
    return m6502_dbg_bp_get_next();
}


/// m6502_dbg_bp_get_empty
/// Get an empty slot in the bp storage
/// @return Index of the empty breakpoint or -1 if error
int m6502_dbg_bp_get_empty() {
    for (int i = 0; i < DEBUG_MAX_BREAKPOINTS; i++) {
        if ( breakpoints[i] == 0 ) {
            return i;
        }
    }
    // no empty slots
    return -1;
}


/// m6502_dbg_bp_get_not_empty
/// Get first not empty slot in the bp storage
/// @return Index of the empty breakpoint or -1 if error
int m6502_dbg_bp_get_not_empty() {
    for (int i = 0; i < DEBUG_MAX_BREAKPOINTS; i++) {
        if ( breakpoints[i] ) {
            return i;
        }
    }
    // no empty slots
    return -1;
}


/// m6502_dbg_bp_compact
/// move array down to eliminate
void m6502_dbg_bp_compact() {
    int i = m6502_dbg_bp_get_not_empty();
    memcpy(breakpoints, breakpoints + i, bp_last_idx * sizeof(uint16_t));
    memset(breakpoints + bp_last_idx, 0, (DEBUG_MAX_BREAKPOINTS - bp_last_idx) * sizeof(uint16_t));
    bp_last_idx = m6502_dbg_bp_get_last(bp_last_idx);
}


/// m6502_dbg_bp_get_first
/// Get first valid BP
/// @return addr of BP or 0 if non
_Bool m6502_dbg_bp_is_exists(uint16_t addr) {
    if (addr) {
        int i = m6502_dbg_bp_search(breakpoints, 0, bp_last_idx, addr);
        return i >= 0;
    }

    else return 0;
}


/// m6502_dbg_bp_add
/// Add breakpoint
/// @param addr address to add
/// @return Index of breakpoint or -1 if error
int m6502_dbg_bp_add(uint16_t addr) {
    if (bp_last_idx < DEBUG_MAX_BREAKPOINTS - 1) {
        breakpoints[++bp_last_idx] = addr;
        m6502_dbg_bp_sort(breakpoints, 0, bp_last_idx);
        return bp_last_idx;
    }
    // no empty slots
    return -1;
}


/// m6502_dbg_bp_del
/// Remove a breakpoint
/// @param addr address to remove
void m6502_dbg_bp_del(uint16_t addr) {
    int i = m6502_dbg_bp_search(breakpoints, 0, bp_last_idx, addr);
    if (i >= 0) {
        breakpoints[i] = 0;
        m6502_dbg_bp_sort(breakpoints, 0, bp_last_idx);
        m6502_dbg_bp_compact();
    }
}


void m6502_dbg_bp_del_all(void) {
    bp_idx = 0;
    memset(breakpoints, 0, sizeof(breakpoints));
}


void m6502_dbg_init(void) {
    m6502_dbg_bp_del_all();
}



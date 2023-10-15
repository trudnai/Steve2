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
#include "6502.h"
#include "6502_bp.h"

/// Array of addresses of active breakpoints
/// @note DEBUG_MAX_BREAKPOINTS controls its size
/// last index (size) is located at index 0
uint16_t bp_array[DEBUG_MAX_BREAKPOINTS];
/// Pointer to the bp_array
/// @note We have to do this way because of interfacing problem with Swift
uint16_t * breakpoints = bp_array;
/// Array of addresses of memory read breakpoints
/// @note DEBUG_MAX_BREAKPOINTS controls its size
/// last index (size) is located at index 0
uint16_t mem_rd_bp[DEBUG_MAX_BREAKPOINTS];
uint16_t * mem_read_breakpoints = mem_rd_bp;
/// Array of addresses of memory write breakpoints
/// @note DEBUG_MAX_BREAKPOINTS controls its size
/// last index (size) is located at index 0
uint16_t mem_wr_bp[DEBUG_MAX_BREAKPOINTS];
uint16_t * mem_write_breakpoints = mem_wr_bp;

/// Index of current breapoint
/// @note It is more like a temporary variable
int bp_next_idx = 0;


/// Swap 2 items
/// @param a Pointer of first item
/// @param b Pointer to second item
static inline void swap(uint16_t * a, uint16_t * b) {
    uint16_t temp = *a;
    *a = *b;
    *b = temp;
}


/// m6502_dbg_bp_sort
/// Sort breakpoint array ascending
/// @param arr Breakpoint array
/// @param first Index of first element to sort
/// @param last Index of last element to sort
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
/// @param arr Breakpoint array
/// @param l Left index
/// @param r Right index
/// @param addr Address to search for
/// @return lIndex of addr in Breakpoints, otherwise -1
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


/// Get index of the last BP
/// @return Index of the last breakpoint or 0 if non
int m6502_dbg_bp_get_last(uint16_t *bp) {
    for(int i = LAST_IDX(bp); i >= 0; i--) {
        if ( bp[i] ) {
            return i;
        }
    }
    // index should be 0 if there is no any BPs
    return 0;
}


/// Get first valid BP
/// @return addr of BP or 0 if non
uint16_t m6502_dbg_bp_get_next(uint16_t * bp) {
    while ( bp_next_idx++ <= LAST_IDX(bp) ) {
        uint16_t addr = bp[bp_next_idx];
        if (addr) {
            return addr;
        }
    }
    // no empty slots
    return 0;
}


/// Get first valid BP
/// @return addr of BP or 0 if non
uint16_t m6502_dbg_bp_get_first(uint16_t * bp) {
    bp_next_idx = 0;
    return m6502_dbg_bp_get_next(bp);
}


/// m6502_dbg_bp_get_empty
/// Get an empty slot in the bp storage
/// @return Index of the empty breakpoint or -1 if error
int m6502_dbg_bp_get_empty(void) {
    for (int i = 0; i < DEBUG_MAX_BREAKPOINTS; i++) {
        if ( breakpoints[i] == 0 ) {
            return i;
        }
    }
    // no empty slots
    return -1;
}


/// Get first not empty slot in the bp storage
/// @return Index of the empty breakpoint or -1 if error
int m6502_dbg_bp_get_not_empty(uint16_t * bp) {
    for (int i = 1; i < DEBUG_MAX_BREAKPOINTS; i++) {
        if ( bp[i] ) {
            return i;
        }
    }
    // no empty slots
    return -1;
}


/// Move array down to eliminate leading zeros
/// @note: Array must be sorted before this!
/// @return last index
int m6502_dbg_bp_compact(uint16_t * bp) {
    int i = m6502_dbg_bp_get_not_empty(bp);
    if ( i > 1 ) {
        memcpy(bp + 1, bp + i, LAST_IDX(bp) * sizeof(uint16_t));
    }
    LAST_IDX(bp) = i = m6502_dbg_bp_get_last(bp);
    memset(bp + i + 1, 0, (DEBUG_MAX_BREAKPOINTS - i - 2) * sizeof(uint16_t));
    return i;
}


/// Check if BP exists
/// @param addr Address to check
/// @return 1 (true) if exists, 0 (false) if not
_Bool m6502_dbg_bp_exists(uint16_t * bp, uint16_t addr) {
    if (addr) {
        int i = m6502_dbg_bp_search(bp, 1, LAST_IDX(bp), addr);
        return i >= 0;
    }

    else return 0;
}


/// Add breakpoint
/// @param addr Address to add
/// @return Index of breakpoint or 0 if error
int m6502_dbg_bp_add(uint16_t * bp, uint16_t addr) {
    if (LAST_IDX(bp) < DEBUG_MAX_BREAKPOINTS - 1) {
        bp[++LAST_IDX(bp)] = addr;
        m6502_dbg_bp_sort(bp, 1, LAST_IDX(bp));
        m6502_dbg_bp_compact(bp);
        return LAST_IDX(bp);
    }
    // no empty slots
    return 0;
}


/// Remove a breakpoint
/// @param addr address to remove
int m6502_dbg_bp_del(uint16_t * bp, uint16_t addr) {
    int i = m6502_dbg_bp_search(bp, 1, LAST_IDX(bp), addr);
    if (i >= 0) {
        bp[i] = 0;
        m6502_dbg_bp_sort(breakpoints, 1, LAST_IDX(bp));
        m6502_dbg_bp_compact(bp);
    }
    return LAST_IDX(bp);
}


/// Delete all breakpoints
void m6502_dbg_bp_del_all(uint16_t * bp) {
    bp_next_idx = 0;
    memset(bp, 0, sizeof(uint16_t) * DEBUG_MAX_BREAKPOINTS);
}



//
//  A2Mac-Bridging-Header.h
//  Steve ][
//
//  Created by Tamas Rudnai on 2/17/20.
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
//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#ifndef A2Mac_Bridging_Header_h
#define A2Mac_Bridging_Header_h


#import "6502.h"
#import "6502_bp.h"
#import "6502_dbg.h"
#import "6502_dis.h"
#import "6502_dis_utils.h"
#import "speaker.h" // So we can access to speaker stuff from Swift
#import "disk.h"
#import "dsk2woz.h"
#import "mmio.h"
#import "hires.h"
#import "woz.h"


#endif /* A2Mac_Bridging_Header_h */

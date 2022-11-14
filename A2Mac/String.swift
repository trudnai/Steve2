//
//  String.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 11/13/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

import Foundation

extension String {
    func hexValue() -> UInt {
        return UInt(lowercased(), radix: 16) ?? 0
    }
}


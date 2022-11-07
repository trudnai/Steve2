//
//  NSLayoutConstraint.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 11/5/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

import Cocoa


extension NSLayoutConstraint {

    override public var description: String {
        let id = identifier ?? super.description
        return "constaint id: \(id), constant: \(constant)" //you may print whatever you want here
    }
}


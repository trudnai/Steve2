//
//  ViewController.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 7/25/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

import Cocoa

class ViewController: NSViewController {

    @IBOutlet weak var display: NSTextFieldCell!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        display.stringValue = "lll"

        // emulate in he background
//        DispatchQueue.global(qos: .background).async {
            tst6502()
//        }

        // let r = UnsafeMutablePointer<UInt8>(RAM)
        
//        let text = ""
//        while(true) {
//            for i in 0x400...0x7FF {
//        //        text += r[i]
//            }
//        }
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }

}


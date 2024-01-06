//
//  HexDigitFormatter.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 11/13/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

import Foundation

class HexDigitFormatter: Formatter {

    let maxLength : Int
    let wrongCharacterSet = CharacterSet(charactersIn: "0123456789ABCDEFabcdef").inverted

    init(maxLen : Int) {
        maxLength = maxLen
        super.init()
    }

    required init?(coder: NSCoder) {
        maxLength = 2
        super.init(coder: coder)
    }

    override func isPartialStringValid(_ partialString: String, newEditingString newString: AutoreleasingUnsafeMutablePointer<NSString?>?, errorDescription error: AutoreleasingUnsafeMutablePointer<NSString?>?) -> Bool {
        if partialString.count > maxLength {
            return false
        }

        if partialString.rangeOfCharacter(from: wrongCharacterSet) != nil {
            return false
        }

        return true
    }


    override func string(for obj: Any?) -> String? {
        guard let string = obj as? String,
              string.count <= maxLength
        else {
            return nil
        }

        return string.uppercased()
    }


    override func getObjectValue(
        _ obj: AutoreleasingUnsafeMutablePointer<AnyObject?>?,
        for string: String,
        errorDescription error: AutoreleasingUnsafeMutablePointer<NSString?>?
    ) -> Bool {

        let hexValue: String
        if string.starts(with: "#") {
            hexValue = String(string.dropFirst()).uppercased()
        }
        else if string.starts(with: "%") {
            hexValue = String(string.dropFirst()).uppercased()
        }
        else if string.starts(with: "$") {
            hexValue = String(string.dropFirst()).uppercased()
        }
        else if string.starts(with: "0x") {
            hexValue = String(string.dropFirst(2)).uppercased()
        }
        else {
            hexValue = string.uppercased()
        }

        obj?.pointee = hexValue as AnyObject
        return true
    }

//    override func attributedString(for obj: AnyObject, withDefaultAttributes attrs: [String : AnyObject]? = [:]) -> AttributedString? {
//        let stringVal = string(for: obj)
//
//        guard let string = stringVal else { return nil }
//
//        return AttributedString(string: string, attributes: attributes)
//    }

//    var attributes: [String: AnyObject] {
//        let style = NSMutableParagraphStyle()
//        style.minimumLineHeight = 100
//        style.maximumLineHeight = 100
//        style.paragraphSpacingBefore = 0
//        style.paragraphSpacing = 0
//        style.alignment = .center
//        style.lineHeightMultiple = 1.0
//        style.lineBreakMode = .byTruncatingTail
//        let droidSansMono = NSFont(name: "DroidSansMono", size: 70)!
//        return [NSParagraphStyleAttributeName: style, NSFontAttributeName: droidSansMono, NSBaselineOffsetAttributeName: -60]
//    }
}

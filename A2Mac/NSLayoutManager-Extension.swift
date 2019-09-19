//
//  NSLayoutManager-Extension.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 9/17/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

import Foundation
import Cocoa
import AppKit

class LayoutManager : NSLayoutManager {
    
    var text: String? { return textStorage?.string }
    
    var font: NSFont = NSFont.systemFont(ofSize: NSFont.systemFontSize) {
        didSet {
            guard let text = self.text else { return }
            let textRange = NSMakeRange(0, (text as NSString).length)
            invalidateGlyphs(forCharacterRange: textRange, changeInLength: 0, actualCharacterRange: nil)
            invalidateLayout(forCharacterRange: textRange, actualCharacterRange: nil)
        }
    }
    
    override func drawBackground(forGlyphRange glyphsToShow: NSRange, at origin: CGPoint) {
        
        super.drawBackground(forGlyphRange: glyphsToShow, at:origin)
        
        guard let text = self.text else { return }
        
        enumerateLineFragments(forGlyphRange: glyphsToShow)
        { (rect: CGRect, usedRect: CGRect, textContainer: NSTextContainer, glyphRange: NSRange, stop: UnsafeMutablePointer<ObjCBool>) -> Void in
            
            let characterRange = self.characterRange(forGlyphRange: glyphRange, actualGlyphRange: nil)
            
            // Draw invisible tab space characters
            
            let line = (self.text! as NSString).substring(with: characterRange)
            
            do {
                
                let expr = try NSRegularExpression(pattern: "\t", options: [])
                
                expr.enumerateMatches(in: line, options: [.reportProgress], range: NSRange(location: 0, length: line.count))
                { (result: NSTextCheckingResult?, flags: NSRegularExpression.MatchingFlags, stop: UnsafeMutablePointer<ObjCBool>) in
                    
                    if let result = result {
                        
                        let range = NSMakeRange(result.range.location + characterRange.location, result.range.length)
                        let characterRect = self.boundingRect(forGlyphRange: range, in: textContainer)
                        
                        let symbol = "\u{21E5}"
                        let attrs = [NSAttributedString.Key.font : self.font]
                        let height = (symbol as NSString).size(withAttributes: attrs).height
                        symbol.draw(in: characterRect.offsetBy(dx: 1.0, dy: height * 0.5), withAttributes: attrs)
                        
                    }
                    
                }
                
            } catch let error as NSError {
                print(error.localizedDescription)
            }
            
        }
        
    }
    
}

//
//  RepeatingTimer.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 9/15/19.
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

import Foundation

/// RepeatingTimer mimics the API of DispatchSourceTimer but in a way that prevents
/// crashes that occur from calling resume multiple times on a timer that is
/// already resumed (noted by https://github.com/SiftScience/sift-ios/issues/52
class RepeatingTimer {
    
    let timeInterval: TimeInterval
    var eventHandler: (() -> Void)?
    
    private enum State {
        case suspended
        case resumed
    }
    
    private var state: State = .suspended

    init(timeInterval: TimeInterval) {
        self.timeInterval = timeInterval
    }
    
//    var dt_start = DispatchTime.now()

    private lazy var timer: DispatchSourceTimer = {
        let t = DispatchSource.makeTimerSource()
//        let t = DispatchSource.makeTimerSource(flags: .strict, queue: .global(qos: .userInitiated))
//        dt_start = DispatchTime.now()
        t.schedule(deadline: .now() + self.timeInterval, repeating: self.timeInterval)
        t.setEventHandler(handler: { [weak self] in
//            let dt_end = DispatchTime.now()
//
//            let nanoTime = dt_end.uptimeNanoseconds - (self?.dt_start.uptimeNanoseconds)! // <<<<< Difference in nano seconds (UInt64)
//            let timeInterval = Double(nanoTime) / 1_000_000_000 // Technically could overflow for long running tests
//
//            if ( timeInterval < 0.0001 ) {
//                print("timer: \(timeInterval) seconds")
//            }
//
//            self?.dt_start = dt_end

            guard let strongSelf = self else {
                print("RepeatingTimer Error: No self!\n");
                return
            }
            strongSelf.eventHandler?()
        })
        return t
    }()
    
    
    deinit {
        timer.setEventHandler {}
        timer.cancel()
        /*
         If the timer is suspended, calling cancel without resuming
         triggers a crash. This is documented here https://forums.developer.apple.com/thread/15902
         */
        resume()
        eventHandler = nil
    }
    
    func resume() {
        if state == .resumed {
            return
        }
        state = .resumed
        timer.resume()
    }
    
    func suspend() {
        if state == .suspended {
            return
        }
        state = .suspended
        timer.suspend()
    }
    
    func kill() {
        timer.cancel()
    }
}

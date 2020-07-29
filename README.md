# Steve2

**Steve \]\[ - Speedy Apple \]\[ Emulator**

_Pronounciation:_ `Steve Two`

Steve \]\[ is an [Apple \]\[](https://en.wikipedia.org/wiki/Apple_II) emulator initially developped for Mac under GPL v3 licence. 

(See licensing and copyright info in COPYING)

The name refers to "two Steves", _Steve Wozniak_ and _Steve Jobs_ who created Apple \]\[ and founded Apple.

Steve \]\[ was introduce at [KansasFest 2020](https://www.kansasfest.org) (Journey of an Apple ][ Emulator) and release on GitHub as an Open Source project.

Original goal of the project was to find out how fast Apple \]\[ would be in today's technologies. The original Apple \]\[ was introduced in 1977, including Integer BASIC and Assembler Monitor functions. Later on the BASIC interpreter was replaced by Microsoft's AppleSoft BASIC, and the hardware design went through on several iterations. Most models were using a variant of [6502](https://en.wikipedia.org/wiki/MOS_Technology_6502) CPU at a clock speeed of 1.023 MHz. Looking at it 40 years later it seems like impossible to do anything on those computers, however, at that time Apple \]\[ was the state of the art technology. People used it for text processing, the very first spreadsheet app was written on it, and of course games, many many games. Some notable titles include Karateka, Prince of Persia and Castle of Wolfeinstein. This latter one evolved to Doom on the PC platform.

At 1.023 MHz operation speed was limited. Just an example of it, if you wrote a BASIC program to count from 1 to 65536 printing out the number at each iterations, it takes about 31 minutes. The very same emulating Apple ][ at 1.3 GHz takes only 2 seconds.

**Main Features of Steve \]\[**

- Very fast 6502 CPU emulation
- Fast Disk \]\[ emulation
- Modern User Interface with Light and Dark mode
- Game Mode with low Input Latency and 60 FPS
- Eco Mode for fast operation and low energy consumption
- Natively supports WOZ 1 disk image format
- DSK / DO / PO disk image support with smoothless internal autoconvert

**Further Features**

- CRT Mode for realistic retro feeling
- Color Monitor
- Green Mono Monitor
- Amber Mono Monitor
- White Mono Monitor
- Select + Copy + Paste from/to Text screen


**How to clone (download from GitHub):**

Open Terminal app and enter the following code:

```git clone http://github.com/trudnai/Steve2```

**How to run:**

Open ```A2Mac``` project, make sure ```A2Mac > My Mac``` target is selected at the top left corner and hit the ```Play``` button or press ```⌘R```


**In Memoriam**

I would like to dedicate this project to my Father. He was a mechanical engineer, plumming engineer designing heating and water supply system for large buildings. He bought our first Apple \]\[ computer to help on his projects. He was a dedicated man and even though he was not a software developer, he wrote several programs, most notable a budget caluculator. Budget calculation for plumbing system meant he had to count all components such as different lenght and size of pipes and fittings and count every single bits to the last screw. Then he had to look all the components up one by one for prices and esimated work hours to be able to estimate the financial needs to realize the project. It was a tedious job, and he was able to reduce that work from 3 weeks to 3 days with the computer. This is obvious today, but at that time it was novel. His program on the Apple \]\[ was working on the calculation for 3-4 hours each time. Back then that was considered as small amount of time compared to what it takes to do that manually.

When my Father passed away, I have found our old Apple \]\[ in the basement, and many memories rushed into my head from that time. I remembered the times he eagerly waited for the Apple \]\[ to finish a calculation. He had a nice 1.5 hours lunch and coffee and conversation with the family, and the program was still running.

I started to wonder how long that calculation would take on a modern computer -- or more precisely if the Apple \]\[ was built by today's technology. That was the initial goal of this project.

Steve \]\[ on my 2015 15" MacBook Pro was able to emulate a 1.3 GHz Apple \]\[. That is 1300x faster than the original one. So if that claculation is lasted for 4 hours back then:

```
]O = 4 * 60 * 60

]? O
14400

]? O / 1300
11.0769231
```

...then it would have lasted for 11 seconds if Apple \]\[ was built today (calculating with the measued 1300x speed increase)

_Disclaimer: This is just an estimation, on a real hardware most probably it would even faster several times. That is because even if Steve \]\[ very is fast, it is still an emulator -- and an emulator needs to do a lot of things on the host CPU to emulate just a single opcode._




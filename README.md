# Steve2

**Steve \]\[ - Speedy Apple \]\[ Emulator**

_Pronounciation:_ `Steve Two`

Steve \]\[ is an [Apple \]\[](https://en.wikipedia.org/wiki/Apple_II) emulator initially developped for Mac under GPL v3 licence. 
(See licensing and copyright info in COPYING)

The name refers to "two Steves", _Steve Wozniak_ and _Steve Jobs_ who created Apple \]\[ and founded Apple.

Steve \]\[ was introduce at [KansasFest 2020](https://www.kansasfest.org) (Journey of an Apple ][ Emulator) and release in GitHub as an Open Source project.

Original goal of the project was to find out how fast Apple ][ would be in today's technologies. The original Apple ][ was introduced in 1977, including Integer BASIC and Assembler Monitor functions. Later on the BASIC interpreter was replaced by Microsoft's AppleSoft BASIC, and the hardware design went through on several iterations. Most models were using a variant of [6502](https://en.wikipedia.org/wiki/MOS_Technology_6502) CPU at a clock speeed of 1.023 MHz. Looking at it 40 years later it seems like impossible to do anything on those computers, however, at that time Apple ][ was the state of the art technology. People used it for text processing, the very first spreadsheet app was written on it, and of course games, many many games. Some notable titles include Karateka, Prince of Persia and Castle of Wolfeinstein. This latter one evolved to Doom on the PC platform.

At 1.023 MHz operation speed was limited. Just an example of it, if you wrote a BASIC program to count from 1 to 65536 printing out the number at each iterations, it takes about 31 minutes. The very same emulating Apple ][ at 1.3 GHz takes only 2 seconds.

**Main Features of Steve ][**

- Very fast 6502 CPU emulation
- Fast Disk ][ emulation
- Modern User Interface with Light and Dark mode
- Game Mode with low Input Latency and 60 FPS
- Eco Mode for fast operation and low energy consumption
- Native WOZ 1 disk image format support
- DSK / DO / PO disk Image support with internal autoconvert

**Further Features**

- CRT Mode for realistic retro feeling
- Color Monitor
- Green Mono Monitor
- Amber Mono Monitor
- White Mono Monitor
- Select + Copy + Paste from/to Text screen


How to clone:

Open Terminal app and enter the following code:

```git clone http://github.com/trudnai/Steve2```

After that open _A2Mac_ project, make sure _A2Mac > My Mac_ target is selected at the top left corner and hit the Play button or press ⌘+R


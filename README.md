# The Amethyst Colour Video System

https://www.youtube.com/watch?v=6GKgxBEGH1M

The Amethyst is a retro-styled "home computer" built around an Atmel ATmega1284 microcontroller.
It features composite (NTSC) video output with both high-color and high-resolution modes, mono
audio output, and a full-travel mechanical keyboard. It includes a full-featured implementation
of the powerful Forth programming language, with graphics and sound commands, debugger, and
screen editor. A single USB Type-B cable provides 5V power as well as serial communication with
a PC. Amethyst is compatible with the Optiboot (Arduino) bootloader, and new firmware may be
uploaded to it without a specialized programming device.


## Full specifications

- ATmega1284 microcontroller (8-bit) running at 14.318 MHz
- 16KB internal memory (16352 bytes available for applications)
- 4KB nonvolatile EEPROM memory
- Mechanical keyboard with Cherry MX keyswitches
- NTSC color and monochrome composite video output:

    - 256-color, 16-color, and 4-color bitmap graphics at resolutions up to 160x200 pixels
    - Monochrome bitmap graphics at resolutions up to 640x200 pixels
    - 40x25 and 80x25 monochrome text modes
    - 40x25 color text mode
    - 40x25 color tiled graphics modes
    
- Single-channel audio output (pulse wave or PWM)
- USB serial communication at speeds up to 57600 baud
- Four SPI expansion ports for peripherals, controllers, and storage devices
- Only 6 chips (ATmega1284, FT320X, 2x 74HC157, 2x 74HC166)

**TODO 1/5/20: add more detailed docs, instructions, and BOM**


## About

Amethyst is a hobby project and is not currently available for purchase in kit or built form.

Copyright 2019 Matt Sarnoff.

https://twitter.com/txsector

http://msarnoff.org

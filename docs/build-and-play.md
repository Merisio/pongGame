# Build, flash, and play

[Back to repository](../README.md) · [Hardware](hardware.md) · [Firmware architecture](firmware.md)

## Build requirements

- An **MSP430G2553** target and an appropriate programmer/debugger.
- An MSP430 compiler providing `msp430.h`, the device's register definitions, interrupt support, and delay intrinsics.
- An IDE/toolchain compatible with the source's `#pragma vector`, `__interrupt`, `__delay_cycles`, and binary integer literals.

[TI Code Composer Studio](https://www.ti.com/tool/CCSTUDIO), configured with MSP430 device support and a compatible TI MSP430 compiler, is a suitable workflow for this style of source. Ordinary desktop C compilation cannot access the target registers or provide its interrupt runtime. Porting to another compiler may require changes to ISR declarations and intrinsics.

## Create the firmware project

The repository contains one source file; it does not include an existing IDE project, linker configuration, or a Makefile.

1. Create an empty C firmware project for **MSP430G2553** in your compatible MSP430 IDE.
2. Select the debugger/programmer attached to your board and let the IDE provide the device-specific startup code and linker configuration.
3. Add or link [src/pongGame.c](../src/pongGame.c) to the project. If the project template created another `main.c`, exclude it so there is only one `main()` definition.
4. Build the project and review the compiler diagnostics and memory report. Device headers should come from the toolchain, not a replacement desktop header.
5. Assemble the [hardware connections](hardware.md), flash the target, and start execution. Check the clock assumptions in that guide if startup stops in `ini_uCon()`.

Build outputs can stay in `Debug/`, `Release/`, or `build/`; these directories are ignored by Git. No successful build or hardware run is claimed for this documentation update.

## Player controls

| Stage | Action | Firmware behavior |
| --- | --- | --- |
| Title animation | Press P1.3 button | Enters mode selection after the animation loop checks the press |
| Mode selection | Wait for the desired symbol, then press | The display automatically alternates between the two modes; a press selects the current one |
| Match preparation | Press to begin | Clears the initial-start flag and starts the game timer again |
| Play | Rotate each potentiometer | Moves the corresponding paddle |
| Play | Press button | Pauses the match |
| Paused match | Press button | Resumes timer operation and reinitializes the ball position/direction |
| End of match | Either player reaches three points | Shows both scores, clears them, and returns to the title animation |

The display modes are selected by whether `dificuldade == 1` or its alternate value. The first draws two-pixel paddles and requests a shorter timer interval; the alternate draws three-pixel paddles with a longer interval. See the [timing table](firmware.md#timing-and-modes).

Button response is not always immediate: title/menu processing includes blocking delays. Resume restarts the ball rather than preserving its previous trajectory; that is the behavior of the unchanged source.

## Troubleshooting

| Symptom | What to inspect |
| --- | --- |
| Build reports an unknown device register or interrupt syntax | MCU selection, device headers, and compiler compatibility |
| Program does not reach the title | 16 MHz calibration constants and the `LFXT1OF` wait in `ini_uCon()` |
| Matrices stay blank | Power, ground, interface levels, DIN/CLK/LOAD connections, and daisy-chain direction |
| Display halves or paddle directions appear reversed | Physical matrix orientation/chain order and potentiometer outer-terminal connections |
| Paddle does not move | Wiper connection to A1/A2 and ADC input voltage range |
| Button does not respond | P1.3-to-ground connection and existing menu delays |

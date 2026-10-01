# Hardware and connections

[Back to repository](../README.md) · [Build and play](build-and-play.md)

The pin mapping below is inferred from [the firmware](../src/pongGame.c). The original repository did not include a circuit schematic or a board layout. Use GPIO names rather than physical package-pin numbers when assembling the circuit.

## Parts

| Quantity | Component | Purpose |
| --- | --- | --- |
| 1 | MSP430G2553 board with programming/debug access | Runs the firmware |
| 2 | MAX7219-based 8×8 LED matrix modules | Display the playing field and menus |
| 2 | 10 kΩ potentiometers | Control the two paddles |
| 1 | Normally open push button, or a board button on P1.3 | Menu selection and pause/resume |
| As needed | Breadboard and jumper wires | Interconnect the components |
| As needed | Regulated supplies and appropriate logic-level translation | Power the MCU/displays and match the serial interface levels |

## Firmware pin mapping

| MSP430 signal | Connection | Role |
| --- | --- | --- |
| P1.7 / UCB0SIMO | DIN of the first MAX7219, through level translation as required | Serial display data |
| P1.5 / UCB0CLK | CLK of both MAX7219s, through level translation as required | Shared serial clock |
| P1.4 | LOAD/CS of both MAX7219s, through level translation as required | Shared latch signal |
| P1.1 / A1 | Wiper of the left-player potentiometer | Left paddle input |
| P1.2 / A2 | Wiper of the right-player potentiometer | Right paddle input |
| P1.3 | Button to ground | Active-low button with an internal pull-up |
| Ground | Both display modules, controls, supplies, and MCU | Common reference |

Connect each potentiometer's outer terminals to the MCU supply and ground, with its wiper connected to the corresponding ADC input. Reversing the outer terminals reverses the paddle-control direction. The MCU's GPIO alternate functions are documented in the [TI MSP430G2553 datasheet](https://www.ti.com/lit/ds/symlink/msp430g2553.pdf).

## Display chain

```text
MSP430 P1.7 -- level translation --> DIN [MAX7219 #1]
                                         DOUT --------> DIN [MAX7219 #2]
MSP430 P1.5 -- level translation --> CLK [both modules]
MSP430 P1.4 -- level translation --> LOAD [both modules]
```

The first chip is nearest the MCU's data output. `spi_max()` shifts the second chip's address/data pair first, then the first chip's pair, before raising LOAD. No display data is read back into the MCU; leave the final DOUT unconnected.

Matrix orientation depends on the module's LED wiring. Arrange/rotate the modules so the two buffers form a continuous field with paddles at opposite edges. If the halves appear reversed, compare their chain order with the two argument pairs passed to `spi_max()`.

## Supply and logic levels

Use a suitable MCU supply, such as a regulated 3.3 V board rail, and keep potentiometer outputs within that supply range. The MSP430G2553 supply range is 1.8–3.6 V; the allowable clock frequency also depends on the supply voltage. See the [TI electrical specifications](https://www.ti.com/lit/ds/symlink/msp430g2553.pdf).

The MAX7219 is specified for a 4.0–5.5 V supply and a minimum logic-high input of 3.5 V. A direct 3.3 V GPIO connection therefore does not guarantee a valid high level. Use level translation appropriate for DIN, CLK, and LOAD, unless the chosen module already provides a compatible interface. Keep the display supply separate from the MCU/ADC voltage domain and share ground. These limits come from the [Analog Devices MAX7219/MAX7221 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX7219-MAX7221.pdf).

This guide assumes complete matrix modules with their LED-current-setting resistor and supply decoupling. If using bare driver ICs, follow the manufacturer's application circuit for those components.

## Clock and board assumptions

`ini_uCon()` loads the factory 16 MHz DCO calibration values and waits while `LFXT1OF` is set. On a board without a working low-frequency crystal, execution can remain in that wait loop. Check the board's oscillator configuration and calibration memory before relying on the startup procedure. The pin table describes the firmware's interface, not a verified reconstruction of the original circuit.

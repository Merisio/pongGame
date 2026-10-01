# Firmware architecture

[Back to repository](../README.md) · [Source](../src/pongGame.c)

The game is implemented in a single C translation unit with Portuguese function/variable names. It accesses MSP430 peripheral registers directly and uses global state shared by foreground code and interrupt handlers.

## Initialization and control flow

`main()` configures the GPIOs, clocks, debounce timer, serial interface, and ADC, then calls `tela_inicial()`. That function contains the foreground state loop and normally never returns; the empty loop at the end of `main()` is not the gameplay loop.

| `jogo_ativo` value | State | Main work |
| --- | --- | --- |
| `0` | Title | Sends 19 paired animation frames to the display chain and checks for a button press |
| `1` | Mode selection | Alternates mode bitmaps until a press selects one |
| `2` | Match | Initializes the ball/game timer and handles button events while timer interrupts update the game |

`jogo_comecando` distinguishes the initial start press from pause/resume. `jogo_pausado` gates gameplay updates. Button handling uses a Port 1 interrupt; Timer_A0 temporarily suppresses subsequent button interrupts for debounce.

## Function map

| Function or handler | Responsibility |
| --- | --- |
| `ini_P1_P2()` | Configures P1.3 button/pull-up, P1.4 latch, and serial pin multiplexing |
| `ini_uCon()` | Disables watchdog, loads DCO calibration, configures clock division, and enables interrupts |
| `spi_init()` / `spi_max()` | Configure USCI_B0 and send chained MAX7219 register writes |
| `tela_inicial()` / `mostra_modo()` | Initialize displays, render title/menu, and manage foreground states |
| `RTI_PORTA_1()` / `RTI_Mod_0_Timer_0()` | Capture/debounce presses and toggle pause/start flags |
| `RTI_Timer_Game()` | Starts ADC acquisition and calls `loop_pong()` |
| `RTI_ADC10()` | Averages eight readings and alternates the selected channel between A1/A2 |
| `desenha_raquete_*()` / `desenha_bola()` | Set pixels in the two display buffers |
| `atualiza_bola()` | Move the ball, test collisions, and award points |
| `loop_pong()` / `mostra_ponto()` | Clear/redraw the field or show the final scores |

## Display model

`tela_esq[8]` and `tela_dir[8]` hold eight bytes each. Together they represent a 16×8 logical field. Despite their names, `bola_x` is the bit position along the eight-pixel dimension, and `bola_y` selects an address along the sixteen-pixel dimension: values below eight use the first buffer, and values eight or higher use the second.

The left paddle is drawn in `tela_esq[0]`; the right paddle is drawn in `tela_dir[7]`. Each update clears the buffers, draws both paddles and the ball, then writes eight paired digit registers. Display decoding is disabled, scan limit is set to eight registers, and intensity is set to `0x08`.

`IMAGES` contains 38 eight-byte patterns sent as 19 paired title frames. `modos` stores the paired mode symbols, and `pontos` stores the end-of-match score patterns.

## Timing and modes

Nominal timing below is calculated from the register settings, not measured on hardware. The calibrated DCO/MCLK is 16 MHz; `DIVS1` selects SMCLK ÷4, giving 4 MHz, and both timer configurations apply a further ÷8. Timer register/divider behavior is described in the [TI MSP430F2xx/G2xx family guide](https://www.ti.com/lit/ug/slau144k/slau144k.pdf).

| Configuration | Timer value | Nominal interval | Drawn paddle |
| --- | --- | --- | --- |
| `dificuldade == 1` | `TA1CCR0 = 24999` | 50 ms | Two pixels |
| Alternate mode | `TA1CCR0 = 49999` | 100 ms | Three pixels |
| Button debounce | `TA0CCR0 = 49999` | 100 ms | — |

The ADC averages eight samples from one channel, then selects the other channel for a subsequent acquisition. Paddle mapping is `(media * 6) / 1023` for mode 1 and `(media * 5) / 1023` otherwise. These ranges keep the drawn paddles within the eight-pixel dimension.

Title frames use a 2,000,000-cycle delay; mode alternation uses 16,000,000 cycles. At nominal MCLK these are 125 ms and one second. Interrupt execution adds time to foreground delays; point/score delays also make effective game updates differ from the simple timer periods.

## Gameplay

Ball velocity components are integer steps of ±1. The ball reflects at the boundaries of the eight-pixel dimension. Paddle tests occur at `bola_y == 1` and `bola_y == 14`. Reaching `bola_y == 0` or `15` awards a point to the opposite side, delays briefly, and recenters the ball. When either score reaches three, the firmware displays the scores and returns to the title state.

## Original implementation details

These details are retained and documented, rather than changed during the repository organization:

- **Pause/resume resets the ball.** Returning through the foreground match branch reassigns its position/velocity before restarting Timer_A1.
- **Collision ranges differ from visible paddles.** Mode 1 draws two pixels but accepts `paddle - 1` through `paddle + 2`; the alternate draws three pixels but accepts `paddle - 1` through `paddle + 3`.
- **Mode selection uses bitwise complement.** `dificuldade = ~dificuldade` alternates `1` with its unsigned complement, rather than conventional mode values `1` and `2`.
- **Shared variables lack `volatile`.** Interrupt/foreground state and loop counters are shared. Compiler optimization and interrupt interleaving should be reviewed before changing the build settings.
- **Interrupt handlers perform blocking work.** Gameplay includes serial waits and cycle delays inside Timer_A1 interrupt execution, which can delay button and ADC servicing.
- **Initialization enables interrupts early.** Global interrupts are enabled before all peripheral setup is complete, and startup waits for the low-frequency oscillator fault to clear. Review initialization order and board clock configuration when reproducing the project.

The original firmware's SHA-256 is `1AB8058D62D8300E14CF682911A80128FDE8C7D1CEE8C65DA2E9D233BB801344`. Its contents are unchanged after moving it to `src/`. The reorganization verifies source integrity and documentation links; compilation and hardware behavior remain unverified in this update.

# C2000 Motor Control

Motor control firmware project based on the TI TMS320F28379D.

The goal of this project is to build a motor-control software stack from the low-level peripheral drivers up to the control and application layers, while making use of C2000-specific hardware features instead of treating the device like a generic MCU.

The firmware is developed incrementally. Each layer is kept relatively small and hardware-specific functionality is pushed down to the lowest reasonable layer.

## Target

- MCU: TMS320F28379D
- Board: LAUNCHXL-F28379D
- Toolchain: TI C2000 Compiler
- IDE: Code Composer Studio
- Language:
  - C for MCAL and control-related low-level code
  - C++ may be used in higher layers where it provides a clear architectural benefit

## Software Architecture

+--------------------------------------------------+
|                Application / Services            |
|                                                  |
|  State machine, diagnostics, communication, etc. |
+----------------------+---------------------------+
                       |
             +---------+---------+
             |                   |
             v                   v
+------------------------+  +------------------------+
|      HAL / Device      |  |      Control Core      |
|                        |  |                        |
| Motor PWM, current     |  | Clarke / Park,         |
| sensing, gate driver,  |  | PI control, SVPWM,     |
| encoder, fault handling|  | current / speed control|
+-----------+------------+  +------------------------+
            |
            v
+--------------------------------------------------+
|                       MCAL                       |
|                                                  |
|  GPIO  ePWM  ADC  DMA  CMPSS  X-BAR             |
|  PIE   CPU Timer  DAC  SCI  SPI                  |
+-------------------------+------------------------+
                          |
                          v
+--------------------------------------------------+
|                TMS320F28379D Hardware            |
+--------------------------------------------------+

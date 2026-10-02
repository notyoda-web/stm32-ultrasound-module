# STM32F103C8T6 Ultrasonic Distance Measurement Firmware (HC-SR04)

A high-precision, hardware-timed ultrasonic ranging firmware designed for the STM32F103C8T6 (Blue Pill) microcontroller. This project leverages separate timer channels for edge capture to measure echo durations deterministically with minimal CPU overhead, streaming distance data asynchronously over USART1.

---

## Technical Specifications & Architecture

* **MCU Core:** ARM Cortex-M3 (72 MHz operating frequency configured via external 8 MHz HSE crystal with PLL $\times 9$).
* **Trigger Generation (TIM3):** Configured to output a periodic trigger signal for the HC-SR04 sensor at 25 Hz using PWM Generation on Channel 1 (`PA6`).
* **Echo Measurement (TIM2):** Utilizes dual timer channels on `PA0` / `TIM2_CH1` to capture echo pulse widths directly in hardware:
  * **Channel 1 (Rising Edge):** Configured to capture the rising edge of the echo pulse and reset the counter timer.
  * **Channel 2 (Falling Edge):** Configured with indirect input capture to detect the falling edge, providing the direct pulse width in microseconds via a 1 µs timer tick (Prescaler: 71, 16-bit Counter Period: 65535).
* **Telemetry Interface:** USART1 configured at **115200 Baud, 8 Data Bits, No Parity, 1 Stop Bit (115200 8N1)** for transmitting formatted distance telemetry.

##physical limitation

Using separate timer channels for the rising and falling edges was chosen because the STM32F103 (Blue Pill) hardware lacks the ability to capture both rising and falling edges on a single channel simultaneously for this type of measurement. Attempting to configure a single channel for both edges cannot independently isolate the pulse width without manual state toggling and software edge tracking, which introduces latency and tracking errors. By dedicating Channel 1 to capture the rising edge and automatically reset the timer counter, and Channel 2 (in indirect mode) to capture the falling edge, the hardware directly yields the exact pulse width in microseconds with cycle-accurate precision. This design completely eliminates software ambiguity, overcomes the chip's single-channel limitations, ensures reliable timing across rapid sensor pings, and maintains clean, predictable telemetry streaming over UART.
---

## Hardware Wiring Configuration

Connections between the STM32 Blue Pill, the HC-SR04 ultrasonic sensor, and the telemetry adapter:

| Component / Peripheral | Component Pin | Blue Pill Pin | Functional Description |
| :--- | :--- | :--- | :--- |
| **HC-SR04 Sensor** | VCC | 5V | Sensor Power Supply |
| **HC-SR04 Sensor** | TRIG | **PA6** (`TIM3_CH1`) | Hardware Trigger Signal Input |
| **HC-SR04 Sensor** | ECHO | **PA0** (`TIM2_CH1`) | Ultrasonic Echo Return ( via 1k/2k voltage divider to 3.3V ) |
| **HC-SR04 Sensor** | GND | GND | Common System Ground |
| **USB-TTL Adapter** | TXD | **PA9** (`USART1_TX`) | MCU Telemetry Transmission Line |
| **USB-TTL Adapter** | RXD | **PA10** (`USART1_RX`) | MCU Telemetry Reception Line |
| **USB-TTL Adapter** | GND | GND | Common System Ground |

*Note: Ensure logic levels on the ECHO line are stepped down to a safe 3.3V level for STM32 GPIO pins using a resistive voltage divider.*

---

## Peripheral Configuration Summary

* **System Clock:** HCLK = 72 MHz, APB1 Prescaler = /2 (36 MHz timer clocks).
* **TIM3 (Trigger):** Internal Clock, Channel 1 = PWM Generation CH1, Prescaler = 71, Counter Period = 39999, Pulse = 10 (produces a $\approx 10\,\mu\text{s}$ trigger pulse at 25 Hz).
* **TIM2 (Echo Capture):** Internal Clock, Channel 1 = Input Capture direct mode (`PA0` / Rising Edge), Channel 2 = Input Capture indirect mode (Falling Edge), Prescaler = 71, Counter Period = 65535, Interrupt Enabled on TIM2 global interrupt.
* **USART1:** Asynchronous Mode, 115200 Baud, 8 Data Bits, 1 Stop Bit, No Parity, Interrupt Enabled.

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===== Hardware Pins (ESP32) =====
namespace Config {
  namespace Pins {
    constexpr uint8_t UART_RX  = 16;  // UART2 RX (connected to E32 TX)
    constexpr uint8_t UART_TX  = 17;  // UART2 TX (connected to E32 RX)
    constexpr uint8_t E32_AUX  = 19;  // E32 AUX status pin
    constexpr uint8_t LED      = 2;   // Built-in LED
    // M0 и M1 зафиксированы на GND (NORMAL MODE)
  }

  namespace Timing {
    constexpr uint32_t MODULE_INIT_TIMEOUT   = 5000;  // Timeout for module ready (ms)
    constexpr uint32_t MODULE_READY_POLL     = 100;   // Poll interval for AUX (ms)
    constexpr uint32_t SERIAL_INIT_DELAY     = 500;   // Delay after Serial.begin() (ms)
    constexpr uint32_t UART_INIT_DELAY       = 100;   // Delay after UART begin (ms)
    constexpr uint32_t MODULE_STARTUP_DELAY  = 500;   // Delay after e32.begin() (ms)
    constexpr uint32_t PING_INTERVAL         = 1000;  // Interval between PING messages (ms)
  }

  namespace Protocol {
    constexpr size_t RX_BUFFER_SIZE         = 2048;  // UART RX buffer size
    constexpr size_t MAX_MESSAGE_LENGTH     = 256;   // Max message length
    constexpr size_t MAX_SERIAL_INPUT       = 200;   // Max Serial input buffer
    constexpr uint32_t SERIAL_BAUD_RATE     = 115200; // USB Serial baud
    constexpr uint32_t LORA_BAUD_RATE       = 9600;  // LoRa module UART baud
  }
}

#endif // CONFIG_H

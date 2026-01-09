#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===== Platform Detection =====
// Detect which board we're compiling for
#if defined(ESP32)
  #define PLATFORM_ESP32 1
  #define PLATFORM_NAME "ESP32S"
#elif defined(__AVR_ATmega2560__)
  #define PLATFORM_MEGA2560 1
  #define PLATFORM_NAME "Arduino Mega 2560"
#else
  #error "Unsupported platform"
#endif

// ===== Hardware Configuration =====
namespace Config {
  namespace Pins {
    #ifdef PLATFORM_ESP32
      // ===== ESP32 Pins =====
      constexpr uint8_t UART_RX  = 16;  // UART2 RX (connected to E32 TX)
      constexpr uint8_t UART_TX  = 17;  // UART2 TX (connected to E32 RX)
      constexpr uint8_t E32_AUX  = 19;  // E32 AUX status pin
      constexpr uint8_t LED      = 21;  // External LED (220 Ohm resistor)
      constexpr uint8_t OLED_SDA = 23;  // SSD1306 I2C Data
      constexpr uint8_t OLED_SCL = 18;  // SSD1306 I2C Clock
      
    #elif defined(PLATFORM_MEGA2560)
      // ===== Arduino Mega 2560 Pins =====
      constexpr uint8_t UART_RX  = 18;  // UART1 RX (connected to E32 TX)
      constexpr uint8_t UART_TX  = 19;  // UART1 TX (connected to E32 RX)
      constexpr uint8_t E32_AUX  = 20;  // E32 AUX status pin
      constexpr uint8_t LED      = 13;  // Built-in LED
    #endif
    
    // M0 и M1 зафиксированы на GND (NORMAL MODE) - не управляются программно
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
    #ifdef PLATFORM_ESP32
      constexpr size_t RX_BUFFER_SIZE      = 2048;  // UART RX buffer size (ESP32)
    #elif defined(PLATFORM_MEGA2560)
      constexpr size_t RX_BUFFER_SIZE      = 256;   // UART RX buffer size (Mega - меньше памяти)
    #endif
    
    constexpr size_t MAX_MESSAGE_LENGTH     = 256;   // Max message length
    constexpr size_t MAX_SERIAL_INPUT       = 200;   // Max Serial input buffer
    constexpr uint32_t SERIAL_BAUD_RATE     = 115200; // USB Serial baud
    constexpr uint32_t LORA_BAUD_RATE       = 9600;  // LoRa module UART baud
  }

  namespace Display {
    constexpr uint8_t OLED_ADDRESS = 0x3C;  // SSD1306 I2C address (0x3C or 0x3D)
    constexpr uint8_t OLED_WIDTH   = 128;   // OLED width in pixels
    constexpr uint8_t OLED_HEIGHT  = 64;    // OLED height in pixels
    constexpr bool    OLED_ENABLED = true;  // Enable/disable OLED display
  }
}

#endif // CONFIG_H

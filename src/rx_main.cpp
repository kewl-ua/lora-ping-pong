/*
  ESP32S + E32-900T30D
  RX (приёмник) версия

  Wiring (ESP32 работает на 3.3V, делители НЕ нужны):
    ESP32 RX2/GPIO16 (UART2) <- E32 TXD
    ESP32 TX2/GPIO17 (UART2) -> E32 RXD
    ESP32 GPIO21             -> E32 M0
    ESP32 GPIO22             -> E32 M1
    ESP32 GPIO19             <- E32 AUX
*/

#include <Arduino.h>
#include <HardwareSerial.h>

// E32_TTL_1W and FREQUENCY_915 defined in build flags
#include "LoRa_E32.h"

// ===== Pins (ESP32) =====
static const uint8_t PIN_RX  = 16; // UART2 RX (connected to E32 TX)
static const uint8_t PIN_TX  = 17; // UART2 TX (connected to E32 RX)
static const uint8_t PIN_AUX = 19;
// M0 и M1 зафиксированы на GND (NORMAL MODE) - не управляются программно
static const uint8_t PIN_LED = 2;  // Built-in LED on ESP32

// HardwareSerial UART2 for LoRa module communication
HardwareSerial loraSerial(2);
LoRa_E32 e32(&loraSerial, PIN_AUX);

static bool checkModule() {
  // Ждем готовности модуля (AUX HIGH)
  Serial.println("Waiting for module ready (AUX HIGH)...");

  uint32_t startMs = millis();

  while (digitalRead(PIN_AUX) == LOW && millis() - startMs < 5000) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("AUX state: ");
  Serial.println(digitalRead(PIN_AUX) ? "HIGH (Ready)" : "LOW (Not ready)");
  
  if (digitalRead(PIN_AUX) == LOW) {
    Serial.println("ERROR: Module not ready. Check power and wiring.");
    return false;
  }
  
  Serial.println("Module ready! Using factory/pre-configured settings.");
  Serial.println("Note: M0=GND, M1=GND => NORMAL MODE (no configuration change possible)");
  return true;
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_AUX, INPUT);  // Внешние pull-up резисторы 4.7k на схеме

  Serial.begin(115200);
  delay(500);

  // HardwareSerial UART2 to LoRa module (RX=16, TX=17)
  loraSerial.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(100);

  Serial.println("Starting E32 module...");
  Serial.println("M0=GND, M1=GND => NORMAL MODE (fixed)");

  e32.begin();

  delay(500);  // Give module time to initialize

  Serial.println();
  Serial.println("Role: RX (receiver)");

  if (!checkModule()) {
    Serial.println("WARNING: Module check failed.");
  }
}

void loop() {
  // Читаем напрямую из UART построчно (быстрее и точнее)
  static String rxBuffer = "";
  
  while (loraSerial.available() > 0) {
    char c = loraSerial.read();
    
    if (c == '\n' || c == '\r') {
      if (rxBuffer.length() > 0) {
        // Полная строка получена - выводим немедленно
        Serial.print("RX< ");
        Serial.println(rxBuffer);
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        
        rxBuffer = "";  // Очищаем буфер для следующей строки
      }
    } else {
      rxBuffer += c;
      
      // Защита от переполнения
      if (rxBuffer.length() > 256) {
        Serial.println("Buffer overflow, clearing...");
        rxBuffer = "";
      }
    }
    
    yield();  // Даем время другим задачам
  }
}

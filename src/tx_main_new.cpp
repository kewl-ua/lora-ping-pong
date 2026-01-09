/*
  ESP32S + E32-900T30D
  TX (передатчик) - Beacon для TDOA навигации

  Wiring (ESP32 работает на 3.3V, делители НЕ нужны):
    ESP32 RX2/GPIO16 (UART2) <- E32 TXD
    ESP32 TX2/GPIO17 (UART2) -> E32 RXD
    ESP32 GPIO19             <- E32 AUX
    M0 и M1 зафиксированы на GND (NORMAL MODE)
*/

#include "config.h"
#include "lora_module.h"
#include "packet.h"

static uint32_t sequenceNumber = 0;

void setup() {
  // Инициализация LoRa модуля
  if (!loraModule.initialize()) {
    Serial.println("WARNING: Module initialization failed.");
  }
  
  Serial.println();
  Serial.println("===== TX MODE: TDOA Beacon =====");
  Serial.println("Sending packets with EUID for TDOA navigation");
  Serial.println("Type text in Serial Monitor to send custom messages.");
  Serial.println();
}

void loop() {
  // 1) Автоматическая отправка beacon пакетов
  static uint32_t lastSendMs = 0;
  
  const uint32_t now = millis();
  if (now - lastSendMs >= Config::Timing::PING_INTERVAL) {
    lastSendMs = now;
    
    // Формируем пакет с EUID и временной меткой
    String packet = buildPacket("BEACON", sequenceNumber++);
    
    if (loraModule.sendMessage(packet)) {
      Serial.print("TX> ");
      Serial.println(packet);
    }
  }
  
  // 2) Отправка пользовательских сообщений из Serial Monitor
  static String inputBuffer;
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    
    if (ch == '\r' || ch == '\n') {
      if (inputBuffer.length() > 0) {
        String packet = buildPacket(inputBuffer, sequenceNumber++);
        
        if (loraModule.sendMessage(packet)) {
          Serial.print("TX> ");
          Serial.println(packet);
        }
        
        inputBuffer = "";
      }
    } else {
      if (inputBuffer.length() < Config::Protocol::MAX_SERIAL_INPUT) {
        inputBuffer += ch;
      }
    }
  }
}

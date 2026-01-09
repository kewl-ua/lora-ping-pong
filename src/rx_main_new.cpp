/*
  ESP32S + E32-900T30D
  RX (приёмник) - Anchor для TDOA навигации

  Wiring (ESP32 работает на 3.3V, делители НЕ нужны):
    ESP32 RX2/GPIO16 (UART2) <- E32 TXD
    ESP32 TX2/GPIO17 (UART2) -> E32 RXD
    ESP32 GPIO19             <- E32 AUX
    M0 и M1 зафиксированы на GND (NORMAL MODE)
*/

#include "config.h"
#include "lora_module.h"
#include "packet.h"
#include "tdoa.h"

// Конфигурация этого anchor узла
static const uint8_t ANCHOR_ID = 0;    // Уникальный ID этого RX (0, 1, 2...)
static const float ANCHOR_X = 0.0;     // Координата X (метры)
static const float ANCHOR_Y = 0.0;     // Координата Y (метры)

void setup() {
  // Инициализация LoRa модуля
  if (!loraModule.initialize()) {
    Serial.println("WARNING: Module initialization failed.");
  }
  
  // Регистрация этого узла как anchor для TDOA
  tdoaNavigator.registerAnchor(ANCHOR_ID, ANCHOR_X, ANCHOR_Y);
  
  Serial.println();
  Serial.println("===== RX MODE: TDOA Anchor =====");
  Serial.print("Anchor ID: ");
  Serial.print(ANCHOR_ID);
  Serial.print(" at position (");
  Serial.print(ANCHOR_X);
  Serial.print(", ");
  Serial.print(ANCHOR_Y);
  Serial.println(")");
  Serial.println("Listening for beacon packets...");
  Serial.println();
}

void loop() {
  static String rxBuffer = "";
  
  while (loraModule.available() > 0) {
    uint32_t rxTime_us = micros();  // Захватываем время приема максимально точно
    char c = loraModule.read();
    
    if (c == '\n' || c == '\r') {
      if (rxBuffer.length() > 0) {
        // Парсим пакет
        PacketData packet = parsePacket(rxBuffer);
        
        if (packet.valid) {
          // Вычисляем статистику приема
          RxStats stats = calculateRxStats(packet, rxTime_us);
          
          // Сохраняем в TDOA navigator для будущих расчетов
          tdoaNavigator.processRxPacket(packet, stats);
          
          // Выводим информацию
          Serial.print("[");
          Serial.print(rxTime_us);
          Serial.print("µs] EUID:");
          Serial.print(packet.euid);
          Serial.print(" | SEQ:");
          Serial.print(packet.sequence);
          Serial.print(" | MSG:");
          Serial.print(packet.message);
          Serial.print(" | LAT:");
          Serial.print(stats.latency_us);
          Serial.print("µs | RSSI:");
          Serial.print(stats.rssi);
          Serial.print("dBm | SNR:");
          Serial.print(stats.snr);
          Serial.println("dB");
        } else {
          // Неизвестный формат
          Serial.print("[");
          Serial.print(rxTime_us);
          Serial.print("µs] RAW< ");
          Serial.println(rxBuffer);
        }
        
        rxBuffer = "";
      }
    } else {
      rxBuffer += c;
      
      // Защита от переполнения
      if (rxBuffer.length() > Config::Protocol::MAX_MESSAGE_LENGTH) {
        Serial.println("Buffer overflow!");
        rxBuffer = "";
      }
    }
    
    yield();
  }
}

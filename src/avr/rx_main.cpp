/*
  Arduino Mega 2560 + E32-900T30D
  RX (приёмник) - Anchor для TDOA навигации

  Hardware:
    Arduino Mega 2560 (5V logic, требуется делитель напряжения для E32 TX)
    LoRa UART1: RX1/GPIO18, TX1/GPIO19
    E32 AUX: GPIO20
    LED: GPIO13 (встроенный)
    
  ВНИМАНИЕ: E32 работает на 3.3V!
    - Mega TX (GPIO19) -> делитель напряжения 5V→3.3V -> E32 RX
    - E32 TX -> напрямую Mega RX (GPIO18) (3.3V безопасно для 5V входа)
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
    Serial.println("ERROR: Module initialization failed!");
    while (1) { 
      digitalWrite(Config::Pins::LED, !digitalRead(Config::Pins::LED));
      delay(200); // Мигание при ошибке
    }
  }
  
  // Регистрация этого узла как anchor для TDOA
  tdoaNavigator.registerAnchor(ANCHOR_ID, ANCHOR_X, ANCHOR_Y);
  
  Serial.println();
  Serial.println("===== Arduino Mega 2560 RX MODE =====");
  Serial.println("Platform: ATmega2560 @ 16MHz");
  Serial.print("Anchor ID: ");
  Serial.print(ANCHOR_ID);
  Serial.print(" at position (");
  Serial.print(ANCHOR_X);
  Serial.print(", ");
  Serial.print(ANCHOR_Y);
  Serial.println(")");
  Serial.println("Listening for LoRa packets...");
  Serial.println();
}

void loop() {
  static String rxBuffer;
  
  // Читаем UART побайтово для точного захвата времени
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
          Serial.print("us] EUID:");
          Serial.print(packet.euid);
          Serial.print(" | SEQ:");
          Serial.print(packet.sequence);
          Serial.print(" | MSG:");
          Serial.print(packet.message);
          Serial.print(" | LAT:");
          Serial.print(stats.latency_us);
          Serial.print("us | RSSI:");
          Serial.print(stats.rssi);
          Serial.print("dBm | SNR:");
          Serial.print(stats.snr);
          Serial.println("dB");
          
          // Мигание LED при приеме
          digitalWrite(Config::Pins::LED, HIGH);
          delay(10);
          digitalWrite(Config::Pins::LED, LOW);
        } else {
          // Неизвестный формат
          Serial.print("[");
          Serial.print(rxTime_us);
          Serial.print("us] RAW< ");
          Serial.println(rxBuffer);
        }
        
        rxBuffer = "";
      }
    } else {
      rxBuffer += c;
      
      // Защита от переполнения (Mega имеет меньше RAM)
      if (rxBuffer.length() > Config::Protocol::MAX_MESSAGE_LENGTH) {
        Serial.println("Buffer overflow!");
        rxBuffer = "";
      }
    }
  }
}

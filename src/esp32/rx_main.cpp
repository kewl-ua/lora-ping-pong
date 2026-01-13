/*
  ESP32S + E32-900T30D + SSD1306 OLED
  RX (приёмник) - Anchor для TDOA навигации

  Hardware:
    ESP32 v1302
    LoRa UART: RX2/GPIO16, TX2/GPIO17
    E32 AUX: GPIO19
    LED: GPIO21 (220 Ohm resistor)
    OLED I2C: SDA=GPIO23, SCL=GPIO18
*/

#include "config.h"
#include "lora_module.h"
#include "packet.h"
#include "tdoa.h"
#include "display.h"

// Конфигурация этого anchor узла
static const uint8_t ANCHOR_ID = 0;    // Уникальный ID этого RX (0, 1, 2...)
static const float ANCHOR_X = 0.0;     // Координата X (метры)
static const float ANCHOR_Y = 0.0;     // Координата Y (метры)

void setup() {
  // Инициализация дисплея (до LoRa модуля)
  displayManager.initialize();
  displayManager.showInitScreen("RX ANCHOR");
  delay(1000);
  
  // Инициализация LoRa модуля
  if (!loraModule.initialize()) {
    Serial.println("ERROR: Module initialization failed!");
    displayManager.showError("LoRa Init Failed");
    while (1) { delay(1000); }
  }
  
  // Регистрация этого узла как anchor для TDOA
  tdoaNavigator.registerAnchor(ANCHOR_ID, ANCHOR_X, ANCHOR_Y);
  
  Serial.println();
  Serial.println("===== ESP32 RX MODE: TDOA Anchor =====");
  Serial.println("Platform: ESP32 v1302 with OLED display");
  Serial.print("Anchor ID: ");
  Serial.print(ANCHOR_ID);
  Serial.print(" at position (");
  Serial.print(ANCHOR_X);
  Serial.print(", ");
  Serial.print(ANCHOR_Y);
  Serial.println(")");
  Serial.println("Listening for LoRa packets...");
  
  // Выводим ключевые параметры перед началом работы
  Serial.println();
  Serial.println(">>> KEY PARAMETERS <<<");
  ResponseStructContainer config = loraModule.getE32()->getConfiguration();
  if (config.status.code == E32_SUCCESS) {
    Configuration* cfg = (Configuration*)config.data;
    Serial.print("ADDH/ADDL: 0x");
    Serial.print(cfg->ADDH, HEX);
    Serial.print("/0x");
    Serial.println(cfg->ADDL, HEX);
    Serial.print("Channel: ");
    Serial.println(cfg->CHAN);
    Serial.print("Air Data Rate: ");
    Serial.println(cfg->SPED.getAirDataRate());
    config.close();
  }
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>");
  Serial.println();
}

void loop() {
  static String rxBuffer;
  static uint32_t lastDebugMs = 0;
  
  // Периодический debug вывод что живы
  if (millis() - lastDebugMs >= 5000) {
    lastDebugMs = millis();
    Serial.print("RX alive, buffer: ");
    Serial.print(rxBuffer.length());
    Serial.print(" bytes, available: ");
    Serial.println(loraModule.available());
  }
  
  // Читаем UART побайтово для точного захвата времени
  while (loraModule.available() > 0) {
    uint32_t rxTime_us = micros();  // Захватываем время приема максимально точно
    char c = loraModule.read();
    
    Serial.print("RX byte: 0x");
    Serial.print((byte)c, HEX);
    Serial.print(" '");
    if (c >= 32 && c < 127) Serial.print(c);
    else Serial.print(".");
    Serial.println("'");
    
    if (c == '\n' || c == '\r') {
      if (rxBuffer.length() > 0) {
        Serial.println("Parsing packet...");
        // Парсим пакет
        PacketData packet = parsePacket(rxBuffer);
        
        if (packet.valid) {
          // Вычисляем статистику приема
          RxStats stats = calculateRxStats(packet, rxTime_us);
          
          // Сохраняем в TDOA navigator для будущих расчетов
          tdoaNavigator.processRxPacket(packet, stats);
          
          // Обновление дисплея
          displayManager.showRxStatus(packet, stats);
          
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
    
    yield();  // Для watchdog
  }
}

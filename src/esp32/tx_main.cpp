/*
  ESP32S + E32-900T30D + SSD1306 OLED
  TX (передатчик) - Beacon для TDOA навигации

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
#include "display.h"

static uint32_t sequenceNumber = 0;

void setup() {
  // Инициализация дисплея (до LoRa модуля)
  displayManager.initialize();
  displayManager.showInitScreen("TX BEACON");
  delay(1000);
  
  // Инициализация LoRa модуля
  if (!loraModule.initialize()) {
    Serial.println("ERROR: Module initialization failed!");
    displayManager.showError("LoRa Init Failed");
    while (1) { delay(1000); }
  }
  
  Serial.println();
  Serial.println("===== ESP32 TX MODE: TDOA Beacon =====");
  Serial.println("Platform: ESP32 v1302 with OLED display");
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
    bool success = loraModule.sendMessage(packet);
    
    Serial.print("TX> ");
    Serial.print(packet);
    Serial.println(success ? " [OK]" : " [FAIL]");
    
    // Обновление дисплея
    displayManager.showTxStatus(sequenceNumber - 1, "BEACON", success);
  }
  
  // 2) Отправка пользовательских сообщений из Serial Monitor
  static String inputBuffer;
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    
    if (ch == '\r' || ch == '\n') {
      if (inputBuffer.length() > 0) {
        String packet = buildPacket(inputBuffer, sequenceNumber++);
        bool success = loraModule.sendMessage(packet);
        
        Serial.print("TX> ");
        Serial.print(packet);
        Serial.println(success ? " [OK]" : " [FAIL]");
        
        // Обновление дисплея
        displayManager.showTxStatus(sequenceNumber - 1, inputBuffer, success);
        
        inputBuffer = "";
      }
    } else {
      if (inputBuffer.length() < Config::Protocol::MAX_SERIAL_INPUT) {
        inputBuffer += ch;
      }
    }
  }
}

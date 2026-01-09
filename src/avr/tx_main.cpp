/*
  Arduino Mega 2560 + E32-900T30D
  TX (передатчик) - Beacon для TDOA навигации

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

static uint32_t sequenceNumber = 0;

void setup() {
  // Инициализация LoRa модуля
  if (!loraModule.initialize()) {
    Serial.println("ERROR: Module initialization failed!");
    while (1) { 
      digitalWrite(Config::Pins::LED, !digitalRead(Config::Pins::LED));
      delay(200); // Мигание при ошибке
    }
  }
  
  Serial.println();
  Serial.println("===== Arduino Mega 2560 TX MODE =====");
  Serial.println("Platform: ATmega2560 @ 16MHz");
  Serial.println("Sending TDOA beacon packets");
  Serial.println("Type text in Serial Monitor to send custom messages.");
  Serial.println();
}

void loop() {
  // 1) Автоматическая отправка beacon пакетов
  static uint32_t lastSendMs = 0;
  
  const uint32_t now = millis();
  if (now - lastSendMs >= Config::Timing::PING_INTERVAL) {
    lastSendMs = now;
    
    // Проверка готовности модуля перед отправкой
    if (digitalRead(Config::Pins::E32_AUX) == LOW) {
      Serial.println("WARNING: Module not ready (AUX=LOW), skipping send");
      return;
    }
    
    // Формируем пакет с EUID и временной меткой
    String packet = buildPacket("BEACON", sequenceNumber++);
    packet += "\n";  // Add newline terminator for RX parsing
    uint32_t txTime = micros();
    bool success = loraModule.sendMessage(packet);
    uint32_t txDuration = micros() - txTime;
    
    Serial.print("TX> [");
    Serial.print(txTime);
    Serial.print("us] ");
    Serial.print(packet);
    
    if (success) {
      Serial.print(" [OK] (");
      Serial.print(txDuration);
      Serial.println("us)");
      
      // Мигание LED при успешной отправке
      digitalWrite(Config::Pins::LED, HIGH);
      delay(50);
      digitalWrite(Config::Pins::LED, LOW);
    } else {
      Serial.println(" [FAIL - Module error!]");
    }
  }
  
  // 2) Отправка пользовательских сообщений из Serial Monitor
  static String inputBuffer;
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    
    if (ch == '\r' || ch == '\n') {
      if (inputBuffer.length() > 0) {
        // Проверка готовности модуля перед отправкой
        if (digitalRead(Config::Pins::E32_AUX) == LOW) {
          Serial.println("WARNING: Module not ready (AUX=LOW), skipping send");
          inputBuffer = "";
          continue;
        }
        
        String packet = buildPacket(inputBuffer, sequenceNumber++);
        packet += "\n";  // Add newline terminator for RX parsing
        uint32_t txTime = micros();
        bool success = loraModule.sendMessage(packet);
        uint32_t txDuration = micros() - txTime;
        
        Serial.print("TX> [");
        Serial.print(txTime);
        Serial.print("us] ");
        Serial.print(packet);
        
        if (success) {
          Serial.print(" [OK] (");
          Serial.print(txDuration);
          Serial.println("us)");
          
          // Мигание LED при успешной отправке
          digitalWrite(Config::Pins::LED, HIGH);
          delay(50);
          digitalWrite(Config::Pins::LED, LOW);
        } else {
          Serial.println(" [FAIL - Module error!]");
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

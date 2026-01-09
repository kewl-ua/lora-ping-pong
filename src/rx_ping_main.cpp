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

// ===== Timing constants (ms) =====
static const uint32_t MODULE_INIT_TIMEOUT = 5000;   // Timeout for module ready (AUX HIGH)
static const uint32_t MODULE_READY_POLL   = 100;    // Poll interval for AUX ready
static const uint32_t SERIAL_INIT_DELAY   = 500;    // Delay after Serial.begin()
static const uint32_t UART_INIT_DELAY     = 100;    // Delay after UART begin
static const uint32_t MODULE_STARTUP_DELAY = 500;   // Delay after e32.begin()

// ===== Buffer and protocol constants =====
static const size_t RX_BUFFER_SIZE        = 2048;   // UART RX buffer size
static const size_t MAX_MESSAGE_LENGTH    = 256;    // Max message length in rxBuffer
static const uint32_t SERIAL_BAUD_RATE   = 115200; // USB Serial baud
static const uint32_t LORA_BAUD_RATE     = 9600;   // LoRa module UART baud

// HardwareSerial UART2 for LoRa module communication
HardwareSerial loraSerial(2);
LoRa_E32 e32(&loraSerial, PIN_AUX);

// ===== Packet parsing =====
struct PacketData {
  String euid;
  String message;
  uint32_t txTime;
  bool valid;
};

// Парсинг пакета формата: EUID:<id>,MSG:<message>,TIME:<ms>
PacketData parsePacket(const String& packet) {
  PacketData data;
  data.valid = false;
  
  int euidStart = packet.indexOf("EUID:");
  int msgStart = packet.indexOf(",MSG:");
  int timeStart = packet.indexOf(",TIME:");
  
  if (euidStart != -1 && msgStart != -1 && timeStart != -1) {
    data.euid = packet.substring(euidStart + 5, msgStart);
    data.message = packet.substring(msgStart + 5, timeStart);
    data.txTime = packet.substring(timeStart + 6).toInt();
    data.valid = true;
  }
  
  return data;
}

static bool checkModule() {
  // Ждем готовности модуля (AUX HIGH)
  Serial.println("Waiting for module ready (AUX HIGH)...");

  uint32_t startMs = millis();

  while (digitalRead(PIN_AUX) == LOW && millis() - startMs < MODULE_INIT_TIMEOUT) {
    delay(MODULE_READY_POLL);
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

  Serial.begin(SERIAL_BAUD_RATE);
  delay(SERIAL_INIT_DELAY);

  // HardwareSerial UART2 to LoRa module (RX=16, TX=17)
  loraSerial.begin(LORA_BAUD_RATE, SERIAL_8N1, PIN_RX, PIN_TX);
  loraSerial.setRxBufferSize(RX_BUFFER_SIZE);
  delay(UART_INIT_DELAY);

  Serial.println("Starting E32 module...");
  Serial.println("M0=GND, M1=GND => NORMAL MODE (fixed)");
  
  // Диагностика перед инициализацией
  Serial.print("GPIO19 (AUX) initial state: ");
  Serial.println(digitalRead(PIN_AUX) ? "HIGH" : "LOW");

  e32.begin();

  delay(MODULE_STARTUP_DELAY);  // Give module time to initialize

  Serial.println();
  Serial.println("Role: RX (receiver)");

  if (!checkModule()) {
    Serial.println("WARNING: Module check failed.");
  }
}

void loop() {
  // Читаем напрямую из UART построчно с временными метками
  static String rxBuffer = "";
  
  uint32_t startMicros = micros();
  
  while (loraSerial.available() > 0) {
    // Захватываем время ДО чтения байта для максимальной точности
    uint32_t rxTime_us = micros();  // микросекунды
    
    char c = loraSerial.read();
    
    if (c == '\n' || c == '\r') {
      if (rxBuffer.length() > 0) {
        // Парсим пакет
        PacketData pkt = parsePacket(rxBuffer);
        
        if (pkt.valid) {
          // Получаем RSSI (Received Signal Strength Indicator)
          // Примечание: E32 не всегда предоставляет RSSI в NORMAL MODE
          // Для получения RSSI нужен специальный режим или AT команды
          int rssi = -100;  // Заглушка, библиотека не предоставляет RSSI в прозрачном режиме
          int snr = 0;      // SNR тоже недоступен в NORMAL MODE
          
          // Вычисляем задержку
          uint32_t rxTime = millis();
          int32_t latency = rxTime - pkt.txTime;
          
          // Выводим полную информацию
          Serial.print("[");
          Serial.print(rxTime_us);
          Serial.print("µs] EUID:");
          Serial.print(pkt.euid);
          Serial.print(" | MSG:");
          Serial.print(pkt.message);
          Serial.print(" | LAT:");
          Serial.print(latency);
          Serial.print("ms | RSSI:");
          Serial.print(rssi);
          Serial.print("dBm | SNR:");
          Serial.print(snr);
          Serial.println("dB");
        } else {
          // Неизвестный формат - выводим как есть
          Serial.print("[");
          Serial.print(rxTime_us);
          Serial.print("µs] RX< ");
          Serial.println(rxBuffer);
        }
        
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        rxBuffer = "";  // Очищаем буфер для следующей строки
      }
    } else {
      rxBuffer += c;
      
      // Защита от переполнения
      if (rxBuffer.length() > MAX_MESSAGE_LENGTH) {
        Serial.println("Buffer overflow, clearing...");
        rxBuffer = "";
      }
    }
    
    yield();  // Даем время другим задачам
  }
}

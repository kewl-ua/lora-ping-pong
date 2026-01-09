/*
  ESP32S + E32-900T30D
  TX (передатчик) версия

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
static const uint32_t MODULE_INIT_TIMEOUT    = 5000;   // Timeout for module ready (AUX HIGH)
static const uint32_t MODULE_READY_POLL      = 100;    // Poll interval for AUX ready
static const uint32_t SERIAL_INIT_DELAY      = 500;    // Delay after Serial.begin()
static const uint32_t UART_INIT_DELAY        = 100;    // Delay after UART begin
static const uint32_t MODULE_STARTUP_DELAY   = 500;    // Delay after e32.begin()
static const uint32_t PING_INTERVAL          = 1000;   // Interval between PING messages (ms)
static const size_t MAX_SERIAL_INPUT_LENGTH  = 200;    // Max length for Serial input buffer

// ===== Buffer and protocol constants =====
static const uint32_t SERIAL_BAUD_RATE      = 115200; // USB Serial baud
static const uint32_t LORA_BAUD_RATE        = 9600;   // LoRa module UART baud

// HardwareSerial UART2 for LoRa module communication
HardwareSerial loraSerial(2);
LoRa_E32 e32(&loraSerial, PIN_AUX);

// ===== EUID Generation =====
// Генерация уникального ID пакета на основе счетчика и микросекунд
static uint32_t packetCounter = 0;

String generateEUID() {
  // Формат: COUNTER_MICROS (например: 123_4567890)
  uint32_t us = micros();
  return String(packetCounter++) + "_" + String(us);
}

// Формирование пакета с EUID и данными
// Формат: EUID:<id>,MSG:<message>,TIME:<ms>
String buildPacket(const String& message) {
  String euid = generateEUID();
  uint32_t timestamp = millis();
  
  return "EUID:" + euid + ",MSG:" + message + ",TIME:" + String(timestamp);
}

static void printStatus(const char* tag, ResponseStatus& st) {
  Serial.print(tag);
  Serial.print(": code=");
  Serial.print(st.code);
  Serial.print(" desc=");
  Serial.println(st.getResponseDescription());
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

static void sendLine(const String& line) {
  if (line.length() == 0) return;

  ResponseStatus rs = e32.sendMessage(line);
  printStatus("sendMessage", rs);

  if (rs.code == E32_SUCCESS) {
    Serial.print("TX> ");
    Serial.println(line);
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
  }
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_AUX, INPUT);  // Внешние pull-up резисторы 4.7k на схеме

  Serial.begin(SERIAL_BAUD_RATE);
  delay(SERIAL_INIT_DELAY);

  // HardwareSerial UART2 to LoRa module (RX=16, TX=17)
  loraSerial.begin(LORA_BAUD_RATE, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(UART_INIT_DELAY);

  Serial.println("Starting E32 module...");
  Serial.println("M0=GND, M1=GND => NORMAL MODE (fixed)");
  e32.begin();
  delay(MODULE_STARTUP_DELAY);  // Give module time to initialize

  Serial.println();
  Serial.println("Role: TX (sender)");

  if (!checkModule()) {
    Serial.println("WARNING: Module check failed.");
  }

  Serial.println("Sending PING every second...");
  Serial.println("Type text in Serial Monitor to send custom messages.");
}

void loop() {
  // 1) Отправка по таймеру (ping)
  static uint32_t lastSendMs = 0;

  const uint32_t now = millis();
  if (now - lastSendMs >= PING_INTERVAL) {
    lastSendMs = now;
    // Создаем пакет с EUID
    String packet = buildPacket("PING");
    sendLine(packet);
  }

  // 2) Отправка того, что введёте в Serial Monitor
  static String buf;
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (buf.length() > 0) {
        // Оборачиваем пользовательское сообщение в пакет с EUID
        String packet = buildPacket(buf);
        sendLine(packet);
        buf = "";
      }
    } else {
      if (buf.length() < MAX_SERIAL_INPUT_LENGTH) buf += ch;
    }
  }
}

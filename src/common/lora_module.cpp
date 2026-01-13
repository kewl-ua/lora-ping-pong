#include "lora_module.h"

LoRaModule loraModule;

// Инициализация для разных платформ
#ifdef PLATFORM_ESP32
LoRaModule::LoRaModule()
  : loraSerial(2),  // ESP32: UART2
    e32(&loraSerial, Config::Pins::E32_AUX, Config::Pins::E32_M0, Config::Pins::E32_M1) {
}
#elif defined(PLATFORM_MEGA2560)
LoRaModule::LoRaModule()
  : loraSerial(Serial1),  // Mega: UART1
    e32(&Serial1, Config::Pins::E32_AUX, Config::Pins::E32_M0, Config::Pins::E32_M1) {
}
#endif

bool LoRaModule::initialize() {
  // Настройка пинов
  pinMode(Config::Pins::LED, OUTPUT);
  digitalWrite(Config::Pins::LED, LOW);
  pinMode(Config::Pins::E32_AUX, INPUT);
  
  // USB Serial
  Serial.begin(Config::Protocol::SERIAL_BAUD_RATE);
  delay(Config::Timing::SERIAL_INIT_DELAY);
  
  // Диагностика перед инициализацией
  Serial.print("GPIO");
  Serial.print(Config::Pins::E32_AUX);
  Serial.print(" (AUX) initial state: ");
  Serial.println(digitalRead(Config::Pins::E32_AUX) ? "HIGH" : "LOW");
  
  // LoRa UART
  #ifdef PLATFORM_ESP32
    loraSerial.begin(Config::Protocol::LORA_BAUD_RATE, SERIAL_8N1, 
                     Config::Pins::UART_RX, Config::Pins::UART_TX);
  #elif defined(PLATFORM_MEGA2560)
    Serial1.begin(Config::Protocol::LORA_BAUD_RATE);
  #endif
  delay(Config::Timing::UART_INIT_DELAY);
  
  Serial.println("Starting E32 module...");
  Serial.println("M0/M1 pins under software control");

  e32.begin();
  delay(Config::Timing::MODULE_STARTUP_DELAY);
  
  return checkReady();
}

bool LoRaModule::checkReady() {
  Serial.println("Waiting for module ready (AUX HIGH)...");
  uint32_t startMs = millis();
  
  while (digitalRead(Config::Pins::E32_AUX) == LOW && 
         millis() - startMs < Config::Timing::MODULE_INIT_TIMEOUT) {
    delay(Config::Timing::MODULE_READY_POLL);
    Serial.print(".");
  }
  Serial.println();
  
  Serial.print("AUX state: ");
  Serial.println(digitalRead(Config::Pins::E32_AUX) ? "HIGH (Ready)" : "LOW (Not ready)");
  
  if (digitalRead(Config::Pins::E32_AUX) == LOW) {
    Serial.println("ERROR: Module not ready. Check power and wiring.");
    return false;
  }
  
  // Читаем конфигурацию модуля
  Serial.println("Reading module configuration...");
  ResponseStructContainer config = e32.getConfiguration();
  if (config.status.code == E32_SUCCESS) {
    Configuration* cfg = (Configuration*)config.data;
    Serial.println("=== Module Configuration ===");
    Serial.print("ADDH: 0x"); Serial.println(cfg->ADDH, HEX);
    Serial.print("ADDL: 0x"); Serial.println(cfg->ADDL, HEX);
    Serial.print("Channel: "); Serial.println(cfg->CHAN);
    Serial.print("Air Data Rate: "); Serial.println(cfg->SPED.getAirDataRate());
    Serial.print("UART Baud: "); Serial.println(cfg->SPED.getUARTBaudRate());
    Serial.print("UART Parity: "); Serial.println(cfg->SPED.getUARTParityDescription());
    Serial.print("Transmission Mode: "); Serial.println(cfg->OPTION.getFixedTransmissionDescription());
    Serial.print("Transmission Power: "); Serial.println(cfg->OPTION.getTransmissionPowerDescription());
    Serial.print("FEC: "); Serial.println(cfg->OPTION.getFECDescription());
    Serial.println("============================");
    config.close();
  } else {
    Serial.print("Failed to read config: ");
    Serial.println(config.status.getResponseDescription());
  }
  
  Serial.println("Module ready!");
  return true;
}

bool LoRaModule::sendMessage(const String& message) {
  if (message.length() == 0) return false;
  
  ResponseStatus rs = e32.sendMessage(message);
  printStatus("sendMessage", rs);
  
  if (rs.code == E32_SUCCESS) {
    digitalWrite(Config::Pins::LED, !digitalRead(Config::Pins::LED));
    return true;
  }
  
  return false;
}

int LoRaModule::available() {
  return loraSerial.available();
}

char LoRaModule::read() {
  return loraSerial.read();
}

void LoRaModule::printStatus(const char* tag, ResponseStatus& st) {
  Serial.print(tag);
  Serial.print(": code=");
  Serial.print(st.code);
  Serial.print(" desc=");
  Serial.println(st.getResponseDescription());
}

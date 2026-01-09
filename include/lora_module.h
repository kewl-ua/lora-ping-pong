#ifndef LORA_MODULE_H
#define LORA_MODULE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "LoRa_E32.h"
#include "config.h"

// ===== LoRa Module Management =====

class LoRaModule {
public:
  LoRaModule();
  
  // Инициализация модуля
  bool initialize();
  
  // Проверка готовности модуля (AUX HIGH)
  bool checkReady();
  
  // Отправка сообщения
  bool sendMessage(const String& message);
  
  // Проверка доступности данных для чтения
  int available();
  
  // Чтение байта
  char read();
  
  // Получить указатель на Serial для расширенных операций
  HardwareSerial* getSerial() { return &loraSerial; }
  
private:
  HardwareSerial loraSerial;
  LoRa_E32 e32;
  
  void printStatus(const char* tag, ResponseStatus& st);
};

// Глобальный экземпляр (определен в lora_module.cpp)
extern LoRaModule loraModule;

#endif // LORA_MODULE_H

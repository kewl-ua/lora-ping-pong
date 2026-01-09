#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "config.h"

// Forward declarations для всех платформ
struct PacketData;
struct RxStats;

#ifdef PLATFORM_ESP32
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  #include "packet.h"
#endif

class DisplayManager {
public:
  DisplayManager();
  
  // Инициализация дисплея
  bool initialize();
  
  // TX режим: показать статус передачи
  void showTxStatus(uint32_t sequence, const String& message, bool success);
  
  // RX режим: показать принятый пакет
  void showRxStatus(const PacketData& packet, const RxStats& stats);
  
  // Показать экран инициализации
  void showInitScreen(const String& mode);
  
  // Показать ошибку модуля
  void showError(const String& error);
  
  // Очистить дисплей
  void clear();
  
private:
#ifdef PLATFORM_ESP32
  Adafruit_SSD1306* display;
  bool isEnabled;
  
  // Форматирование времени
  String formatTime(uint32_t micros);
  
  // Обрезка длинных строк
  String truncateString(const String& str, uint8_t maxLen);
#endif
};

extern DisplayManager displayManager;

#endif // DISPLAY_H

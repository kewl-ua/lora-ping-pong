#ifndef TDOA_H
#define TDOA_H

#include <Arduino.h>
#include "packet.h"

// ===== TDOA (Time Difference of Arrival) Navigation =====
// Для GPS-less навигации по LoRa

struct Position2D {
  float x;  // Координата X (метры)
  float y;  // Координата Y (метры)
  bool valid;
  
  Position2D() : x(0), y(0), valid(false) {}
};

struct AnchorNode {
  uint8_t id;
  float x;
  float y;
  uint32_t lastRxTime_us;
  
  AnchorNode() : id(0), x(0), y(0), lastRxTime_us(0) {}
};

class TDOANavigator {
public:
  TDOANavigator();
  
  // Регистрация anchor узла (RX станции с известными координатами)
  void registerAnchor(uint8_t id, float x, float y);
  
  // Обработка принятого пакета на anchor узле
  void processRxPacket(const PacketData& packet, const RxStats& stats);
  
  // Вычисление позиции на основе TDOA (минимум 3 anchor)
  Position2D calculatePosition(const String& euid);
  
  // Получить количество зарегистрированных anchor
  uint8_t getAnchorCount() const { return anchorCount; }
  
private:
  static constexpr uint8_t MAX_ANCHORS = 8;
  AnchorNode anchors[MAX_ANCHORS];
  uint8_t anchorCount;
  
  // Хранение временных меток для TDOA расчетов
  struct TDOAMeasurement {
    String euid;
    uint32_t rxTimes_us[MAX_ANCHORS];
    uint8_t rxCount;
    uint32_t lastUpdate_ms;
  };
  
  static constexpr uint8_t MAX_MEASUREMENTS = 10;
  TDOAMeasurement measurements[MAX_MEASUREMENTS];
  
  // Поиск/создание записи измерения по EUID
  TDOAMeasurement* findOrCreateMeasurement(const String& euid);
  
  // Триангуляция по TDOA
  Position2D trilaterate(const TDOAMeasurement& meas);
};

// Глобальный экземпляр (определен в tdoa.cpp)
extern TDOANavigator tdoaNavigator;

#endif // TDOA_H

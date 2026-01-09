#include "tdoa.h"

TDOANavigator tdoaNavigator;

TDOANavigator::TDOANavigator() : anchorCount(0) {
  // Очистка массивов измерений
  for (uint8_t i = 0; i < MAX_MEASUREMENTS; i++) {
    measurements[i].euid = "";
    measurements[i].rxCount = 0;
    measurements[i].lastUpdate_ms = 0;
  }
}

void TDOANavigator::registerAnchor(uint8_t id, float x, float y) {
  if (anchorCount >= MAX_ANCHORS) {
    Serial.println("TDOA: Max anchors reached!");
    return;
  }
  
  anchors[anchorCount].id = id;
  anchors[anchorCount].x = x;
  anchors[anchorCount].y = y;
  anchorCount++;
  
  Serial.print("TDOA: Registered anchor #");
  Serial.print(id);
  Serial.print(" at (");
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.println(")");
}

void TDOANavigator::processRxPacket(const PacketData& packet, const RxStats& stats) {
  if (!packet.valid) return;
  
  TDOAMeasurement* meas = findOrCreateMeasurement(packet.euid);
  if (!meas) return;
  
  // Сохраняем время приема на этом anchor узле
  // Предполагаем что anchor ID = 0 для первого RX, 1 для второго и т.д.
  // В реальности нужно передавать anchor ID
  if (meas->rxCount < MAX_ANCHORS) {
    meas->rxTimes_us[meas->rxCount] = stats.rxTime_us;
    meas->rxCount++;
    meas->lastUpdate_ms = millis();
  }
  
  Serial.print("TDOA: Recorded RX time for EUID:");
  Serial.print(packet.euid);
  Serial.print(" (");
  Serial.print(meas->rxCount);
  Serial.println(" anchors)");
}

Position2D TDOANavigator::calculatePosition(const String& euid) {
  Position2D pos;
  
  // Найти измерение
  TDOAMeasurement* meas = nullptr;
  for (uint8_t i = 0; i < MAX_MEASUREMENTS; i++) {
    if (measurements[i].euid == euid) {
      meas = &measurements[i];
      break;
    }
  }
  
  if (!meas || meas->rxCount < 3) {
    Serial.println("TDOA: Not enough measurements (need 3+ anchors)");
    return pos;
  }
  
  // TODO: Реализовать TDOA триангуляцию
  // Это заглушка на будущее
  pos = trilaterate(*meas);
  
  return pos;
}

TDOANavigator::TDOAMeasurement* TDOANavigator::findOrCreateMeasurement(const String& euid) {
  // Поиск существующего
  for (uint8_t i = 0; i < MAX_MEASUREMENTS; i++) {
    if (measurements[i].euid == euid) {
      return &measurements[i];
    }
  }
  
  // Создание нового (замена самого старого)
  uint8_t oldestIdx = 0;
  uint32_t oldestTime = measurements[0].lastUpdate_ms;
  
  for (uint8_t i = 1; i < MAX_MEASUREMENTS; i++) {
    if (measurements[i].lastUpdate_ms < oldestTime) {
      oldestTime = measurements[i].lastUpdate_ms;
      oldestIdx = i;
    }
  }
  
  measurements[oldestIdx].euid = euid;
  measurements[oldestIdx].rxCount = 0;
  measurements[oldestIdx].lastUpdate_ms = millis();
  
  return &measurements[oldestIdx];
}

Position2D TDOANavigator::trilaterate(const TDOAMeasurement& meas) {
  Position2D pos;
  
  // TODO: Реализовать алгоритм TDOA триангуляции
  // Гиперболическая триангуляция на основе разницы времен прихода
  // https://en.wikipedia.org/wiki/Multilateration
  
  Serial.println("TDOA: Trilateration not implemented yet (stub)");
  
  return pos;
}

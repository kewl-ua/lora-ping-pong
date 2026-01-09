#ifndef PACKET_H
#define PACKET_H

#include <Arduino.h>

// ===== Packet Structure for TDOA Navigation =====
// Формат пакета: EUID:<id>,MSG:<message>,TIME:<micros>,SEQ:<seq>

struct PacketData {
  String euid;          // Уникальный ID пакета (для корреляции на RX)
  String message;       // Полезная нагрузка
  uint32_t txTime_us;   // Время отправки (микросекунды)
  uint32_t sequence;    // Порядковый номер
  bool valid;           // Флаг успешного парсинга
  
  PacketData() : txTime_us(0), sequence(0), valid(false) {}
};

// Структура для статистики приема на RX
struct RxStats {
  uint32_t rxTime_us;   // Время приема (микросекунды)
  int32_t latency_us;   // Задержка TX->RX (микросекунды)
  int rssi;             // RSSI (dBm) - пока заглушка
  int snr;              // SNR (dB) - пока заглушка
  
  RxStats() : rxTime_us(0), latency_us(0), rssi(-100), snr(0) {}
};

// ===== Функции работы с пакетами =====

// Генерация уникального EUID
String generateEUID();

// Формирование пакета для отправки
String buildPacket(const String& message, uint32_t sequence);

// Парсинг принятого пакета
PacketData parsePacket(const String& rawData);

// Вычисление статистики приема
RxStats calculateRxStats(const PacketData& packet, uint32_t rxTime_us);

#endif // PACKET_H

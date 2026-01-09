#include "packet.h"

static uint32_t packetCounter = 0;

String generateEUID() {
  // Формат: COUNTER_MICROS (например: 123_4567890)
  uint32_t us = micros();
  return String(packetCounter++) + "_" + String(us);
}

String buildPacket(const String& message, uint32_t sequence) {
  String euid = generateEUID();
  uint32_t timestamp_us = micros();
  
  // Формат: EUID:<id>,MSG:<message>,TIME:<us>,SEQ:<seq>
  String packet = "EUID:" + euid + 
                  ",MSG:" + message + 
                  ",TIME:" + String(timestamp_us) +
                  ",SEQ:" + String(sequence);
  
  return packet;
}

PacketData parsePacket(const String& rawData) {
  PacketData data;
  data.valid = false;
  
  int euidStart = rawData.indexOf("EUID:");
  int msgStart = rawData.indexOf(",MSG:");
  int timeStart = rawData.indexOf(",TIME:");
  int seqStart = rawData.indexOf(",SEQ:");
  
  if (euidStart != -1 && msgStart != -1 && timeStart != -1 && seqStart != -1) {
    data.euid = rawData.substring(euidStart + 5, msgStart);
    data.message = rawData.substring(msgStart + 5, timeStart);
    
    String timeStr = rawData.substring(timeStart + 6, seqStart);
    String seqStr = rawData.substring(seqStart + 5);
    
    data.txTime_us = timeStr.toInt();
    data.sequence = seqStr.toInt();
    data.valid = true;
  }
  
  return data;
}

RxStats calculateRxStats(const PacketData& packet, uint32_t rxTime_us) {
  RxStats stats;
  stats.rxTime_us = rxTime_us;
  
  if (packet.valid) {
    // Вычисляем задержку в микросекундах
    stats.latency_us = (int32_t)(rxTime_us - packet.txTime_us);
    
    // RSSI и SNR пока заглушки (требуют AT команд или fixed mode)
    stats.rssi = -100;
    stats.snr = 0;
  }
  
  return stats;
}

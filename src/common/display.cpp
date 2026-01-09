#include "display.h"

DisplayManager displayManager;

#ifdef PLATFORM_ESP32

DisplayManager::DisplayManager() {
  display = new Adafruit_SSD1306(Config::Display::OLED_WIDTH, 
                                  Config::Display::OLED_HEIGHT, 
                                  &Wire, -1);
  isEnabled = Config::Display::OLED_ENABLED;
}

bool DisplayManager::initialize() {
  if (!isEnabled) {
    return true; // Дисплей отключен
  }
  
  // Инициализация I2C с нестандартными пинами
  Wire.begin(Config::Pins::OLED_SDA, Config::Pins::OLED_SCL);
  
  // Инициализация дисплея
  if (!display->begin(SSD1306_SWITCHCAPVCC, Config::Display::OLED_ADDRESS)) {
    Serial.println("ERROR: SSD1306 allocation failed");
    isEnabled = false;
    return false;
  }
  
  Serial.print("OLED: Initialized at 0x");
  Serial.print(Config::Display::OLED_ADDRESS, HEX);
  Serial.print(" (SDA=GPIO");
  Serial.print(Config::Pins::OLED_SDA);
  Serial.print(", SCL=GPIO");
  Serial.print(Config::Pins::OLED_SCL);
  Serial.println(")");
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println("  LoRa TDOA System");
  display->println("  ================");
  display->println();
  display->println("  Initializing...");
  display->display();
  
  return true;
}

void DisplayManager::showInitScreen(const String& mode) {
  if (!isEnabled) return;
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setCursor(0, 0);
  
  display->println("===== LoRa TDOA =====");
  display->println();
  display->print("Mode: ");
  display->println(mode);
  display->println();
  display->println("Platform: ESP32 v1302");
  display->print("LoRa: GPIO");
  display->print(Config::Pins::UART_RX);
  display->print("/");
  display->println(Config::Pins::UART_TX);
  display->print("AUX:  GPIO");
  display->println(Config::Pins::E32_AUX);
  display->print("LED:  GPIO");
  display->println(Config::Pins::LED);
  
  display->display();
}

void DisplayManager::showError(const String& error) {
  if (!isEnabled) return;
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setCursor(0, 0);
  
  display->println("===== ERROR =====");
  display->println();
  display->println(error);
  display->println();
  display->println("Check connections");
  display->println("and reset device.");
  
  display->display();
}

void DisplayManager::showTxStatus(uint32_t sequence, const String& message, bool success) {
  if (!isEnabled) return;
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setCursor(0, 0);
  
  // Заголовок
  display->println("==== TX MODE ====");
  display->println();
  
  // Статус отправки
  display->print("Status: ");
  if (success) {
    display->println("OK");
  } else {
    display->println("FAIL");
  }
  
  // Счетчик пакетов
  display->print("Packets: ");
  display->println(sequence);
  
  // Текущее сообщение (обрезаем если длинное)
  display->println();
  display->print("MSG: ");
  String truncMsg = truncateString(message, 16);
  display->println(truncMsg);
  
  // Время работы
  display->println();
  display->print("Uptime: ");
  display->print(millis() / 1000);
  display->println("s");
  
  display->display();
}

void DisplayManager::showRxStatus(const PacketData& packet, const RxStats& stats) {
  if (!isEnabled) return;
  
  display->clearDisplay();
  display->setTextSize(1);
  display->setCursor(0, 0);
  
  // Заголовок
  display->println("==== RX MODE ====");
  display->println();
  
  // EUID (обрезаем)
  display->print("EUID: ");
  String truncEUID = truncateString(packet.euid, 12);
  display->println(truncEUID);
  
  // Номер пакета
  display->print("SEQ:  ");
  display->println(packet.sequence);
  
  // Сообщение (обрезаем)
  display->print("MSG:  ");
  String truncMsg = truncateString(packet.message, 12);
  display->println(truncMsg);
  
  // Задержка
  display->println();
  display->print("LAT: ");
  if (stats.latency_us >= 0) {
    if (stats.latency_us < 1000) {
      display->print(stats.latency_us);
      display->println(" us");
    } else {
      display->print(stats.latency_us / 1000.0, 2);
      display->println(" ms");
    }
  } else {
    display->println("N/A");
  }
  
  // RSSI и SNR
  display->print("RSSI: ");
  display->print(stats.rssi);
  display->print(" SNR: ");
  display->println(stats.snr);
  
  display->display();
}

void DisplayManager::clear() {
  if (!isEnabled) return;
  display->clearDisplay();
  display->display();
}

String DisplayManager::formatTime(uint32_t micros) {
  if (micros < 1000) {
    return String(micros) + "us";
  } else if (micros < 1000000) {
    return String(micros / 1000.0, 2) + "ms";
  } else {
    return String(micros / 1000000.0, 2) + "s";
  }
}

String DisplayManager::truncateString(const String& str, uint8_t maxLen) {
  if (str.length() <= maxLen) {
    return str;
  }
  return str.substring(0, maxLen - 2) + "..";
}

#else
// Заглушки для платформ без дисплея (Arduino Mega)
DisplayManager::DisplayManager() {}
bool DisplayManager::initialize() { return true; }
void DisplayManager::showTxStatus(uint32_t sequence, const String& message, bool success) {}
void DisplayManager::showRxStatus(const PacketData& packet, const RxStats& stats) {}
void DisplayManager::showInitScreen(const String& mode) {}
void DisplayManager::showError(const String& error) {}
void DisplayManager::clear() {}
#endif

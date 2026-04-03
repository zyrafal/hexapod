#include "power_manager.h"

void PowerManager::begin()
{
    if (_ina.begin())
    {
        _ina.setMaxCurrentShunt(60, 0.0005);
        _ina.setAverage(INA228_64_SAMPLES); // Options: 1, 4, 16, 64, 128, 256, 512, 1024
        _ina.setAccumulation(1);            // Reset mAh
    }
    LittleFS.begin(true);
}

void PowerManager::update(int8_t rssi, int lq)
{
    if (millis() - _lastLogTime >= LOG_INTERVAL)
    {
        PowerStats currentStats = read();
        logToFile(currentStats, rssi, lq);
        _lastLogTime = millis();
    }
}

PowerStats PowerManager::read()
{
    PowerStats stats;
    stats.voltage = _ina.getBusVoltage();
    stats.current = _ina.getAmpere();
    stats.power = _ina.getPower();
    stats.mah = _ina.getCharge() / 3.6;
    stats.avgCell = (stats.voltage > 1.0f) ? (stats.voltage / 6.0f) : 0.0f;

    return stats;
}

void PowerManager::logToFile(PowerStats stats, int8_t rssi, int lq)
{
    File file = LittleFS.open("/log.txt", FILE_APPEND);
    if (file)
    {
        file.printf("%.1fV | %.1fV/c\n", stats.voltage, stats.avgCell);
        file.printf("%.1fA | %.1fW\n", stats.current, stats.power);
        file.printf("%ddBm | %d:100\n", (int)rssi, lq);
        file.printf("%s | %.0f mAh\n", getTimestamp().c_str(), stats.mah);
        file.println("-----------------------");
        file.close();
    }
}

void PowerManager::dumpLog()
{
    File file = LittleFS.open("/log.txt", FILE_READ);
    if (file)
    {
        while (file.available())
            Serial.write(file.read());
        file.close();
    }
}

void PowerManager::wipeLog() { LittleFS.remove("/log.txt"); }

String PowerManager::getTimestamp()
{
    long seconds = millis() / 1000;
    char buffer[25];
    // Formats: [HH:MM:SS.mmm]
    sprintf(buffer, "[%02d:%02d:%02d.%03d]",
            (int)(seconds / 3600),
            (int)((seconds % 3600) / 60),
            (int)(seconds % 60),
            (int)(millis() % 1000));
    return String(buffer);
}

#pragma once
#include <INA228.h>
#include <LittleFS.h>
#include <config.h>

struct PowerStats
{
    float voltage;
    float current;
    float power;
    float mah;
    float avgCell;
};

class PowerManager
{
public:
    void begin();
    void update(int8_t rssi, int lq);

    PowerStats read();

    void logToFile(PowerStats stats, int8_t rssi, int lq);
    void dumpLog();
    void wipeLog();

private:
    unsigned long _lastLogTime = 0;
    INA228 _ina = INA228(ADDR_POWER);
    String getTimestamp();
};

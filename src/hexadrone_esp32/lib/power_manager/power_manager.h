#pragma once
#include <INA228.h>
#include <config.h>
#include <logger.h>

enum class BatteryState
{
    NORMAL,
    WARNING,
    SOFT_CUTOFF,
    HARD_CUTOFF
};

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
    BatteryState update(int8_t rssi, int lq);

private:
    unsigned long _lastLogTime = 0;
    INA228 _ina = INA228(ADDR_POWER);

    // Battery Cutoff Trackers
    unsigned long _warningStartTime = 0;
    unsigned long _softStartTime = 0;
    bool _isWarning = false;
    bool _isSoft = false;
    bool _warningLogged = false;

    PowerStats read();
    void logPowerStats(PowerStats stats, int8_t rssi, int lq);
    BatteryState evaluateHealth(float voltage);
};

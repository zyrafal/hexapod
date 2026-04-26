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
    INA228 _ina = INA228(ADDR_POWER);

    // Battery Cutoff Trackers
    unsigned long _warningStartTime = 0;
    unsigned long _softStartTime = 0;
    bool _isWarning = false;
    bool _isSoft = false;
    bool _warningLogged = false;

    // State trackers for delta logging
    float _lastLoggedV = 0.0f;
    float _lastLoggedI = 0.0f;
    uint32_t _lastLoggedMah = 0;

    PowerStats read();
    // void logPowerStats(PowerStats stats); TODO: use for the display output, implement the function there or somewhere else
    BatteryState evaluateHealth(float voltage);
};

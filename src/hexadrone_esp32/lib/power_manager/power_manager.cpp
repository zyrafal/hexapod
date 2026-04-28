#include "power_manager.h"
#include "radio_manager.h"

// Access the global radio object defined in main.cpp
extern RadioManager radio;

void PowerManager::begin()
{
    if (_ina.begin())
    {
        _ina.setMaxCurrentShunt(60, 0.0005); // Configured for 60A/0.5mOhm
        _ina.setAverage(INA228_64_SAMPLES);  // Options: 1, 4, 16, 64, 128, 256, 512, 1024
        _ina.setAccumulation(1);             // Reset mAh

        Blackbox.logSystem("[POWER] INA228 Initialized.");
    }
}

BatteryState PowerManager::update(int8_t rssi, int lq)
{
    PowerStats stats = read();

    // 1. Delta Evaluation
    bool shouldLog = false;

    if (abs(stats.voltage - _lastLoggedV) >= LOG_VOLTAGE_THRESH)
    {
        shouldLog = true;
    }
    else if (abs(stats.current - _lastLoggedI) >= LOG_CURRENT_THRESH)
    {
        shouldLog = true;
    }
    else if ((stats.mah - _lastLoggedMah) >= 1.0f)
    {
        shouldLog = true;
    }

    // 2. Write if triggered
    if (shouldLog)
    {
        Blackbox.logPower(stats.avgCell, stats.voltage, stats.current, stats.power, stats.mah);

        _lastLoggedV = stats.voltage;
        _lastLoggedI = stats.current;
        _lastLoggedMah = stats.mah;
    }

    // 3. Telemetry Downlink (Send to Radio at 5Hz)
    static unsigned long _lastTelemetryTime = 0;
    if (millis() - _lastTelemetryTime >= 200)
    {
        _lastTelemetryTime = millis();

        // Calculate remaining battery percentage
        float clampedV = stats.voltage;
        if (clampedV > 24.6f)
            clampedV = 24.6f;
        if (clampedV < 20.4f)
            clampedV = 20.4f;
        uint8_t remaining_percent = (uint8_t)(((clampedV - 20.4f) / 5.2f) * 100.0f);

        radio.sendBatteryTelemetry(stats.voltage, stats.current, stats.mah, remaining_percent);
    }

    return evaluateHealth(stats.voltage);
}

PowerStats PowerManager::read()
{
    PowerStats stats;
    stats.voltage = _ina.getBusVoltage();
    stats.current = _ina.getAmpere();
    stats.power = _ina.getPower();
    stats.mah = _ina.getCharge() / 3.6; // Coulombs to mAh
    stats.avgCell = (stats.voltage > 1.0f) ? (stats.voltage / 6.0f) : 0.0f;

    return stats;
}

/* void PowerManager::logPowerStats(PowerStats stats, int8_t rssi, int lq)
{
    Blackbox.printf(
        "%s\n%.1fV | %.1fV/c\n%.1fA | %.1fW\n%.0f mAh\n%ddBm | %d:100\n-----------------------\n",
        Blackbox.getTimestamp().c_str(),
        stats.voltage, stats.avgCell,
        stats.current, stats.power,
        stats.mah,
        (int)rssi, lq);
} */
// TODO: use for the display output, implement the function there or somewhere else

BatteryState PowerManager::evaluateHealth(float voltage)
{
    // 1. USB/No-Battery Gate: Ignore logic if voltage is below 6V.
    if (voltage < 6.0f)
    {
        _isSoft = false;
        _isWarning = false;
        _warningLogged = false;
        return BatteryState::NORMAL;
    }

    // 2. Immediate Hard Cutoff
    if (voltage < HARD_CUTOFF_VOLTAGE)
        return BatteryState::HARD_CUTOFF;

    // 3. Soft Cutoff (Delayed)
    if (voltage < SOFT_CUTOFF_VOLTAGE)
    {
        if (!_isSoft)
        {
            _isSoft = true;
            _softStartTime = millis();
        }
        if (millis() - _softStartTime >= SOFT_CUTOFF_INTERVAL)
            return BatteryState::SOFT_CUTOFF;
    }
    else
    {
        _isSoft = false;
    }

    // 4. Warning (Delayed)
    if (voltage < WARNING_VOLTAGE)
    {
        if (!_isWarning)
        {
            _isWarning = true;
            _warningStartTime = millis();
        }
        if (millis() - _warningStartTime >= WARNING_INTERVAL)
        {
            if (!_warningLogged)
            {
                Blackbox.logSystem("[POWER] Low Battery! Sagging below %.1fV (%.2fV)", WARNING_VOLTAGE, voltage);
                _warningLogged = true;
            }
            return BatteryState::WARNING;
        }
    }
    else
    {
        _isWarning = false;
        _warningLogged = false;
    }

    return BatteryState::NORMAL;
}

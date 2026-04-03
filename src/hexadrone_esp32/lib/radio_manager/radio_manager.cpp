#include "radio_manager.h"

void RadioManager::begin()
{
    Serial2.begin(RADIO_BAUD, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN);
    _crsf.begin(Serial2);
}

void RadioManager::update()
{
    _crsf.update();

    bool currentlyConnected = isConnected();

    if (currentlyConnected && !_wasConnected)
    {
        Serial.println("[RADIO] Connection Acquired.");
        _wasConnected = true;
    }
    else if (!currentlyConnected && _wasConnected)
    {
        Serial.println("[RADIO] Connection Lost.");
        _wasConnected = false;
    }
}

int RadioManager::getChannel(int ch) { return _crsf.getChannel(ch); }

bool RadioManager::isArmed() { return _crsf.getChannel(5) > 1500; }

bool RadioManager::isKillTriggered()
{
    if (_killLatched)
        return true;

    int killSwitchValue = _crsf.getChannel(9);

    if (killSwitchValue > 1500)
    {
        if (!_isKillTiming)
        {
            _isKillTiming = true;
            _killTimerStart = millis();
        }
        else if (millis() - _killTimerStart >= KILL_SWITCH_INTERVAL)
        {
            _killLatched = true;
        }
    }
    else
    {
        _isKillTiming = false;
    }

    return _killLatched;
}

bool RadioManager::isConnected()
{
    return _crsf.isLinkUp();
}

int8_t RadioManager::getRSSI()
{
    auto stats = _crsf.getLinkStatistics();
    return stats ? (int8_t)stats->uplink_RSSI_1 : -128; // -128 = "Dead"
}

int RadioManager::getLQ()
{
    return _crsf.getLinkStatistics() ? _crsf.getLinkStatistics()->uplink_Link_quality : 0;
}

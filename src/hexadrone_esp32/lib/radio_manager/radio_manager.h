#pragma once
#include <AlfredoCRSF.h>
#include <config.h>

class RadioManager
{
public:
    void begin();
    void update();

    int getChannel(int ch);
    bool isArmed();
    bool isKillTriggered();
    bool isConnected();

    int8_t getRSSI();
    int getLQ();

private:
    AlfredoCRSF _crsf;

    unsigned long _killTimerStart = 0;
    bool _isKillTiming = false;
    bool _killLatched = false;
    bool _wasConnected = false;
};

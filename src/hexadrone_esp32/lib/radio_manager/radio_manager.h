#pragma once
#include <AlfredoCRSF.h>
#include <config.h>
#include <logger.h>
#include <hexadrone_core/state_machine.hpp>

class RadioManager
{
public:
    void begin();
    void update();
    void end();

    int getChannel(int ch);
    bool isConnected();
    int8_t getRSSI();
    int getLQ();

    // Public builder method
    Hexadrone::ControllerInput buildInput();

private:
    AlfredoCRSF _crsf;

    bool _wasConnected = false;

    // Private helpers
    float normalizeStick(int raw);
    int normalize3Pos(int raw);
};

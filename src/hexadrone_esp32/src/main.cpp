#include <Arduino.h>
#include <Wire.h>
#include <comms_manager.h>
#include <radio_manager.h>
#include <servo_manager.h>
#include <power_manager.h>
#include <hexadrone_core/brain.hpp>
#include <config.h>

CommsManager comms;
RadioManager radio;
ServoManager servo;
PowerManager power;
Hexadrone::Brain brain;

// --- Radio translation ---

// Normalize a raw CRSF channel value [988, 2012] to [-1.0, 1.0]
static float normalizeStick(int raw)
{
    return (raw - 1500) / 512.0f;
}

// Normalize a 3-position switch channel to -1 / 0 / +1
static int normalize3Pos(int raw)
{
    if (raw > 1700) return 1;
    if (raw < 1300) return -1;
    return 0;
}

// Build a ControllerInput from the current CRSF channel values.
// Channel numbering matches the RadioMaster CRSF layout in brain.cpp.
static Hexadrone::ControllerInput buildInput()
{
    Hexadrone::ControllerInput ci{};

    ci.roll    = normalizeStick(radio.getChannel(1));   // CH1
    ci.pitch   = normalizeStick(radio.getChannel(2));   // CH2
    ci.yaw     = normalizeStick(radio.getChannel(4));   // CH4

    // Velocity: stick range [-1, 1] → [0, 1] (matches BridgeOps convention)
    float stick_v  = normalizeStick(radio.getChannel(3)); // CH3
    ci.velocity    = (stick_v + 1.0f) * 0.5f;

    ci.armed_switch   = radio.getChannel(5) > 1500 ? 1 : -1;    // CH5  SA
    ci.posture_switch = normalize3Pos(radio.getChannel(7));      // CH7  SB
    ci.gear_switch    = normalize3Pos(radio.getChannel(8));      // CH8  SC
    ci.oe_kill_button = radio.getChannel(9) > 1500;              // CH9  SE

    ci.trim_coxa  = normalizeStick(radio.getChannel(11));        // CH11
    ci.trim_femur = normalizeStick(radio.getChannel(12));        // CH12
    ci.trim_tibia = normalizeStick(radio.getChannel(13));        // CH13
    ci.leg_selector = (int)((normalizeStick(radio.getChannel(14)) + 1.0f) * 3.0f + 1.0f); // CH14 → 1-6

    return ci;
}

// --- Timing ---

static unsigned long _lastLoopTime = 0;

static float getDt()
{
    unsigned long now = millis();
    float dt = (now - _lastLoopTime) / 1000.0f;
    _lastLoopTime = now;
    return dt;
}

// --- Setup / Loop ---

void setup()
{
    Serial.begin(DEBUG_BAUD);
    Wire.begin(SDA_CUSTOM, SCL_CUSTOM);

    power.begin();
    comms.begin(WIFI_SSID, WIFI_PASS);
    radio.begin();
    servo.begin();

    _lastLoopTime = millis();

    Serial.println("Hexadrone Initialized.");
}

void loop()
{
    comms.update();
    radio.update();
    power.update(radio.getRSSI(), radio.getLQ());

    // Hardware kill: disable PWM output immediately and stop processing
    if (radio.isKillTriggered())
    {
        servo.rapidKill();
        return;
    }

    float dt = getDt();
    Hexadrone::ControllerInput input = buildInput();
    std::vector<float> angles = brain.update(dt, input);
    servo.applyAngles(angles);

    // Debug commands over Serial
    if (Serial.available())
    {
        char c = Serial.read();
        if (c == 'd') power.dumpLog();
        if (c == 'w') power.wipeLog();
    }
}

#include <Arduino.h>
#include <Wire.h>
#include <config.h>
#include <logger.h>
#include <comms_manager.h>
#include <power_manager.h>
#include <radio_manager.h>
#include <servo_manager.h>
#include <hexadrone_core/brain.hpp>

CommsManager comms;
PowerManager power;
RadioManager radio;
ServoManager servo;
Hexadrone::Brain brain;

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
    Serial.setTimeout(10); // Set to 10ms so it doesn't freeze the loop
    Wire.begin(SDA_CUSTOM, SCL_CUSTOM);

    Blackbox.begin();

    power.begin(); // INA228 has to be initialized first
    comms.begin(WIFI_SSID, WIFI_PASS);
    radio.begin();
    servo.begin();

    _lastLoopTime = millis();

    Blackbox.println("[SYSTEM] Hexadrone Initialized.");
}

void loop()
{
    // 1. DATA ACQUISITION: Get the latest radio packets and timing
    radio.update();
    float dt = getDt();
    if (dt > 0.025f)
    { // If a loop takes longer than 25ms
        Blackbox.printf("[SYSTEM] Loop Time Spike detected: %.3f sec\n", dt);
    }
    Hexadrone::ControllerInput input = radio.buildInput();

    // 2. FLY-BY-WIRE INTERCEPT: Battery Soft Cutoff
    static bool soft_cutoff_latched = false;
    if (soft_cutoff_latched)
    {
        // Force the Brain to think the physical switch is disarmed
        input.armed_switch = -1;
    }

    // 3. PROCESSING: Let the Brain decide the new state and angles
    std::vector<float> angles = brain.update(dt, input);
    Hexadrone::DroneState current_state = brain.getState();

    // 4. BATTERY EVALUATION: Handle cuttoffs
    BatteryState batt_state = power.update(radio.getRSSI(), radio.getLQ());

    static bool hard_cutoff_triggered = false;
    if (batt_state == BatteryState::HARD_CUTOFF && !hard_cutoff_triggered)
    {
        Blackbox.println("[POWER] Hard Cutoff! Pulling power instantly.");
        hard_cutoff_triggered = true;
    }
    else if (batt_state == BatteryState::SOFT_CUTOFF && !soft_cutoff_latched)
    {
        Blackbox.println("[POWER] Soft Cutoff! Forcing Disarm sequence.");
        soft_cutoff_latched = true;
    }

    // 5. SUBSYSTEM UPDATE: React to the new state
    comms.update(current_state);

    // 6. SAFETY GATE: Check the NEW state immediately
    if (current_state == Hexadrone::DroneState::DRONE_OE_KILLED || hard_cutoff_triggered)
    {
        servo.rapidKill();
        return;
    }

    // 7. ACTUATION: Send angles to hardware
    servo.update(current_state);
    servo.applyAngles(angles);

    // 8. DEBUG COMMANDS
    if (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd == "xxdumplogxx")
            Blackbox.dumpLog();
        else if (cmd == "xxwipelogxx")
            Blackbox.wipeLog();
        else if (cmd == "flush")
            Blackbox.flush();
    }
}

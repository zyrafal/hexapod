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
    Wire.setClock(400000); // 400kHz I2C speed
    Wire.setTimeOut(10);   // Set timeout to 10ms (default is much higher)

    Blackbox.begin();

    power.begin(); // INA228 has to be initialized first
    comms.begin(WIFI_SSID, WIFI_PASS);
    radio.begin();
    servo.begin();

    _lastLoopTime = millis();

    Blackbox.logSystem("[SYSTEM] Hexadrone Initialized.");
}

void loop()
{
    static Hexadrone::DroneState lastNormalState = Hexadrone::DroneState::DRONE_DISARMED;
    static bool radio_link_active = false;
    static bool arm_switch_released_since_link = false;
    static bool soft_cutoff_latched = false;
    static bool hard_cutoff_triggered = false;

    float dt = getDt();
    if (dt > 0.025f)
    { // If a loop takes longer than 25ms
        Blackbox.logSystem("[SYSTEM] Loop Time Spike detected: %.3f sec", dt);
    }

    bool otaLockdown = comms.isOTALockdown();
    if (otaLockdown)
    {
        // OTA mode: freeze legs, ignore radio, skip I2C sensors, keep WiFi alive
        Hexadrone::DroneState current_state = lastNormalState;
        comms.update(current_state);

        // Skip all other subsystems while OTA upload is in progress.
    }
    else
    {
        // 1. DATA ACQUISITION: Get the latest radio packets and timing
        radio.update();
        Hexadrone::ControllerInput input = radio.buildInput();

        // 2. FLY-BY-WIRE INTERCEPT: Radio connection and battery soft cutoff
        if (!radio.isConnected())
        {
            // If no radio link is present, always disarm the drone
            input.armed_switch = -1;
            radio_link_active = false;
            arm_switch_released_since_link = false;
        }
        else
        {
            // On first connect, require the ARM switch to be moved to DISARM before allowing ARMED state
            if (!radio_link_active)
            {
                radio_link_active = true;
                arm_switch_released_since_link = false;
            }

            if (input.armed_switch == -1)
            {
                arm_switch_released_since_link = true;
            }

            if (!arm_switch_released_since_link)
            {
                input.armed_switch = -1;
            }
        }

        if (soft_cutoff_latched)
        {
            // Force the Brain to think the physical switch is disarmed
            input.armed_switch = -1;
        }

        // 3. PROCESSING: Let the Brain decide the new state and angles
        std::vector<float> angles = brain.update(dt, input);
        Hexadrone::DroneState current_state = brain.getState();
        lastNormalState = current_state;

        // 4. BATTERY EVALUATION: Handle cuttoffs
        BatteryState batt_state = power.update(radio.getRSSI(), radio.getLQ());
        if (batt_state == BatteryState::HARD_CUTOFF && !hard_cutoff_triggered)
        {
            Blackbox.logSystem("[POWER] Hard Cutoff! Pulling power instantly.");
            hard_cutoff_triggered = true;
        }
        else if (batt_state == BatteryState::SOFT_CUTOFF && !soft_cutoff_latched)
        {
            Blackbox.logSystem("[POWER] Soft Cutoff! Forcing Disarm sequence.");
            soft_cutoff_latched = true;
        }

        // 5. SUBSYSTEM UPDATE: React to the new state
        comms.update(current_state);

        // 6. SAFETY GATE: Check the NEW state immediately
        if (current_state == Hexadrone::DroneState::DRONE_OE_KILLED || hard_cutoff_triggered)
        {
            servo.rapidKill();
        }
        else
        {
            // 7. ACTUATION: Send angles to hardware ONLY if safe and ARMED (or Prone)
            servo.update(current_state);

            // The gate now allows angles if ARMED *OR* if it is currently lowering to the ground
            // Set posture state for servo manager
            servo.setPostureState(brain.getPostureState());
            if (current_state == Hexadrone::DroneState::DRONE_ARMED || servo.isDisarming())
            {
                servo.applyAngles(angles);
            }
            else
            {
                // When fully disarmed and the timer has expired, this safely blocks
                // applyAngles so servo.update() can turn the chips off sequentially!
            }
        }
    }

    // 8. DEBUG COMMANDS
    if (Serial.available())
    {
        static String inputBuffer = ""; // Static keeps the memory alive between loops
        char c = Serial.read();

        // If it's a newline or carriage return, we are done typing
        if (c == '\n' || c == '\r')
        {
            if (inputBuffer.length() > 0)
            {
                inputBuffer.trim();

                if (inputBuffer == "xxwipelogxx")
                    Blackbox.wipeLog();
                else if (inputBuffer == "flush")
                {
                    Blackbox.flushSystem();
                    Blackbox.flushPower();
                }
                else if (inputBuffer == "dump")
                    Blackbox.dumpLog();

                inputBuffer = ""; // Clear the buffer for the next command
            }
        }
        else
        {
            // Add the letter to the buffer
            inputBuffer += c;
        }
    }
}

//
// In main.cpp loop()
// static unsigned long last_servo_update = 0;
// unsigned long now = millis();

// 1. Math is always updated for maximum precision
// std::vector<float> angles = brain.update(dt, input);

// 2. Physical hardware is updated only at the PWM frequency
// if (now - last_servo_update >= 20) { // 20ms = 50Hz
//    last_servo_update = now;
//    servo.applyAngles(angles);
//}

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
        Blackbox.println("[RADIO] Connection Acquired.");
        _wasConnected = true;
    }
    else if (!currentlyConnected && _wasConnected)
    {
        Blackbox.println("[RADIO] Connection Lost.");
        _wasConnected = false;
    }
}

int RadioManager::getChannel(int ch) { return _crsf.getChannel(ch); }

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

// --- Radio translation ---

// Normalize a raw CRSF channel value [988, 2012] to [-1.0, 1.0]
float RadioManager::normalizeStick(int raw)
{
    return (raw - 1500) / 512.0f;
}

// Normalize a 3-position switch channel to -1 / 0 / +1
int RadioManager::normalize3Pos(int raw)
{
    if (raw > 1700)
        return 1;
    if (raw < 1300)
        return -1;
    return 0;
}

// Build a ControllerInput from the Radiomaster Pocket CRSF channel values.
// Channel numbering matches the layout in brain.cpp
Hexadrone::ControllerInput RadioManager::buildInput()
{
    Hexadrone::ControllerInput ci{};

    // 1. Primary Sticks [-1.0, 1.0]
    ci.roll = normalizeStick(getChannel(1));     // CH1 (Ail)
    ci.pitch = normalizeStick(getChannel(2));    // CH2 (Ele)
    ci.throttle = normalizeStick(getChannel(3)); // CH3 (Thr)
    ci.yaw = normalizeStick(getChannel(4));      // CH4 (Rud)

    // 2. State & Safety Switches
    ci.armed_switch = (getChannel(5) > 1500) ? 1 : -1; // CH5 (SA)
    ci.posture_switch = normalize3Pos(getChannel(6));  // CH6 (SB)
    ci.gear_switch = normalize3Pos(getChannel(7));     // CH7 (SC)
    ci.oe_kill_button = getChannel(9) > 1500;          // CH9 (SE)

    // 3. Manual Trims ([-1.0, 1.0] for fine-tuning)
    ci.trim_coxa = normalizeStick(getChannel(11));  // CH11
    ci.trim_femur = normalizeStick(getChannel(12)); // CH12
    ci.trim_tibia = normalizeStick(getChannel(13)); // CH13

    // 4. Leg Selector (Digital Trim Mapping)
    int raw_selector = getChannel(14);
    ci.btn_prev_leg = (raw_selector < 1300); // Left push
    ci.btn_next_leg = (raw_selector > 1700); // Right push

    return ci;
}

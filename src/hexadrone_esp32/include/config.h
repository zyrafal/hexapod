#pragma once

// WiFi Configuration
#define WIFI_SSID "norusko"
#define WIFI_PASS "azovsbs93"

// --- Hardware Mapping ---

// Maps physical PCA9685 wiring blocks (0-5) to Brain logical indices
// (LF=0, RM=1, LB=2, RF=3, LM=4, RB=5)
constexpr int LEG_REMAP[6] = {1, 4, 3, 0, 5, 2};

// I2C Pins
#define SDA_CUSTOM 25
#define SCL_CUSTOM 33
#define OE_CUSTOM 23

// Radio Pins
#define CRSF_RX_PIN 27
#define CRSF_TX_PIN 26

// I2C Addresses
#define ADDR_SERVO_1 0x41
#define ADDR_SERVO_2 0x70
#define ADDR_POWER 0x45

// --- Software mapping ---

// Logging & Blackbox Configuration
#define BLACKBOX_BUFFER_SIZE 1024 // RAM buffer before flushing (bytes)
#define BLACKBOX_MAX_FLUSHES 500
#define LOG_VOLTAGE_THRESH 0.2f
#define LOG_CURRENT_THRESH 0.5f

// Battery voltages (V) and intervals (ms)
#define HARD_CUTOFF_VOLTAGE 18.0f // 3.0V per cell
#define SOFT_CUTOFF_VOLTAGE 20.4f // 3.4V per cell
#define SOFT_CUTOFF_INTERVAL 5000
#define WARNING_VOLTAGE 21.6f // 3.6V per cell
#define WARNING_INTERVAL 2000

// Arming Intervals (ms)
#define SERVO_ARM_INTERVAL 100
#define DISARM_TRIGGER_INTERVAL 2000

// Baud rates
#define DEBUG_BAUD 115200
#define RADIO_BAUD 420000

// Servo PWM Ticks (Standard 0.5ms - 2.5ms range)
#define PWM_FREQ 60
#define SERVOMIN 120    // -90 degrees
#define SERVOMAX 610    // +90 degrees
#define INITIAL_ANGLE 0 // Neutral starting position

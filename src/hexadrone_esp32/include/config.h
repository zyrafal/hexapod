#pragma once

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

// WiFi Configuration
#define WIFI_SSID "norusko"
#define WIFI_PASS "azovsbs93"

// Servo PWM Ticks (Standard 0.5ms - 2.5ms range)
#define SERVOMIN 120    // -90 degrees
#define SERVOMAX 610    // +90 degrees
#define INITIAL_ANGLE 0 // Neutral starting position

#define TOTAL_SERVOS 18

// Intervals (ms)
#define LOG_INTERVAL 100
#define SERVO_ARM_INTERVAL 100
#define KILL_SWITCH_INTERVAL 1000

// Baud rates
#define DEBUG_BAUD 115200
#define RADIO_BAUD 420000

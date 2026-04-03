// pio run -t upload
// pio device monitor -p /dev/ttyUSB0 -b 115200

//
// ========== SERVO SETUP ==========
//

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// SDA a SCL piny
#define SDA_CUSTOM GPIO_NUM_25
#define SCL_CUSTOM GPIO_NUM_33

// Inicializace PWM driverů
Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x70);

// Servo min a max
#define SERVOMIN 100 
#define SERVOMAX 350

int angles3[32] = {
  180,180,180,180, 180,180,180,180,
  180,180,180,180, 180,180,180,180,
  180,180,180,180, 180,180,180,180,
  180,180,180,180, 180,180,180,180
};

//
// ========== RADIO SETUP ==========
// 

#include <AlfredoCRSF.h>
#include <HardwareSerial.h>

// --- PINS ---
// ESP32 Hardware Serial 2 (UART2)
// Connect Receiver RX -> GPIO 17
// Connect Receiver TX -> GPIO 16
#define CRSF_RX_PIN 27
#define CRSF_TX_PIN 26

// --- OBJECTS ---
HardwareSerial crsfSerial(2); // Use UART2
AlfredoCRSF crsf;

// =================================

#include "lib.hpp"

const float L0 = 68.0;
const float L1 = 86.634;
const float L2 = 113.551;

Vec3 LEG_OFFSETS[6] = {};
const float LEG_ORIENTATIONS[6] = {-135, -90, -45, 45, 90, 135};

const Vec3 DOWN = Vec3(0, -1, 0);

Vec3 leg_positions[6] = {};

Vec3 gait_pos[2] = {Vec3(), Vec3()};

Vec3 odd_pos = Vec3();
Vec3 even_pos = Vec3();
float odd_rot = 0;
float even_rot = 0;

float spread = 100;
float height = 100;

float front_tilt = 0;
float right_tilt = 0;

float step_height = 20;
float step_length = 50;
float step_angle = 30;

// milimeters per second
float max_speed = 50;

float phase = 0.0;

void posToAngles2D(float w, float h, float &a, float &b) {
  float r_sq = (w * w) + (h * h);
  a = (-acos(((L1 * L1) + r_sq - (L2 * L2)) / (2 * L1 * sqrt(r_sq))) - atan2(h, w)) * rad2deg;
  b = 180 - acos(((L1 * L1) + (L2 * L2) - r_sq) / (2 * L1 * L2)) * rad2deg;
}

void posToAngles3D(float x, float y, float z, float &a, float &b, float &c) {
  float w = sqrt(x * x + z * z);
  float h = y;

  if (w > 0)
    c = asin(-x / w) * rad2deg;
  else
    c = 0;

  posToAngles2D(w - L0, h, a, b);
}

Vec3 globalToLocal(int leg, Vec3 global) {
  return (global - LEG_OFFSETS[leg]).rotate(DOWN, LEG_ORIENTATIONS[leg]);
}

Vec3 tilt(Vec3 vec) {
  return vec.rotate(Vec3(-1, 0, 0), front_tilt).rotate(Vec3(0, 0, 1), right_tilt);
}

void gaitSetPositions() {
  for (int i = 0; i < 6; i += 2) {
    leg_positions[i] = globalToLocal(i, tilt(Vec3(0.0, -height, spread).rotate(DOWN, LEG_ORIENTATIONS[i] + even_rot)) + even_pos);
    leg_positions[i+1] = globalToLocal(i+1, tilt(Vec3(0.0, -height, spread).rotate(DOWN, LEG_ORIENTATIONS[i+1] + odd_rot)) + odd_pos);
  }
}

Vec3 phaseVec(float t) {
  if (t <= 0.5)
    return Vec3(step_length * (t - 0.5) / 0.5, step_height * t / 0.5, 0);
  else if (t > 0.5 && t <= 1.0)
    return Vec3(step_length * (t - 0.5) / 0.5, step_height * (1 - t) / 0.5, 0);
  else if (t > 1.0 && t <= 2.0)
    return Vec3(step_length * (1.5 - t) / 0.5, 0, 0);
  else
    return Vec3(step_length, 0, 0);
}

// Vec3 offsetByPhase(float phase, float orientation, Vec3 vec) {
//   return vec + phaseVec(phase).rotate(UP, -orientation * deg2rad);
// }

 void calculateWalkPos(float phase) {
   float even_phase = fmod(phase, 2.0);
   float odd_phase = fmod((phase + 1.0), 2.0);

   even_pos = phaseVec(even_phase);
   odd_pos = phaseVec(odd_phase);
 }

int angleToPulse(int ang) {
  if (ang==0) return 100;
  int pulse = map(ang, 0, 180, SERVOMIN, SERVOMAX);
  return pulse;
} 

void setServos(int serv_values[]) {
  for (int i = 0; i < 32; i++) {
    int angleBoard1 = serv_values[i];
    int angleBoard2 = serv_values[i + 16];
    board1.setPWM(i, 0, angleToPulse(angleBoard1));
  }
}

float chToPos(int ch) {
  float chh = ch;
  float ang = (chh - 1500) / (2012 - 988) * 2;
  return ang;
}

void setLegPositions() {
  // z = -ch1, y = ch2, x = ch3
  // ang0 = -b, ang4 = -a, ang8 = c
  for (int i = 0; i < 6; i++) {
    float a, b, c;
    Vec3 pos = leg_positions[i];
    posToAngles3D(pos.x, pos.y, pos.z, a, b, c);
    // angles3[4*leg] = 140 - 2*b;
    // angles3[4*leg+1] = 235 - 2*a;
    // angles3[4*leg+2] = 180 + 2*c;
    angles3[4*i] = 285 - 2*b;
    angles3[4*i+1] = 240 - 2*a;
    angles3[4*i+2] = 180 + 2*c;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Started");

  // Inicializace I2C na vlastních pinech
  Wire.begin(SDA_CUSTOM, SCL_CUSTOM);  // <-- Tohle je klíčové pro ESP32

  board1.begin();
  board2.begin();

  board1.setPWMFreq(60);
  board2.setPWMFreq(60);
  

  // 2. Start Receiver Serial (Must be 420,000 baud for CRSF)
  crsfSerial.begin(420000, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN);
  // 3. Initialize Protocol
  crsf.begin(crsfSerial);

  float right = 120.5;
  float righter = right + 22.8;
  float front = 135.0;

  LEG_OFFSETS[0] = Vec3(-right, 0, -front);
  LEG_OFFSETS[1] = Vec3(-righter, 0, 0);
  LEG_OFFSETS[2] = Vec3(-right, 0, front);
  LEG_OFFSETS[3] = Vec3(right, 0, -front);
  LEG_OFFSETS[4] = Vec3(righter, 0, 0);
  LEG_OFFSETS[5] = Vec3(right, 0, front);
}

void loop() {
  crsf.update();
  int ch1 = crsf.getChannel(1); // Roll / Aileron
  int ch2 = crsf.getChannel(2); // Pitch / Elevator
  int ch3 = crsf.getChannel(3); // Throttle
  int ch4 = crsf.getChannel(4); // Yaw / Rudder

  float forward = chToPos(ch3);
  if (abs(forward) < 0.05) forward = 0.0;
  
  phase += forward * max_speed * millis() / 1000 / step_length;

  calculateWalkPos(phase);
  gaitSetPositions();
  setLegPositions();

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    // float a, b, c;
    // posToAngles3D(x, y, z, a, b, c);
    // Serial.printf("x = %.3f, y = %.3f, z = %.3f\n", x, y, z);
    // Serial.printf("a = %.3f, b = %.3f, c = %.3f\n", a, b, c);
    
    if (ch3 > 0) {
      Serial.printf("Roll: %4d | Pitch: %4d | Thr: %4d | Yaw: %4d\n", ch1, ch2, ch3, ch4);
    } else {
      Serial.println("Waiting for Receiver connection...");
    }

    lastPrint = millis();
  }
}

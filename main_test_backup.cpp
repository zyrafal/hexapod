#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <AlfredoCRSF.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <INA228.h> 

// ========== CONFIGURATION ==========
#define SDA_CUSTOM 25
#define SCL_CUSTOM 33
#define CRSF_RX_PIN 27
#define CRSF_TX_PIN 26

#define SERVOMIN 100 
#define SERVOMAX 350
#define INITIAL_ANGLE 180

#define LOG_INTERVAL 500       
#define SERVO_STEP_INTERVAL 500 

// ========== GLOBAL OBJECTS ==========
Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x41); 
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x40); 

HardwareSerial crsfSerial(2); 
AlfredoCRSF crsf;

// Direct object, no longer a pointer!
INA228 pINA(0x45); 

// ========== STATE VARIABLES ==========
float totalMah = 0.0;
unsigned long lastLogTime = 0;
unsigned long lastIntegrationTime = 0;
unsigned long lastServoStepTime = 0;
int currentTestServo = 0; 
bool isArmed = false;

// ========== HELPER FUNCTIONS ==========

String getTimestamp() {
    long seconds = millis() / 1000;
    char buffer[25];
    sprintf(buffer, "[%02d:%02d:%02d.%03d]", 
            (int)(seconds / 3600), (int)((seconds % 3600) / 60), 
            (int)(seconds % 60), (int)(millis() % 1000));
    return String(buffer);
}

void dumpLog() {
    Serial.println("\n--- START OF STORED LOG ---");
    File file = LittleFS.open("/log.txt", FILE_READ);
    if (!file) { Serial.println("No log found."); return; }
    while (file.available()) Serial.write(file.read());
    file.close();
    Serial.println("\n--- END OF LOG ---");
}

void wipeLog() {
    if (LittleFS.remove("/log.txt")) Serial.println("!!! LOG DELETED !!!");
}

void logAutonomousData() {
    // 1. Read High-Precision Sensors
    float voltage = pINA.getBusVoltage();
    float current = pINA.getAmpere(); 
    float watts   = pINA.getPower();
    
    // 2. Use Hardware Charge Accumulation (Coulombs to mAh)
    // 1 mAh = 3.6 Coulombs
    float hardwareMah = pINA.getCharge() / 3.6; 

    // 3. Health & Radio Checks
    if (voltage > 55.0 || voltage < 0.0) voltage = 0.0; 
    float avgCell = (voltage > 5.0) ? (voltage / 6.0) : 0.0;

    int8_t rssi = 0;
    int lq = 0;
    if (crsf.getLinkStatistics()) {
        rssi = (int8_t)crsf.getLinkStatistics()->uplink_RSSI_1;
        lq = crsf.getLinkStatistics()->uplink_Link_quality;
    }

    // 4. Enhanced Display Log
    File file = LittleFS.open("/log.txt", FILE_APPEND);
    if (file) {
        file.printf("--- %s ---\n", getTimestamp().c_str());
        file.printf("%.1fV | %.2fV/c\n", voltage, avgCell);
        file.printf("%.2fA | %.1fW\n", current, watts);
        file.printf("%.0f mAh (HW Acc)\n", hardwareMah); // Now using hardware precision
        file.printf("%ddBm | %d:100\n", (int)rssi, lq);
        file.close();
    }
}

// ========== SETUP ==========

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_CUSTOM, SCL_CUSTOM);
    Wire.setClock(100000); 
    delay(1000);

    // Options: 1, 4, 16, 64, 128, 256, 512, 1024
    pINA.setAverage(INA228_64_SAMPLES);

    Serial.println("\n--- HARDWARE INITIALIZATION ---");
    
    // Using dot (.) operator here as well
    if (pINA.begin()) {
        pINA.setMaxCurrentShunt(60, 0.0005); 
        Serial.println("Holybro PM02D HV (INA228) Online at 0x45!");
    } else {
        Serial.println("Holybro PM02D HV Failed to initialize.");
    }

    board1.begin();
    board1.setPWMFreq(60);
    board2.begin();
    board2.setPWMFreq(60);
    
    if(!LittleFS.begin(true)) Serial.println("LittleFS Error");

    crsfSerial.begin(420000, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN);
    crsf.begin(crsfSerial);

    lastIntegrationTime = millis();
    Serial.println("System Ready. 'd'=dump, 'w'=wipe.");
}

// ========== LOOP ==========

void loop() {
    crsf.update();

    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'd') dumpLog();
        if (c == 'w') wipeLog();
    }

    int armValue = crsf.getChannel(5);
    if (armValue > 1500) {
        if (!isArmed) { Serial.println("!!! ARMED !!!"); isArmed = true; currentTestServo = 0; }
        
        if (millis() - lastServoStepTime > SERVO_STEP_INTERVAL && currentTestServo < 32) {
            int pulse = map(INITIAL_ANGLE, 0, 180, SERVOMIN, SERVOMAX);
            if (currentTestServo < 16) board1.setPWM(currentTestServo, 0, pulse);
            else board2.setPWM(currentTestServo - 16, 0, pulse);
            
            currentTestServo++;
            lastServoStepTime = millis();
        }
    } else {
        isArmed = false;
    }

    if (millis() - lastLogTime > LOG_INTERVAL) {
        logAutonomousData();
        lastLogTime = millis();
    }
}

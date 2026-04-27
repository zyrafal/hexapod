#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <cstdarg>
#include <config.h>

class Logger
{
public:
    void begin();

    void logSystem(const char *format, ...);
    void logPower(float avgCell, float voltage, float current, float power, uint32_t mah);

    void flushSystem();
    void flushPower();
    void wipeLog();
    void dumpLog();

private:
    void formatTimestamp(char *buffer, size_t len);

    void print(const String &msg);
    void println(const String &msg);
    void printf(const char *format, ...);

    bool _fsReady = false;

    String _fileBuffer;
    String _powerBuffer;
    const unsigned int MAX_BUFFER_SIZE = BLACKBOX_BUFFER_SIZE; // Write to flash every ~1KB

    unsigned int _flushCount = 0;
    unsigned int _powerFlushCount = 0;
    const unsigned int MAX_FLUSHES = BLACKBOX_MAX_FLUSHES; // Wipe/Rotate file after n writes
};

// Expose a global instance so all files can use it without passing references
extern Logger Blackbox;

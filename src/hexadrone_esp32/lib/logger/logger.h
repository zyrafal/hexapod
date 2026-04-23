#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <cstdarg>
#include <config.h>

class Logger
{
public:
    void begin();

    void printf(const char *format, ...);
    void println(const String &msg);
    void print(const String &msg);

    void flush();

    void dumpLog();
    void wipeLog();
    String getTimestamp();

private:
    bool _fsReady = false;

    String _fileBuffer;
    const unsigned int MAX_BUFFER_SIZE = BLACKBOX_BUFFER_SIZE; // Write to flash every ~1KB

    unsigned int _flushCount = 0;
    const unsigned int MAX_FLUSHES = BLACKBOX_MAX_FLUSHES; // Wipe/Rotate file after n writes
};

// Expose a global instance so all files can use it without passing references
extern Logger Blackbox;

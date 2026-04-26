#include "logger.h"

// Instantiate the global object
Logger Blackbox;

void Logger::begin()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("[LOGGER] LittleFS Mount Failed");
        _fsReady = false;
        return;
    }
    _fsReady = true;
    _fileBuffer.reserve(MAX_BUFFER_SIZE + 256);  // Pre-allocate to prevent heap fragmentation
    _powerBuffer.reserve(MAX_BUFFER_SIZE + 256); // Pre-allocate to prevent heap fragmentation

    if (!LittleFS.exists("/system.log"))
    {
        File f = LittleFS.open("/system.log", FILE_WRITE);
        if (f)
            f.close();
    }

    logSystem("[LOGGER] LittleFS Mounted. Blackbox active.");

    // Ensure CSV header exists
    File csv = LittleFS.open("/power.csv", FILE_READ);
    if (!csv || csv.size() == 0)
    {
        if (csv)
            csv.close();
        File newCsv = LittleFS.open("/power.csv", FILE_APPEND);
        if (newCsv)
        {
            newCsv.println("Timestamp,Voltage(V),AvgCell(V),Current(A),Power(W),Capacity(mAh)");
            newCsv.close();
        }
    }
    else
    {
        csv.close();
    }
}

void Logger::logSystem(const char *format, ...)
{
    char timeBuf[20];
    formatTimestamp(timeBuf, sizeof(timeBuf));

    char msgBuf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(msgBuf, sizeof(msgBuf), format, args);
    va_end(args);

    String finalMsg = String(timeBuf) + " " + String(msgBuf);

    Serial.println(finalMsg); // Live Serial Monitor
    println(finalMsg);        // Send to the RAM buffer
}

void Logger::logPower(float avgCell, float voltage, float current, float power, uint32_t mah)
{
    if (!_fsReady)
        return;

    char timeBuf[20];
    formatTimestamp(timeBuf, sizeof(timeBuf));

    char msgBuf[128];
    snprintf(msgBuf, sizeof(msgBuf), "%s,%.2f,%.2f,%.2f,%.2f,%lu\n", timeBuf, avgCell, voltage, current, power, mah);

    _powerBuffer += String(msgBuf);
    if (_powerBuffer.length() >= MAX_BUFFER_SIZE)
    {
        flushPower();
    }
}

void Logger::flushSystem()
{
    if (!_fsReady || _fileBuffer.length() == 0)
        return;

    // Rotate log if it hits max flushes
    if (_flushCount >= MAX_FLUSHES)
    {
        LittleFS.remove("/system.log");
        _flushCount = 0;
        _fileBuffer = "[LOGGER] --- System Log Rotated: Max file size reached ---\n" + _fileBuffer;
    }

    // Write the buffer to the file
    File file = LittleFS.open("/system.log", FILE_APPEND);
    if (file)
    {
        file.print(_fileBuffer);
        file.close();
        _fileBuffer.clear();
        _flushCount++;
    }
    else
    {
        Serial.println("[LOGGER] Failed to flush to system.log");
    }
}

void Logger::flushPower()
{
    if (!_fsReady || _powerBuffer.length() == 0)
        return;

    // Rotate CSV if it hits max flushes
    if (_powerFlushCount >= MAX_FLUSHES)
    {
        LittleFS.remove("/power.csv");
        _powerFlushCount = 0;
        _powerBuffer = "Timestamp,Voltage(V),AvgCell(V),Current(A),Power(W),Capacity(mAh)\n" + _powerBuffer;
    }

    // Write the buffer to the file
    File file = LittleFS.open("/power.csv", FILE_APPEND);
    if (file)
    {
        file.print(_powerBuffer);
        file.close();
        _powerBuffer.clear();
        _powerFlushCount++;
    }
    else
    {
        Serial.println("[LOGGER] Failed to flush to power.csv");
    }
}

void Logger::wipeLog()
{
    if (!_fsReady)
        return;

    LittleFS.remove("/system.log");
    LittleFS.remove("/power.csv");
    _fileBuffer.clear();
    _powerBuffer.clear();
    _flushCount = 0;
    _powerFlushCount = 0;

    // 1. Immediately recreate system.log with a header to prevent 0-byte errors
    File sysFile = LittleFS.open("/system.log", FILE_WRITE);
    if (sysFile)
    {
        sysFile.println("[LOGGER] --- Log Reset: All previous data purged ---");
        sysFile.close();
    }

    // 2. Recreate power.csv with the correct header
    File newCsv = LittleFS.open("/power.csv", FILE_WRITE);
    if (newCsv)
    {
        newCsv.println("Timestamp,Voltage(V),AvgCell(V),Current(A),Power(W),Capacity(mAh)");
        newCsv.close();
    }

    Serial.println("[LOGGER] Files wiped and headers recreated.");
}

void Logger::formatTimestamp(char *buffer, size_t len)
{
    uint32_t ms = millis();
    uint32_t s = ms / 1000;
    uint32_t m = s / 60;
    uint32_t h = m / 60;
    snprintf(buffer, len, "[%02lu:%02lu:%02lu.%03lu]", h, m % 60, s % 60, ms % 1000);
}

void Logger::print(const String &msg)
{
    _fileBuffer += msg;
    if (_fileBuffer.length() >= MAX_BUFFER_SIZE)
    {
        flushSystem();
    }
}

void Logger::println(const String &msg)
{
    print(msg + "\n");
}

void Logger::printf(const char *format, ...)
{
    char loc_buf[256];
    va_list arg;
    va_start(arg, format);
    vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(arg);
    print(String(loc_buf));
}

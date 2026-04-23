#include "logger.h"

// Instantiate the global object
Logger Blackbox;

void Logger::begin()
{
    _fileBuffer.reserve(MAX_BUFFER_SIZE + 256);

    if (LittleFS.begin(true))
    {
        _fsReady = true;
        printf("[LOGGER] LittleFS Mounted. Blackbox active.");
    }
    else
    {
        Serial.println("[LOGGER] LittleFS Mount Failed! (Serial only)");
    }
}

void Logger::printf(const char *format, ...)
{
    // 1. Format the string using standard C arguments
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 2. Output to Serial terminal
    Serial.print(buffer);

    // 3. Output to LittleFS Blackbox
    if (_fsReady)
    {
        _fileBuffer += buffer;
        if (_fileBuffer.length() >= MAX_BUFFER_SIZE)
        {
            flush();
        }
    }
}

void Logger::println(const String &msg)
{
    printf("%s\n", msg.c_str());
}

void Logger::print(const String &msg)
{
    printf("%s", msg.c_str());
}

void Logger::flush()
{
    if (!_fsReady || _fileBuffer.length() == 0)
        return;

    // 1. Handle Log Rotation Safely
    if (_flushCount >= MAX_FLUSHES)
    {
        LittleFS.remove("/blackbox.txt"); // Just delete the physical file
        _flushCount = 0;                  // Reset counter

        _fileBuffer = "[LOGGER] --- Log Rotated: Max file size reached ---\n" + _fileBuffer;
    }

    // 2. Write the buffer to the file
    File file = LittleFS.open("/blackbox.txt", FILE_APPEND);
    if (file)
    {
        file.print(_fileBuffer);
        file.close();

        _fileBuffer = "";
        _flushCount++;
    }
}

void Logger::dumpLog()
{
    if (!_fsReady)
        return;

    flush();

    File file = LittleFS.open("/blackbox.txt", FILE_READ);
    if (file)
    {
        Serial.println("\n--- BEGIN BLACKBOX DUMP ---");
        while (file.available())
            Serial.write(file.read());
        Serial.println("--- END BLACKBOX DUMP ---\n");
        file.close();
    }
}

void Logger::wipeLog()
{
    if (!_fsReady)
        return;

    LittleFS.remove("/blackbox.txt");
    println("[LOGGER] Blackbox file wiped.");
}

String Logger::getTimestamp()
{
    long seconds = millis() / 1000;
    char buffer[25];
    sprintf(buffer, "[%02d:%02d:%02d.%03d]",
            (int)(seconds / 3600), (int)((seconds % 3600) / 60),
            (int)(seconds % 60), (int)(millis() % 1000));
    return String(buffer);
}

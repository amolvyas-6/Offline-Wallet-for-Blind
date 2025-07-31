#include "GPS_FNS.h"

TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Using Serial1 for GPS

// AT command related definitions
const long TIMEOUT = 2000; // Timeout for AT command
String response;

void gpsInit()
{
    Serial.println("Initialising GPS...");
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); // Initialize GPS serial
    delay(1000);

    // Initialization of GPS Module
    sendATCommand("AT"); // Wake up GPS
    delay(1000);
    sendATCommand("AT+CGNSSPWR=1"); // Power on GNSS
    delay(10000);                   // Wait for the module to power up
    sendATCommand("AT+CGNSSTST=1"); // Start the GNSS test
    delay(500);
    sendATCommand("AT+CGNSSPORTSWITCH=0,1"); // Switch GNSS port
}

void sendATCommand(const char *cmd)
{
    Serial.print("Sending AT Command: ");
    Serial.println(cmd);
    gpsSerial.println(cmd); // Send command to the GPS module

    // Wait for response
    if (!waitForResponse("OK"))
    {
        Serial.println("No valid response.");
    }
}

bool waitForResponse(const char *expected)
{
    unsigned long start = millis();
    response = ""; // Clear previous response

    while (millis() - start < TIMEOUT)
    {
        while (gpsSerial.available())
        {
            response += (char)gpsSerial.read();

            // Check for expected response
            if (response.indexOf(expected) != -1)
            {
                Serial.println("Response received:");
                Serial.println(response);
                return true; // Found expected response
            }
        }
    }
    return false; // Timeout reached
}

bool getGPSData(TransactionData *t)
{
    sendATCommand("AT+CGNSSTST=1"); // Start the GNSS test
    delay(500);
    sendATCommand("AT+CGNSSPORTSWITCH=0,1");
    delay(500);

    while (gpsSerial.available())
    {
        gps.encode(gpsSerial.read()); // Read and pass data to TinyGPS++
        t->year = gps.date.year();
        t->month = gps.date.month();
        t->day = gps.date.day();
        t->hour = gps.time.hour();
        t->minute = gps.time.minute();
        t->second = gps.time.second();
    }

    if (gps.location.isUpdated())
    {
        t->latitude = gps.location.lat();
        t->longitude = gps.location.lng();
        return true;
    }
    return false;
}
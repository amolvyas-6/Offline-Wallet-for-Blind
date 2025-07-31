#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>
#include "NFC_FNS.h"

#define GPS_RX_PIN 17 // GPS RX pin connected to ESP32 TX
#define GPS_TX_PIN 18 // GPS TX pin connected to ESP32 RX
#define GPS_BAUD 115200

// Function Declarations
void sendATCommand(const char *cmd);
bool waitForResponse(const char *expected);
void gpsInit();
bool getGPSData(TransactionData *t);
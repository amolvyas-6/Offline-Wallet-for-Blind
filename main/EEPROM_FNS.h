#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "NFC_FNS.h"
#include "EncryptionUtils.h" // Include the encryption utility header

// EEPROM I2C Configuration
#define SDA_PIN 7
#define SCL_PIN 8
#define EEPROM_BASE_ADDR 0x50

// EEPROM Buffer Configuration
#define BUFFER_START_ADDRESS 0x100
#define MAX_TRANSACTIONS 100

// User-defined struct stored in EEPROM
typedef struct
{
  char name[10];
  int pin;
  float balance;
  unsigned long walletID;
} EEPROMData;

// Calculates buffer end based on struct size and count
#define BUFFER_END_ADDRESS (MAX_TRANSACTIONS * sizeof(TransactionData) + BUFFER_START_ADDRESS)

// Function declarations
void writeStructEEPROM(uint16_t start_addr, const void *data, size_t size);
void readStructEEPROM(uint16_t start_addr, void *data_out, size_t size);
void addTransaction(const TransactionData *transaction);
String getAllTransactions();
void loadHeadTailFromEEPROM(bool clear = false);
void saveHeadTailToEEPROM();
void readEncryptedEEPROM(uint16_t start_addr, void *data_out, size_t size);
void writeEncryptedEEPROM(uint16_t start_addr, const void *data, size_t size);

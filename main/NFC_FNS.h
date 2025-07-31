#pragma once
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define SDA_PIN 7
#define SCL_PIN 8

void printTransactionData();
void readNFC();
bool isTransactionDataComplete();
void clearTransactionData();
void printTransactionData();
void NFC_init();
int getTransactionAmount();
void writeNFC(int TID);

// Structure to store transaction data in guaranteed order
struct TransactionData
{
    char transactionAmount[5]; // Record 0
    char creditCardNumber[20]; // Record 1
    char currency[5];          // Record 2
    float latitude;
    float longitude;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

TransactionData getLatestTransaction();
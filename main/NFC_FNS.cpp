#include "NFC_FNS.h"

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
TransactionData currentTransaction;

TransactionData getLatestTransaction()
{
    return currentTransaction;
}

void NFC_init()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    esp_log_level_set("i2c.master", ESP_LOG_NONE);
    esp_log_level_set("esp32-hal-i2c-ng.c", ESP_LOG_NONE);
    esp_log_level_set("Wire.cpp", ESP_LOG_NONE);
    Serial.println("NDEF Reader Initializing...");
    nfc.begin();
}

void readNFC()
{
    Serial.println("READING: Waiting for NFC tag...");
    unsigned long timeout = millis() + 5000;
    clearTransactionData();

    while (millis() < timeout)
    {
        if (nfc.tagPresent())
        {
            Serial.println("Tag detected, reading...");
            NfcTag tag = nfc.read();

            if (tag.hasNdefMessage())
            {
                NdefMessage message = tag.getNdefMessage();
                int recordCount = message.getRecordCount();

                Serial.print("Found ");
                Serial.print(recordCount);
                Serial.println(" NDEF records");

                if (recordCount < 3)
                {
                    Serial.println("Error: Expected at least 3 text records (amount, card number, currency)");
                    delay(1000);
                    return;
                }

                for (int i = 0; i < recordCount && i < 3; i++)
                {
                    NdefRecord record = message.getRecord(i);
                    int payloadLength = record.getPayloadLength();
                    byte payload[payloadLength];
                    record.getPayload(payload);

                    byte statusByte = payload[0];
                    byte languageCodeLength = statusByte & 0x3F;
                    int textLength = payloadLength - 1 - languageCodeLength;

                    if (textLength <= 0)
                        continue;

                    String text = "";
                    for (int j = 0; j < textLength; j++)
                    {
                        text += (char)payload[1 + languageCodeLength + j];
                    }

                    Serial.print("Read NFC Text: ");
                    Serial.println(text);

                    // Copy text to struct fields using strncpy
                    if (i == 0)
                    {
                        strncpy(currentTransaction.transactionAmount, text.c_str(), sizeof(currentTransaction.transactionAmount) - 1);
                        currentTransaction.transactionAmount[sizeof(currentTransaction.transactionAmount) - 1] = '\0';
                        Serial.println("  -> Assigned as Transaction Amount");
                    }
                    else if (i == 1)
                    {
                        strncpy(currentTransaction.creditCardNumber, text.c_str(), sizeof(currentTransaction.creditCardNumber) - 1);
                        currentTransaction.creditCardNumber[sizeof(currentTransaction.creditCardNumber) - 1] = '\0';
                        Serial.println("  -> Assigned as Credit Card Number");
                    }
                    else if (i == 2)
                    {
                        strncpy(currentTransaction.currency, text.c_str(), sizeof(currentTransaction.currency) - 1);
                        currentTransaction.currency[sizeof(currentTransaction.currency) - 1] = '\0';
                        Serial.println("  -> Assigned as Currency");
                    }
                }

                if (isTransactionDataComplete())
                {
                    printTransactionData();
                }
                else
                {
                    Serial.println("Failed to extract complete transaction data");
                }
            }
            else
            {
                Serial.println("No NDEF message found");
            }

            delay(1000);
            return;
        }

        delay(100);
        yield();
    }

    Serial.println("NFC Timeout.");
}

bool isTransactionDataComplete()
{
    return strlen(currentTransaction.transactionAmount) > 0 &&
           strlen(currentTransaction.creditCardNumber) > 0 &&
           strlen(currentTransaction.currency) > 0;
}

void clearTransactionData()
{
    currentTransaction.transactionAmount[0] = '\0';
    currentTransaction.creditCardNumber[0] = '\0';
    currentTransaction.currency[0] = '\0';
}

void printTransactionData()
{
    Serial.println("\n=== TRANSACTION DATA ===");
    Serial.print("Transaction Amount: ");
    Serial.println(currentTransaction.transactionAmount);
    Serial.print("Credit Card Number: ");
    Serial.println(currentTransaction.creditCardNumber);
    Serial.print("Currency: ");
    Serial.println(currentTransaction.currency);
    Serial.println("========================\n");
}

int getTransactionAmount()
{
    if (strlen(currentTransaction.transactionAmount) == 0)
    {
        Serial.println("Scan a tag first");
        return 0;
    }
    return atoi(currentTransaction.transactionAmount); // Convert char[] to int
}

void writeNFC(int TID)
{
    Serial.println("WRITING: Waiting for NFC tag...");
    unsigned long timeout = millis() + 5000;

    while (millis() < timeout)
    {
        if (nfc.tagPresent())
        {
            Serial.println("Tag detected, writing...");
            NdefMessage message = NdefMessage();
            String transactionString = "TXN ID: " + String(TID);
            message.addTextRecord(transactionString);

            bool success = nfc.write(message);
            if (success)
                Serial.println("Write success!");
            else
                Serial.println("Write failed!");

            delay(1000);
            return;
        }

        delay(100);
        yield();
    }

    Serial.println("NFC Timeout.");
}

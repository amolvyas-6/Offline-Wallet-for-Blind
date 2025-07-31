#include "EEPROM_FNS.h"

uint16_t tailAddress = BUFFER_START_ADDRESS - 1;
uint16_t headAddress = BUFFER_START_ADDRESS - 1;

void writeEncryptedEEPROM(uint16_t start_addr, const void *data, size_t size)
{
  // Calculate padded size for AES (must be multiple of 16)
  size_t padded_size = ((size + 15) / 16) * 16;

  uint8_t *buffer = (uint8_t *)malloc(padded_size);

  memset(buffer, 0, padded_size); // Zero-pad the entire buffer
  memcpy(buffer, data, size);

  uint8_t *encrypted_data = (uint8_t *)malloc(padded_size);

  aesEncrypt(buffer, encrypted_data, padded_size);
  writeStructEEPROM(start_addr, encrypted_data, padded_size);

  free(buffer);
  free(encrypted_data);
}

void readEncryptedEEPROM(uint16_t start_addr, void *data_out, size_t size)
{
  size_t padded_size = ((size + 15) / 16) * 16;
  uint8_t *buffer = (uint8_t *)malloc(padded_size);
  readStructEEPROM(start_addr, buffer, padded_size);

  uint8_t *decryptedData = (uint8_t *)malloc(padded_size);
  aesDecrypt(buffer, decryptedData, padded_size);

  memcpy(data_out, decryptedData, size);
}
void writeStructEEPROM(uint16_t start_addr, const void *data, size_t size)
{
  const uint8_t *raw = (const uint8_t *)data;
  uint8_t buffer[size];
  memcpy(buffer, raw, size);

  for (size_t i = 0; i < size; ++i)
  {
    Wire.beginTransmission(EEPROM_BASE_ADDR);
    Wire.write((uint8_t)((start_addr + i) >> 8));   // MSB of address
    Wire.write((uint8_t)((start_addr + i) & 0xFF)); // LSB of address
    Wire.write(buffer[i]);
    Wire.endTransmission();
    delay(5); // EEPROM write delay
  }
}
// Generic read function for any struct
void readStructEEPROM(uint16_t start_addr, void *data_out, size_t size)
{
  uint8_t *raw = (uint8_t *)data_out;

  for (size_t i = 0; i < size; ++i)
  {
    Wire.beginTransmission(EEPROM_BASE_ADDR);
    Wire.write((uint8_t)((start_addr + i) >> 8));   // MSB
    Wire.write((uint8_t)((start_addr + i) & 0xFF)); // LSB
    Wire.endTransmission(false);                    // Repeated start

    Wire.requestFrom(EEPROM_BASE_ADDR, 1);
    if (Wire.available())
    {
      raw[i] = Wire.read();
    }
    else
    {
      raw[i] = 0xFF; // Default value in case of error
    }
  }
}

// Add a transaction to circular buffer
void addTransaction(const TransactionData *transaction)
{
  if (tailAddress == BUFFER_START_ADDRESS - 1)
  {
    headAddress = BUFFER_START_ADDRESS;
    tailAddress = BUFFER_START_ADDRESS;
  }
  else
  {
    tailAddress += sizeof(TransactionData);
    if (tailAddress >= BUFFER_END_ADDRESS)
    {
      tailAddress = BUFFER_START_ADDRESS;
    }
    if (tailAddress == headAddress)
    {
      headAddress += sizeof(TransactionData);
      if (headAddress >= BUFFER_END_ADDRESS)
      {
        headAddress = BUFFER_START_ADDRESS;
      }
    }
  }

  writeStructEEPROM(tailAddress, transaction, sizeof(TransactionData));
  saveHeadTailToEEPROM();
}

// Print all transactions from circular buffer
String getAllTransactions()
{

  if (headAddress == BUFFER_START_ADDRESS - 1)
  {
    Serial.println("No transactions");
    return "<p>No Transactions</p>";
  }

  String table = "<table class = \"transaction-table\"> <tr><th>Transaction Amount</th> <th>Recipient Card Number</th> <th>Currency</th> <th>Latitude</th> <th>Longitude</th> <th>Date</th> <th>Time</th></tr>\n";
  uint16_t currentAddress = headAddress;
  TransactionData transaction;

  while (true)
  {
    readStructEEPROM(currentAddress, &transaction, sizeof(TransactionData));

    table += "<tr><td>" +
             String(transaction.transactionAmount) +
             " </td> <td>" + String(transaction.creditCardNumber) +
             " </td> <td>" + String(transaction.currency) +
             " </td> <td>" + String(transaction.latitude) +
             " </td> <td>" + String(transaction.longitude) +
             " </td> <td>" + String(transaction.day) + "/" + String(transaction.month) + "/" + String(transaction.year) +
             " </td> <td>" + String(transaction.hour) + ":" + String(transaction.minute) + ":" + String(transaction.second) +
             " </td></tr>\n";

    if (currentAddress == tailAddress)
      break;

    currentAddress += sizeof(TransactionData);
    if (currentAddress >= BUFFER_END_ADDRESS)
    {
      currentAddress = BUFFER_START_ADDRESS;
    }
  }
  table += "</table>\n";
  return table;
}

#define ADDR_HEAD_TAIL 0x80

void saveHeadTailToEEPROM()
{
  uint16_t buffer[2] = {headAddress, tailAddress};
  writeStructEEPROM(ADDR_HEAD_TAIL, buffer, sizeof(buffer));
}

void loadHeadTailFromEEPROM(bool clear)
{
  uint16_t buffer[2];
  readStructEEPROM(ADDR_HEAD_TAIL, buffer, sizeof(buffer));
  headAddress = buffer[0];
  tailAddress = buffer[1];

  // Check for uninitialized EEPROM (0xFFFF or out-of-range values)
  bool invalid = (headAddress < BUFFER_START_ADDRESS || headAddress > BUFFER_END_ADDRESS ||
                  tailAddress < BUFFER_START_ADDRESS - 1 || tailAddress > BUFFER_END_ADDRESS);

  if (headAddress == 0xFFFF || tailAddress == 0xFFFF || invalid || clear)
  {
    // First run or invalid data, initialize
    headAddress = BUFFER_START_ADDRESS - 1;
    tailAddress = BUFFER_START_ADDRESS - 1;
    saveHeadTailToEEPROM();
  }
}
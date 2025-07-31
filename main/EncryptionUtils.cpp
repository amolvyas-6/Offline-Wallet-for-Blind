#include "EncryptionUtils.h"
#include "mbedtls/aes.h"
#include <Arduino.h>

mbedtls_aes_context aes;
unsigned char key[32] = "abcdefghijklmnopqrstuvwxyzabcde";
unsigned char iv[16] = "aesencryptionis";

// Simple XOR encryption; replace with a stronger algorithm if needed
void encryptData(uint8_t *data, size_t size)
{
    uint8_t key = 0xAA;
    for (size_t i = 0; i < size; i++)
    {
        data[i] ^= key;
    }
}

void decryptData(uint8_t *data, size_t size)
{
    encryptData(data, size);
}

void aesEncrypt(uint8_t *input, uint8_t *output, size_t size)
{
    unsigned char iv_local[16];
    memcpy(iv_local, iv, 16);
    mbedtls_aes_setkey_enc(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, size, iv_local, input, output);
}

void aesDecrypt(uint8_t *input, uint8_t *output, size_t size)
{
    Serial.print("Encrytped Data: ");
    for (size_t i = 0; i < size; ++i)
    {
        Serial.printf("%02X ", output[i]);
    }
    Serial.println();
    unsigned char iv_local[16];
    memcpy(iv_local, iv, 16);
    mbedtls_aes_setkey_dec(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, size, iv_local, input, output);
}

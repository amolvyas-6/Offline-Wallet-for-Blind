#pragma once

#include <stdint.h>
#include <stddef.h>

// Function declarations
void encryptData(uint8_t *data, size_t size);
void decryptData(uint8_t *data, size_t size);

void aesEncrypt(uint8_t *input, uint8_t *output, size_t size);
void aesDecrypt(uint8_t *input, uint8_t *output, size_t size);

# Adafruit Bus IO Library
# https://github.com/adafruit/Adafruit_BusIO
# MIT License

cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/MifareClassic.cpp" "src/MifareUltralight.cpp" "src/Ndef.cpp" "src/NdefMessage.cpp" "src/NdefRecord.cpp" "src/NfcAdapter.cpp" "src/NfcTag.cpp" 
                       INCLUDE_DIRS "." "src"
                       REQUIRES arduino-esp32 PN532)


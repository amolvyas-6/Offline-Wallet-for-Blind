
# Adafruit Bus IO Library
# https://github.com/adafruit/Adafruit_BusIO
# MIT License

cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/emulatetag.cpp" "src/llcp.cpp" "src/mac_link.cpp" "src/PN532_I2C.cpp" "src/PN532_SPI.cpp" "src/PN532.cpp" "src/snep.cpp" 
                       INCLUDE_DIRS "." "src"
                       REQUIRES arduino-esp32)


idf_component_register(SRCS "GPS_FNS.cpp" "EncryptionUtils.cpp" "NFC_FNS.cpp" "main.cpp" "EEPROM_FNS.cpp"
                    INCLUDE_DIRS ".")
# Get the library file path from the arduino-esp32 component
idf_component_get_property(arduino_lib espressif__arduino-esp32 COMPONENT_LIB)

# Get the include directories from the arduino-esp32 component
idf_component_get_property(arduino_include_dirs espressif__arduino-esp32 INCLUDE_DIRS)

# Link the arduino library to your final executable and add its include paths
target_link_libraries(${COMPONENT_LIB} PRIVATE ${arduino_lib})
target_include_directories(${COMPONENT_LIB} PRIVATE ${arduino_include_dirs})
# Project Description

The "Wallet for the Blind" is an innovative payment solution designed specifically for visually impaired users, enabling them to perform tap-and-pay transactions with enhanced security. Current payment systems present significant challenges for visually impaired individuals, often leaving them vulnerable to fraud and making financial transactions difficult. This project addresses these issues by integrating advanced technologies, including a sensor-driven haptic feedback system for two-factor authentication (2FA) and the use of encrypted EEPROM for secure user data storage.

The project aims to increase financial independence among the visually impaired community, providing an easy-to-use, secure wallet that operates through familiar tactile interactions. This report will outline the technology used, the development process, and the expected impact on users who face barriers in traditional banking and financial systems.

# Steps to setup IDE:

1. Install VS Code
2. Install `ESP-IDF Extension` for VS Code.
3. Setup the Extension by following the instructions provided.
4. Choose ESP-IDF version as `v5.4.*`.
5. Leave everything else as default.
6. Wait for setup to complete.
7. Run `idf.py menuconfig` in the IDF terminal and enable auto start Arduino setup and loop options under Arduno configuration.
8. In the generated `sdkconfig` file, Search for the key - `COMFIG_FREERTOS_HZ` - and change its value from `100` to `1000`.
9. While building, select flashing mode as `UART` and select device as `esp32s3`.

# Components Used:

1. PN532 - NFC Module
2. AT24C256LC - EEPROM
3. Push Button with Vibrator Motor for Haptic Feedback
4. Waveshare A7670E development board

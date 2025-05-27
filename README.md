# ESP32 Smart Watch

This project implements a basic smartwatch using the ESP32, built with ESP-IDF. It integrates FreeRTOS with bare-metal I2C drivers to communicate with multiple sensors and an OLED display.

## Features

- Real-time step count and heart rate monitoring
- Custom I2C driver for communication with sensors and display
- Task scheduling using FreeRTOS
- OLED display output using SSD1306 over I2C

## Components

- ESP32 Dev Board
- SSD1306 OLED Display
- MAX30102 Heart Rate Sensor
- ADXL345 Accelerometer

# json2esp32

Utility for converting JSON board definitions into ESP32-compatible C/C++ board support code.

Overview

json2esp32 is a lightweight tool that generates boilerplate code for ESP32-based hardware projects from simple JSON descriptions.

It is inspired by tools like json2daisy, but adapted to the flexibility and diversity of the ESP32 ecosystem.

Instead of manually writing GPIO mappings, ADC setups, I2S configs, and control abstractions, you define your hardware once in JSON and generate consistent, reusable code.

#ifndef __TWAI_PORT_H
#define __TWAI_PORT_H

#pragma once

#include <Arduino.h>
#include "driver/twai.h"
#include <ESP_IOExpander_Library.h>

// Pins used to connect to CAN bus transceiver:
#define RX_PIN 16
#define TX_PIN 15

// Global variables for Subject ID 6160 (Information Upload)
extern uint16_t global_electrical_speed;    // Electrical Speed (0.1 Hz)
extern int16_t global_bus_current;          // Bus Current (0.1 A)
extern uint16_t global_operating_status;    // Operating Status

// Global variables for Subject ID 6161 (Information Upload)
extern uint16_t global_output_throttle;     // Output Throttle
extern int16_t global_bus_voltage;          // Bus Voltage (0.1 V)
extern uint8_t global_mos_temperature;      // MOS Temperature (°C)
extern uint8_t global_cap_temperature;      // Capacitor Temperature (°C)
extern uint8_t global_motor_temperature;    // Motor Temperature (°C)

// Intervall:
#define POLLING_RATE_MS 500

bool waveshare_twai_init();
void waveshare_twai_receive();

#endif
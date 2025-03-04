#include "waveshare_twai_port.h"
#include <canard.h>
#include <stdlib.h> // For malloc() and free()
#include <string.h> // For memcpy()


// Define your node ID
#define NODE_ID 16

// Declare global variables
CanardInstance canard; // Declare the canard instance
CanardTxQueue queue;
CanardRxSubscription subscription;

// initialize Global variables for Subject ID 6160 (Information Upload)
uint16_t global_electrical_speed = 0;    // Electrical Speed (0.1 Hz)
int16_t global_bus_current = 0;          // Bus Current (0.1 A)
uint16_t global_operating_status = 0;    // Operating Status

// initialize Global variables for Subject ID 6161 (Information Upload)
uint16_t global_output_throttle = 0;     // Output Throttle
int16_t global_bus_voltage = 0;          // Bus Voltage (0.1 V)
uint8_t global_mos_temperature = 0;      // MOS Temperature (°C)
uint8_t global_cap_temperature = 0;      // Capacitor Temperature (°C)
uint8_t global_motor_temperature = 0;    // Motor Temperature (°C)

// Memory allocation function
void* memAllocate(CanardInstance* const ins, size_t amount) {
  (void)ins; // Unused parameter
  return malloc(amount); // Allocate memory using malloc()
}

// Memory deallocation function
void memFree(CanardInstance* const ins, void* pointer) {
  (void)ins; // Unused parameter
  free(pointer); // Free memory using free()
}


// Function to initialize the TWAI driver
bool waveshare_twai_init() {
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_LISTEN_ONLY);
  g_config.rx_queue_len = 50; // Increase RX queue size

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("Failed to install driver");
    return false;
  }

  if (twai_start() != ESP_OK) {
    Serial.println("Failed to start driver");
    return false;
  }

  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) != ESP_OK) {
    Serial.println("Failed to reconfigure alerts");
    return false;
  }


  // Initialize the XC-CYPHAL instance
    canard = canardInit(&memAllocate, &memFree); // Initialize canard
    canard.node_id = NODE_ID; // Set the node ID
    queue = canardTxInit(100, 1024); // Initialize the TX queue with 100 transfers and a 1024-byte payload buffer

    // Subscribe to a specific message type

        // Declare local subscription variables
    CanardRxSubscription subscription_6160;
    CanardRxSubscription subscription_6161;

    // Subscribe to Subject ID 6160
    canardRxSubscribe(
        &canard,                      // Canard instance
        CanardTransferKindMessage,    // Transfer kind (message)
        6160,                         // Subject ID 6160
        100,                          // Maximum payload size
        1000000,                      // Transfer ID timeout (1 second in microseconds)
        &subscription_6160            // Output subscription
    );

    // Subscribe to Subject ID 6161
    canardRxSubscribe(
        &canard,                      // Canard instance
        CanardTransferKindMessage,    // Transfer kind (message)
        6161,                         // Subject ID 6161
        100,                          // Maximum payload size
        1000000,                      // Transfer ID timeout (1 second in microseconds)
        &subscription_6161            // Output subscription
    );

  return true;
}


void waveshare_twai_receive() {
    uint32_t alerts_triggered;
    twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(10)); // Check alerts every 10 ms

    if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
        Serial.println("Alert: The RX queue is full causing a received frame to be lost.");
    }

    if (alerts_triggered & TWAI_ALERT_RX_DATA) {
        twai_message_t message;
        while (twai_receive(&message, 0) == ESP_OK) {
            // Decode the CAN ID according to the XC-Cyphal protocol
            uint32_t can_id = message.identifier;
            uint8_t priority = (can_id >> 26) & 0x07; // Bits 28-26
            uint8_t service_not_message = (can_id >> 25) & 0x01; // Bit 25
            uint8_t anonymous = (can_id >> 24) & 0x01; // Bit 24
            uint16_t subject_id = (can_id >> 8) & 0x1FFF; // Bits 23-11
            uint8_t source_node_id = can_id & 0x7F; // Bits 6-0

            // Print the decoded CAN ID fields
            // Serial.printf("CAN ID: 0x%08X\n", can_id);
            // Serial.printf("Priority: %d\n", priority);
            // Serial.printf("Service/Message: %d\n", service_not_message);
            // Serial.printf("Anonymous: %d\n", anonymous);
            // Serial.printf("Subject ID: %d\n", subject_id);
            // Serial.printf("Source Node ID: %d\n", source_node_id);
    
                    // Decode the payload based on the Subject ID
                    if (subject_id == 6160) { // Information Upload 6160
                        // Update global variables
                        global_electrical_speed = *(uint16_t*)&message.data[0];
                        global_bus_current = *(int16_t*)&message.data[2];
                        global_operating_status = *(uint16_t*)&message.data[4];

                        // Print decoded values
                        // Serial.printf("Electrical Speed: %u (0.1 Hz)\n", global_electrical_speed);
                        // Serial.printf("Bus Current: %d (0.1 A)\n", global_bus_current);
                        // Serial.printf("Operating Status: 0x%04X\n", global_operating_status);
                    } else if (subject_id == 6161) { // Information Upload 6161
                        // Update global variables
                        global_output_throttle = *(uint16_t*)&message.data[0];
                        global_bus_voltage = *(int16_t*)&message.data[2];
                        global_mos_temperature = message.data[4] - 40; // Convert to °C
                        global_cap_temperature = message.data[5] - 40; // Convert to °C
                        global_motor_temperature = message.data[6] - 40; // Convert to °C

                        // Print decoded values
                        // Serial.printf("Output Throttle: %u\n", global_output_throttle);
                        // Serial.printf("Bus Voltage: %d (0.1 V)\n", global_bus_voltage);
                        // Serial.printf("MOS Temperature: %d °C\n", global_mos_temperature);
                        // Serial.printf("Capacitor Temperature: %d °C\n", global_cap_temperature);
                        // Serial.printf("Motor Temperature: %d °C\n", global_motor_temperature);  
                    }
            // Print the payload in both hex and raw formats
            // Serial.print("Payload (Hex): ");
            // for (int i = 0; i < message.data_length_code; i++) {
            //     Serial.printf("%02X ", message.data[i]);
            // }
            // Serial.println();

            // Serial.print("Payload (Raw): ");
            // for (int i = 0; i < message.data_length_code; i++) {
            //     Serial.printf("%d ", message.data[i]);
            // }
            // Serial.println();
        }
              
    }
}
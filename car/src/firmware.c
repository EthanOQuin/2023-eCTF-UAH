/**
 * @file main.c
 * @author Frederich Stine
 * @brief eCTF Car Example Design Implementation
 * @date 2023
 *
 * This source file is part of an example system for MITRE's 2023 Embedded
 * System CTF (eCTF). This code is being provided only for educational purposes
 * for the 2023 MITRE eCTF competition, and may not meet MITRE standards for
 * quality. Use this code at your own risk!
 *
 * @copyright Copyright (c) 2023 The MITRE Corporation
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "hydrogen.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/eeprom.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "secrets.h"

#include "board_link.h"
#include "debug.h"
#include "enc.h"
#include "feature_list.h"
#include "hwsec.h"
#include "uart.h"

/*** Structure definitions ***/
// Structure of start_car packet FEATURE_DATA
typedef struct {
  uint32_t car_id;
  uint8_t feature;
  uint8_t signature[hydro_sign_BYTES];
} __attribute__((packed)) ENABLE_PACKET;

typedef struct {
  uint32_t car_id;
  uint8_t num_active;
  uint8_t features[NUM_FEATURES];
  uint8_t signatures[NUM_FEATURES][hydro_sign_BYTES];
} FEATURE_DATA;

/*** Macro Definitions ***/
// Definitions for unlock message location in EEPROM
#define UNLOCK_EEPROM_LOC 0x7C0
#define UNLOCK_EEPROM_SIZE 64

/*** Function definitions ***/
// Core functions - performHandshake, unlockCar, and startCar
uint32_t performHandshake(void);
void unlockCar(void);
void startCar(void);

// Helper functions - sending ack messages
void sendAckSuccess(void);
void sendAckFailure(void);

// Declare Car ID
const uint32_t car_id = CAR_ID;

// Inter-board message encryption key
uint8_t *message_key = MESSAGE_KEY;

// Feature package verification key
uint8_t *feature_verification_key = SIGNING_PUBLIC_KEY;

/**
 * @brief Main function for the car example
 *
 * Initializes the RF module and waits for a successful unlock attempt.
 * If successful prints out the unlock flag.
 */
int main(void) {
  // Lock down unused board functionality
  lockdown();

  // Ensure EEPROM peripheral is enabled
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  EEPROMInit();

  // Change LED color: red
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // r
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);          // b
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);          // g

  // Initialize UART peripheral
  uart_init();

  // Initialize board link UART
  setup_board_link();

  // Initialize libhydrogen
  hydro_init();

  while (true) {
    unlockCar();
  }
}

/**
 * @brief Function implementing simple handshake between car and fob. Returns
 * nonce to be used when processing unlock packet.
 */
uint32_t performHandshake(void) {
  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  message.buffer = buffer;

  debug_print("\r\nWaiting for handshake request");

  receive_board_message_by_type(&message, HANDSHAKE_MAGIC);

  debug_print("\r\nHandshake request received, returning handshake packet");

  // Nonce to be used in unlock request
  uint32_t nonce = hydro_random_u32();
  memcpy(&(message.buffer[0]), &nonce, 4);

  message.message_len = 4;
  message.magic = HANDSHAKE_MAGIC;

  send_board_message(&message);

  return nonce;
}

/**
 * @brief Function that handles unlocking of car
 */
void unlockCar(void) {
  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  message.buffer = buffer;

  debug_print("\r\n\n---- Unlock ----\n");

  // Perform handshake to share nonce between car and fob
  uint32_t nonce = performHandshake();

  // Receive packet with some error checking
  debug_print("\r\nWaiting for unlock message");

  receive_board_message_by_type(&message, UNLOCK_MAGIC);

  debug_print("\r\nUnlock message received\r\n\n");

  uint32_t received_nonce;
  memcpy(&received_nonce, &(message.buffer[0]), 4);

  // If the data transfer is the nonce, unlock
  if (received_nonce == nonce) {
    uint8_t eeprom_message[64];
    // Read last 64B of EEPROM
    EEPROMRead((uint32_t *)eeprom_message, UNLOCK_EEPROM_LOC,
               UNLOCK_EEPROM_SIZE);

    debug_print("\r\n\n==== Begin Unlock Message =====\r\n");
    uart_write(HOST_UART, eeprom_message, UNLOCK_EEPROM_SIZE);
    debug_print("\r\n==== End Unlock Message =====\n");

    sendAckSuccess();

    startCar();
  } else {
    sendAckFailure();
  }
}

/**
 * @brief Function that handles starting of car - feature list
 */
void startCar(void) {
  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  message.buffer = buffer;

  debug_print("\r\n\n---- Start ----\n");

  // Receive start message
  receive_board_message_by_type(&message, START_MAGIC);

  FEATURE_DATA *feature_info = (FEATURE_DATA *)buffer;

  // Verify correct car id
  if (car_id != feature_info->car_id) {
    return;
  }

  // Verify signatures of all active features
  debug_print("\r\nBegin Feature Verification");
  ENABLE_PACKET e;
  e.car_id = car_id;
  for (int i = 0; i < feature_info->num_active; i++) {
    e.feature = feature_info->features[i];

    // If feature signature invalid, exit
    if (hydro_sign_verify(feature_info->signatures[i], &e,
                          sizeof(e.car_id) + sizeof(e.feature), "feature",
                          feature_verification_key) != 0) {
      debug_print("\r\nERROR: Feature verification failed.");
      return;
    }
  }
  debug_print("\r\nFeature Verification Complete");

  // Print out features for all active features
  debug_print("\r\n\n==== Begin Feature Message =====");
  for (int i = 0; i < feature_info->num_active; i++) {
    uint8_t eeprom_message[64];

    uint32_t offset = feature_info->features[i] * FEATURE_SIZE;

    if (offset > FEATURE_END) {
      offset = FEATURE_END;
    }

    EEPROMRead((uint32_t *)eeprom_message, FEATURE_END - offset, FEATURE_SIZE);

    debug_print("\r\n");
    uart_write(HOST_UART, eeprom_message, FEATURE_SIZE);
  }

  debug_print("\r\n==== End Feature Message =====\n");

  // Change LED color: green
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);          // r
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);          // b
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // g
}

/**
 * @brief Function to send successful ACK message
 */
void sendAckSuccess(void) {
  // Create packet for successful ack and send
  MESSAGE_PACKET message;

  debug_print("\r\nSending ACK success");

  uint8_t buffer[1];
  message.buffer = buffer;
  message.magic = ACK_MAGIC;
  buffer[0] = ACK_SUCCESS;
  message.message_len = 1;

  send_board_message(&message);
}

/**
 * @brief Function to send unsuccessful ACK message
 */
void sendAckFailure(void) {
  // Create packet for unsuccessful ack and send
  MESSAGE_PACKET message;

  debug_print("\r\nSending ACK failure");

  uint8_t buffer[1];
  message.buffer = buffer;
  message.magic = ACK_MAGIC;
  buffer[0] = ACK_FAIL;
  message.message_len = 1;

  send_board_message(&message);
}

/**
 * @file main.c
 * @author Frederich Stine
 * @brief eCTF Fob Example Design Implementation
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

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/eeprom.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "hydrogen.h"

#include "secrets.h"

#include "board_link.h"
#include "debug.h"
#include "enc.h"
#include "feature_list.h"
#include "hwsec.h"
#include "uart.h"

#define FOB_STATE_PTR 0x3FC00
#define FLASH_DATA_SIZE                                                        \
  (sizeof(FLASH_DATA) % 4 == 0)                                                \
      ? sizeof(FLASH_DATA)                                                     \
      : sizeof(FLASH_DATA) + (4 - (sizeof(FLASH_DATA) % 4))
#define FLASH_PAIRED 0x00
#define FLASH_UNPAIRED 0xFF

/*** Structure definitions ***/
// Defines a struct for the format of an enable message
typedef struct {
  uint32_t car_id;
  uint8_t feature;
  uint8_t signature[hydro_sign_BYTES];
} __attribute__((packed)) ENABLE_PACKET;

// Defines a struct for the format of a pairing message
typedef struct {
  uint32_t car_id;
  uint8_t pin[8];
  uint8_t message_key[hydro_secretbox_KEYBYTES];
} PAIR_PACKET;

// Defines a struct for the format of start message
typedef struct {
  uint32_t car_id;
  uint8_t num_active;
  uint8_t features[NUM_FEATURES];
  uint8_t signatures[NUM_FEATURES][hydro_sign_BYTES];
} FEATURE_DATA;

// Defines a struct for storing the state in flash
typedef struct {
  uint8_t paired;
  PAIR_PACKET pair_info;
  FEATURE_DATA feature_info;
  uint8_t padding[3];
} FLASH_DATA;

/*** Function definitions ***/
// Core functions - all functionality supported by fob
void saveFobState(FLASH_DATA *flash_data);
void pairFob(FLASH_DATA *fob_state_ram);
uint32_t performHandshake(void);
void unlockCar(FLASH_DATA *fob_state_ram);
void enableFeature(FLASH_DATA *fob_state_ram);
void startCar(FLASH_DATA *fob_state_ram);

// Helper functions - receive ack message
uint8_t receiveAck();

// Inter-board message encryption key
uint8_t *message_key = MESSAGE_KEY;

// Feature package verification key
uint8_t *feature_verification_key = SIGNING_PUBLIC_KEY;

/**
 * @brief Main function for the fob example
 *
 * Listens over UART and SW1 for an unlock command. If unlock command presented,
 * attempts to unlock door. Listens over UART for pair command. If pair
 * command presented, attempts to either pair a new key, or be paired
 * based on firmware build.
 */
int main(void) {
  FLASH_DATA fob_state_ram;
  FLASH_DATA *fob_state_flash = (FLASH_DATA *)FOB_STATE_PTR;

  // Lock down unused board functionality
  lockdown();

  // Initialize UART (early for debugging)
  uart_init();

  // Initialize libhydrogen
  hydro_init();

// If paired fob, initialize the system information and save to flash
#if PAIRED == 1
  if (fob_state_flash->paired == FLASH_UNPAIRED) {
    strcpy((char *)(fob_state_ram.pair_info.pin), PAIR_PIN);
    fob_state_ram.pair_info.car_id = CAR_ID;
    fob_state_ram.feature_info.car_id = CAR_ID;

    memcpy(&fob_state_ram.pair_info.message_key, message_key,
           hydro_secretbox_KEYBYTES);

    fob_state_ram.paired = FLASH_PAIRED;

    saveFobState(&fob_state_ram);
  }
#else
  fob_state_ram.paired = FLASH_UNPAIRED;
#endif

  if (fob_state_flash->paired == FLASH_PAIRED) {
    debug_print("\r\nFob paired to car, loading data");
    memcpy(&fob_state_ram, fob_state_flash, FLASH_DATA_SIZE);

    message_key = fob_state_ram.pair_info.message_key;
  } else {
    debug_print("\r\nFob not paired to car");
  }

  // This will run on first boot to initialize features
  if (fob_state_ram.feature_info.num_active == 0xFF) {
    fob_state_ram.feature_info.num_active = 0;
    saveFobState(&fob_state_ram);
  }

  // Initialize board link UART
  setup_board_link();

  // Setup SW1
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA,
                   GPIO_PIN_TYPE_STD_WPU);

  // Change LED color: white
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // r
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // b
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // g

  // Declare a buffer for reading and writing to UART
  uint8_t uart_buffer[10];
  uint8_t uart_buffer_index = 0;

  uint8_t previous_sw_state = GPIO_PIN_4;
  uint8_t debounce_sw_state = GPIO_PIN_4;
  uint8_t current_sw_state = GPIO_PIN_4;

  // Infinite loop for polling UART
  while (true) {

    // Non blocking UART polling
    if (uart_avail(HOST_UART)) {
      uint8_t uart_char = (uint8_t)uart_readb(HOST_UART);

      if ((uart_char != '\r') && (uart_char != '\n') && (uart_char != '\0') &&
          (uart_char != 0xD)) {
        uart_buffer[uart_buffer_index] = uart_char;
        uart_buffer_index++;
      } else {
        uart_buffer[uart_buffer_index] = 0x00;
        uart_buffer_index = 0;

        if (!(strcmp((char *)uart_buffer, "enable"))) {
          enableFeature(&fob_state_ram);
        } else if (!(strcmp((char *)uart_buffer, "pair"))) {
          pairFob(&fob_state_ram);
        }
      }
    }

    current_sw_state = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
    if ((current_sw_state != previous_sw_state) && (current_sw_state == 0)) {
      // Debounce switch
      for (int i = 0; i < 10000; i++)
        ;
      debounce_sw_state = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
      if (debounce_sw_state == current_sw_state) {
        debug_print("\r\nUnlocking car");
        unlockCar(&fob_state_ram);

        debug_print("\r\nWaiting for ack");
        if (receiveAck()) {
          debug_print("\r\nAck received, starting car");
          startCar(&fob_state_ram);
        }
      }
    }
    previous_sw_state = current_sw_state;
  }
}

/**
 * @brief Function that carries out pairing of the fob
 *
 * @param fob_state_ram pointer to the current fob state in ram
 */
void pairFob(FLASH_DATA *fob_state_ram) {
  debug_print("\r\n\n---- Pair Fob ----\n");
  MESSAGE_PACKET message;

  // Start pairing transaction - fob is already paired
  if (fob_state_ram->paired == FLASH_PAIRED) {
    int16_t bytes_read;
    uint8_t uart_buffer[8];
    uart_write(HOST_UART, (uint8_t *)"P", 1);
    bytes_read = uart_readline(HOST_UART, uart_buffer);

    if (bytes_read == 6) {
      // If the pin is correct
      if (!(strcmp((char *)uart_buffer,
                   (char *)fob_state_ram->pair_info.pin))) {
        // Pair the new key by sending a PAIR_PACKET structure
        // with required information to unlock door
        message.message_len = sizeof(PAIR_PACKET);
        message.magic = PAIR_MAGIC;
        message.buffer = (uint8_t *)&fob_state_ram->pair_info;
        send_board_message(&message);
      }
    }
  }

  // Start pairing transaction - fob is not paired
  else {
    message.buffer = (uint8_t *)&fob_state_ram->pair_info;
    receive_board_message_by_type(&message, PAIR_MAGIC);
    fob_state_ram->paired = FLASH_PAIRED;

    fob_state_ram->feature_info.car_id = fob_state_ram->pair_info.car_id;

    uart_write(HOST_UART, (uint8_t *)"Paired", 6);

    saveFobState(fob_state_ram);
  }
}

/**
 * @brief Function that handles enabling a new feature on the fob
 *
 * @param fob_state_ram pointer to the current fob state in ram
 */
void enableFeature(FLASH_DATA *fob_state_ram) {
  debug_print("\r\n\n---- Enable Feature ----\n");
  if (fob_state_ram->paired == FLASH_PAIRED) {
    uint8_t uart_buffer[256];
    uart_readline(HOST_UART, uart_buffer);

    uint8_t decoded_buffer[128];

    hydro_hex2bin(decoded_buffer, 128, uart_buffer, 138, 0, 0);

    ENABLE_PACKET *enable_message = (ENABLE_PACKET *)decoded_buffer;

    // If feature is intended for a different car, exit
    if (fob_state_ram->pair_info.car_id != enable_message->car_id) {
      return;
    }

    // If feature list full, exit
    if (fob_state_ram->feature_info.num_active == NUM_FEATURES) {
      return;
    }

    // If feature already enabled, exit
    for (int i = 0; i < fob_state_ram->feature_info.num_active; i++) {
      if (fob_state_ram->feature_info.features[i] == enable_message->feature) {
        return;
      }
    }

    // If feature signature invalid, exit
    if (hydro_sign_verify(enable_message->signature, enable_message,
                          sizeof(enable_message->car_id) +
                              sizeof(enable_message->feature),
                          "feature", feature_verification_key) != 0) {
      debug_print("\r\nERROR: Feature verification failed.");
      return;
    }

    // Set feature enabled, store signature (to be verified by car)
    fob_state_ram->feature_info
        .features[fob_state_ram->feature_info.num_active] =
        enable_message->feature;

    memcpy(fob_state_ram->feature_info
               .signatures[fob_state_ram->feature_info.num_active],
           enable_message->signature, hydro_sign_BYTES);

    fob_state_ram->feature_info.num_active++;

    saveFobState(fob_state_ram);
    uart_write(HOST_UART, (uint8_t *)"Enabled", 7);
  }
}

/**
 * @brief Function implementing simple handshake between fob and car. Returns
 * nonce to be used when processing unlock packet.
 */
uint32_t performHandshake(void) {
  debug_print("\r\nPerforming Handshake");
  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  message.buffer = buffer;

  debug_print("\r\nSending handshake request");

  message.magic = HANDSHAKE_MAGIC;
  message.message_len = 0;

  send_board_message(&message);

  debug_print("\r\nWaiting for response packet");

  receive_board_message_by_type(&message, HANDSHAKE_MAGIC);

  debug_print("\r\nHandshake response received");

  uint32_t nonce;

  memcpy(&nonce, &(message.buffer[0]), 4);

  return nonce;
}

/**
 * @brief Function that handles the fob unlocking a car
 *
 * @param fob_state_ram pointer to the current fob state in ram
 */
void unlockCar(FLASH_DATA *fob_state_ram) {
  debug_print("\r\n\n---- Begin Unlock ----\n");
  if (fob_state_ram->paired == FLASH_PAIRED) {
    MESSAGE_PACKET message;
    uint8_t buffer[256];
    message.buffer = buffer;

    uint32_t nonce = performHandshake();

    debug_print("\r\n\n---- Send Unlock ----\n");

    memcpy(&(message.buffer[0]), &nonce, 4);

    debug_print("\r\nSending unlock message");

    message.message_len = 4;
    message.magic = UNLOCK_MAGIC;

    send_board_message(&message);
  }
}

/**
 * @brief Function that handles the fob starting a car
 *
 * @param fob_state_ram pointer to the current fob state in ram
 */
void startCar(FLASH_DATA *fob_state_ram) {
  debug_print("\r\n\n---- Start ----\n");
  if (fob_state_ram->paired == FLASH_PAIRED) {
    MESSAGE_PACKET message;
    message.magic = START_MAGIC;
    message.message_len = sizeof(FEATURE_DATA);
    message.buffer = (uint8_t *)&fob_state_ram->feature_info;
    send_board_message(&message);
  }
}

/**
 * @brief Function that erases and rewrites the non-volatile data to flash
 *
 * @param info Pointer to the flash data ram
 */
void saveFobState(FLASH_DATA *flash_data) {
  FlashErase(FOB_STATE_PTR);
  FlashProgram((uint32_t *)flash_data, FOB_STATE_PTR, FLASH_DATA_SIZE);
}

/**
 * @brief Function that receives an ack and returns whether ack was
 * success/failure
 *
 * @return uint8_t Ack success/failure
 */
uint8_t receiveAck() {
  MESSAGE_PACKET message;
  uint8_t buffer[255];
  message.buffer = buffer;
  receive_board_message_by_type(&message, ACK_MAGIC);

  debug_print("\r\nReceiving ACK");

  return message.buffer[0];
}

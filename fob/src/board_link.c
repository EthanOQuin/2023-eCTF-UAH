/**
 * @file board_link.h
 * @author Frederich Stine
 * @brief Firmware UART interface implementation.
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

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "board_link.h"
#include "debug.h"

#include "hydrogen.h"

extern uint8_t *message_key;

/**
 * @brief Set the up board link object
 *
 * UART 1 is used to communicate between boards
 */
void setup_board_link(void) {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  GPIOPinConfigure(GPIO_PB0_U1RX);
  GPIOPinConfigure(GPIO_PB1_U1TX);

  GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(
      BOARD_UART, SysCtlClockGet(), 115200,
      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

  while (UARTCharsAvail(BOARD_UART)) {
    UARTCharGet(BOARD_UART);
  }
}

/**
 * @brief Send an encrypted message between boards
 *
 * @param message pointer to message to send
 * @return uint32_t the number of bytes sent
 */
uint32_t send_board_message(MESSAGE_PACKET *message) {
  debug_print("\r\nSending board message");

  uint8_t buffer[256];
  hydro_bin2hex(buffer, 256, message_key, hydro_secretbox_KEYBYTES);
  debug_print("\r\n");
  debug_print(buffer);
  debug_print("\r\n");

  UARTCharPut(BOARD_UART, message->magic);
  UARTCharPut(BOARD_UART, message->message_len);

  // If message is a pairing packet, send unencrypted. Otherwise, encrypt
  // message.
  if (message->magic == PAIR_MAGIC) {
    debug_print("\r\nSending unencrypted pairing message");

    for (int i = 0; i < message->message_len; i++) {
      UARTCharPut(BOARD_UART, message->buffer[i]);
    }

    return message->message_len;
  } else {
    const char context[] = "boardmsg";
    uint8_t ciphertext[hydro_secretbox_HEADERBYTES + MESSAGE_MAX_LENGTH];
    uint32_t ciphertext_len =
        hydro_secretbox_HEADERBYTES + message->message_len;

    debug_print("\r\nEncrypting message contents");

    hydro_secretbox_encrypt(ciphertext, message->buffer, message->message_len,
                            0, context, message_key);

    for (int i = 0; i < ciphertext_len; i++) {
      UARTCharPut(BOARD_UART, ciphertext[i]);
    }

    debug_print("\r\nMessage sent");

    return ciphertext_len;
  }
}

/**
 * @brief Receive an encrypted message between boards
 *
 * @param message pointer to message where data will be received
 * @return uint32_t the number of bytes received - 0 for parsing erorr, -1 for
 * corrupted or tampered message
 */
uint32_t receive_board_message(MESSAGE_PACKET *message) {
  message->magic = (uint8_t)UARTCharGet(BOARD_UART);

  if (message->magic == 0) {
    return 0;
  }

  message->message_len = (uint8_t)UARTCharGet(BOARD_UART);

  if (message->magic == PAIR_MAGIC) {
    debug_print("\r\nReceiving unencrypted pairing message");

    for (int i = 0; i < message->message_len; i++) {
      message->buffer[i] = (uint8_t)UARTCharGet(BOARD_UART);
    }
  } else {
    const char context[] = "boardmsg";
    uint8_t ciphertext[hydro_secretbox_HEADERBYTES + MESSAGE_MAX_LENGTH];

    uint32_t ciphertext_len =
        hydro_secretbox_HEADERBYTES + message->message_len;

    for (int i = 0; i < ciphertext_len; i++) {
      ciphertext[i] = (uint8_t)UARTCharGet(BOARD_UART);
    }

    debug_print("\r\nDecrypting board message");

    if (hydro_secretbox_decrypt(message->buffer, ciphertext, ciphertext_len, 0,
                                context, message_key)) {
      debug_print("\r\nERROR: Invalid message received");
      return -1;
    }

    debug_print("\r\nMessage received");
  }

  return message->message_len;
}

/**
 * @brief Function that retreives messages until the specified message is found
 *
 * @param message pointer to message where data will be received
 * @param type the type of message to receive
 * @return uint32_t the number of bytes received
 */
uint32_t receive_board_message_by_type(MESSAGE_PACKET *message, uint8_t type) {
  do {
    receive_board_message(message);
    debug_print("\r\nReceived msg with magic: 0x");
    char magic[8];
    hydro_bin2hex(magic, 3, &(message->magic), 1);
    debug_print(magic);
  } while (message->magic != type);

  return message->message_len;
}

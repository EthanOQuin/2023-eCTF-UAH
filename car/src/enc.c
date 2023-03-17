#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "hydrogen.h"

#include "enc.h"
#include "uart.h"

/* typedef struct { */
/*   uint32_t magic; */
/*   uint32_t seq_num; */
/*   uint32_t random_pad; */
/*   uint32_t random_pad; */
/* } ENC_MESSAGE; */

/* bool hydro_random_rbit(uint16_t x) { */
/*   uint8_t x8; */

/*   x8 = ((uint8_t)(x >> 8)) ^ (uint8_t)x; */
/*   x8 = (x8 >> 4) ^ (x8 & 0xf); */
/*   x8 = (x8 >> 2) ^ (x8 & 0x3); */
/*   x8 = (x8 >> 1) ^ x8; */

/*   return (bool)(x8 & 1); */
/* } */

/* void tiva_adc_init(void) { */
/*   // Enable the ADC0 module. */
/*   SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); */

/*   // Wait for the ADC0 module to be ready. */
/*   while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) { */
/*   } */

/*   // Enable the first sample sequencer to capture the value of channel 0 when
 */
/*   // the processor trigger occurs. */
/*   ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0); */
/*   ADCSequenceStepConfigure(ADC0_BASE, 0, 0, */
/*                            ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0); */
/*   ADCSequenceEnable(ADC0_BASE, 0); */
/* } */

/* bool tiva_adc_rbit(void) { */
/*   uint32_t adc_sample; */

/*   // Trigger the sample sequence. */
/*   ADCProcessorTrigger(ADC0_BASE, 0); */

/*   // Wait until the sample sequence has completed. */
/*   while (!ADCIntStatus(ADC0_BASE, 0, false)) { */
/*   } */

/*   // Read the value from the ADC. */
/*   ADCSequenceDataGet(ADC0_BASE, 0, &adc_sample); */

/*   return hydro_random_rbit((uint16_t)adc_sample); */
/* } */

void crypto_test(void) {
  uint8_t debug1[] = "\nGenerating random number\n";
  uart_write(HOST_UART, debug1, sizeof(debug1));

  uint32_t r_uint32 = hydro_random_u32();

  uint8_t debug2[] = "\nGenerating random buffer\n";
  uart_write(HOST_UART, debug2, sizeof(debug2));

  uint8_t buf[32];
  hydro_random_buf(buf, sizeof buf);
}

void crypto_test2(void) {
  char context[] = "Examples";
  char message[] = "test";
  uint32_t message_len = 4;
  uint32_t ciphertext_len = (hydro_secretbox_HEADERBYTES + message_len);

  uint8_t key[hydro_secretbox_KEYBYTES];
  uint8_t ciphertext[ciphertext_len];

  uint8_t debug1[] = "\nGenerating key\n";
  uart_write(HOST_UART, debug1, sizeof(debug1));
  hydro_secretbox_keygen(key);

  uint8_t debug2[] = "\nEncrypting message\n";
  uart_write(HOST_UART, debug2, sizeof(debug2));
  hydro_secretbox_encrypt(ciphertext, message, message_len, 0, context, key);

  uint8_t debug3[] = "\nDecrypting message\n";
  uart_write(HOST_UART, debug3, sizeof(debug3));
  char decrypted[message_len];
  hydro_secretbox_decrypt(decrypted, ciphertext, ciphertext_len, 0, context,
                          key);

  uint8_t debug4[] = "\r\nKey: ";
  uart_write(HOST_UART, debug4, sizeof(debug4));

  uint8_t key_hex[256];
  hydro_bin2hex(key_hex, 256, key, sizeof(key));
  uart_write(HOST_UART, (uint8_t *)key_hex, strlen(key_hex));

  uint8_t debug5[] = "\r\nEncrypted message: ";
  uart_write(HOST_UART, debug5, sizeof(debug5));

  uint8_t ciphertext_hex[256];
  hydro_bin2hex(ciphertext_hex, 256, ciphertext, sizeof(ciphertext));
  uart_write(HOST_UART, (uint8_t *)ciphertext_hex, strlen(ciphertext_hex));

  uint8_t debug6[] = "\r\nDecrypted message: ";
  uart_write(HOST_UART, debug6, sizeof(debug6));
  uart_write(HOST_UART, (uint8_t *)decrypted, sizeof(decrypted));
}

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "hydrogen.h"

#include "debug.h"
#include "enc.h"
#include "uart.h"

void crypto_test(void) {
  char context[] = "Examples";
  char message[] = "test";
  uint32_t message_len = 4;
  uint32_t ciphertext_len = (hydro_secretbox_HEADERBYTES + message_len);

  uint8_t key[hydro_secretbox_KEYBYTES];
  uint8_t ciphertext[ciphertext_len];

  hydro_init();

  debug_print("\r\n\nGenerating key\n");
  hydro_secretbox_keygen(key);

  debug_print("\r\nEncrypting message\n");
  hydro_secretbox_encrypt(ciphertext, message, message_len, 0, context, key);

  debug_print("\r\nDecrypting message\n");
  char decrypted[message_len];
  if (hydro_secretbox_decrypt(decrypted, ciphertext, ciphertext_len, 0, context,
                              key) != 0) {
    debug_print("\r\nDecryption failed\n");
  }

  debug_print("\r\nKey: ");

  char key_hex[256];
  hydro_bin2hex(key_hex, 256, key, sizeof(key));
  debug_print(key_hex);

  debug_print("\r\nEncrypted message: ");

  char ciphertext_hex[256];
  hydro_bin2hex(ciphertext_hex, 256, ciphertext, sizeof(ciphertext));
  debug_print(ciphertext_hex);

  debug_print("\r\nDecrypted message: ");
  debug_print(decrypted);

  uint8_t message_hash[hydro_hash_BYTES];
  hydro_hash_hash(message_hash, sizeof(message_hash), message, strlen(message),
                  "Test", NULL);

  debug_print("\r\nMessage hash: ");

  char hash_hex[257];
  hydro_bin2hex(hash_hex, 257, message_hash, sizeof(message_hash));
  debug_print(hash_hex);

  debug_print("\r\n\n");
}

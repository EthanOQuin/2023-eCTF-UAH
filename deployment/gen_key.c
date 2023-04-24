#include <stdint.h>
#include <stdio.h>

#include "./lib/libhydrogen/hydrogen.h"

// Generate a shared secret key for car and paired fob
int main(void) {
  char context[] = "genkey";
  uint8_t shared_key[hydro_secretbox_KEYBYTES];

  hydro_init();

  hydro_secretbox_keygen(shared_key);

  for (int i = 0; i < 32; i++) {
    printf("0x%x,", shared_key[i]);
  }
}

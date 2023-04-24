#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lib/libhydrogen/hydrogen.h"

/* typedef struct { */
/* } __attribute__((packed)) FEATURE_PACKAGE; */

typedef struct {
  uint32_t car_id;
  uint8_t feature_num;
  uint8_t signature[hydro_sign_BYTES];
} __attribute__((packed)) SIGNED_FEATURE_PACKAGE;

int main(int argc, char **argv) {
  // Check args
  if (argc < 3) {
    fprintf(stderr, "ERROR: Must provide public key filename, and feature "
                    "package filename.\n");
    return 1;
  }

  // Initialize libhydrogen
  char context[] = "feature";
  uint8_t public_key[hydro_sign_PUBLICKEYBYTES];
  hydro_init();

  // Load signing public key
  FILE *public_key_file = fopen(argv[1], "r");
  char input_buffer[1024];
  fgets(input_buffer, 1024, public_key_file);
  hydro_hex2bin(public_key, hydro_sign_PUBLICKEYBYTES, input_buffer,
                hydro_sign_PUBLICKEYBYTES * 2, 0, 0);

  // Read input from file
  FILE *input_file = fopen(argv[2], "rb");

  SIGNED_FEATURE_PACKAGE s;
  fread(&s, sizeof(SIGNED_FEATURE_PACKAGE), 1, input_file);

  if (hydro_sign_verify(s.signature, &s,
                        sizeof(s.car_id) + sizeof(s.feature_num), context,
                        public_key) != 0) {
    printf("ERROR: Feature verification failed.\n");
    return 1;
  }
}

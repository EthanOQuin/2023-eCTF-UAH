#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lib/libhydrogen/hydrogen.h"

typedef struct {
  uint64_t car_id;
  uint8_t feature_num;
} __attribute__((packed)) FEATURE_PACKAGE;

typedef struct {
  FEATURE_PACKAGE p;
  uint8_t signature[hydro_sign_BYTES];
} __attribute__((packed)) SIGNED_FEATURE_PACKAGE;

// Sign a feature package using the provided private key.
//
// First arg is car ID,
// second is feature number,
// third is secret key filename,
// fourth is output package filename.
int main(int argc, char **argv) {
  // Check args
  if (argc < 4) {
    fprintf(stderr, "ERROR: Must provide car ID, feature number, secret key "
                    "filename, and output package filename.\n");
    return 1;
  }

  // Initialize libhydrogen
  char context[] = "signing";
  hydro_sign_keypair feature_authentication_keypair;
  hydro_init();

  // Load signing secret key
  FILE *secret_key_file = fopen(argv[3], "r");
  char input_buffer[1024];
  fgets(input_buffer, 1024, secret_key_file);
  hydro_hex2bin(feature_authentication_keypair.sk, hydro_sign_SECRETKEYBYTES,
                input_buffer, strlen(input_buffer) - 1, 0, 0);

  // Initialize feature package
  SIGNED_FEATURE_PACKAGE signed_feature;
  signed_feature.p.car_id = strtoull(argv[1], 0, 10);
  signed_feature.p.feature_num = strtol(argv[2], 0, 10) & 0xFF;

  // Create signature
  hydro_sign_create(signed_feature.signature, &signed_feature.p,
                    sizeof(FEATURE_PACKAGE), context,
                    feature_authentication_keypair.sk);

  // Output to file
  FILE *output_file = fopen(argv[4], "wb");
  fwrite(&signed_feature, sizeof(SIGNED_FEATURE_PACKAGE), 1, output_file);
  fclose(output_file);
}

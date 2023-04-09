#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "./lib/libhydrogen/hydrogen.h"

// Sign a feature packet using the provided private key
// First arg is secret key, second is feature number, third is car id
int main(int argc, char **argv) {
  char context[] = "genkey";
  hydro_sign_keypair feature_authentication_keypair;

  hydro_init();

  hydro_hex2bin(feature_authentication_keypair.sk, hydro_sign_SECRETKEYBYTES,
                argv[1], strlen(argv[1]), 0, 0);

  uint64_t feature_packet[2];

  feature_packet[0] = strtoull(argv[2], 0, 10);
  feature_packet[1] = strtoull(argv[3], 0, 10);

  uint8_t signature[hydro_sign_BYTES];

  hydro_sign_create(signature, feature_packet, sizeof(feature_packet), context,
                    feature_authentication_keypair.sk);

  char output[512];

  hydro_bin2hex(output, 512, signature, hydro_sign_BYTES);

  printf("%s\n", output);
}

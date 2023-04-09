#include <stdint.h>
#include <stdio.h>

#include "./lib/libhydrogen/hydrogen.h"

// Generate a public/secret key pair for feature package signing and
// authentication
// The secret key is used when signing features during packaging,
// while the public key is held by the car and used to verify feature
// authenticity.

int main(void) {
  char context[] = "genkey";
  hydro_sign_keypair feature_authentication_keypair;

  hydro_init();

  hydro_sign_keygen(&feature_authentication_keypair);

  char secretkey[500];

  hydro_bin2hex(secretkey, 500, feature_authentication_keypair.sk,
                hydro_sign_SECRETKEYBYTES);

  printf("%s", secretkey);

  printf("\n\n");

  for (int i = 0; i < hydro_sign_PUBLICKEYBYTES; i++) {
    printf("0x%x,", feature_authentication_keypair.pk[i]);
  }

  printf("\n");
}

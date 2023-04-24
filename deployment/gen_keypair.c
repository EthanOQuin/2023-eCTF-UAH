#include <stdint.h>
#include <stdio.h>

#include "./lib/libhydrogen/hydrogen.h"

// Generate a public/secret key pair for feature package signing and
// authentication.
//
// The secret key is used when signing features during
// packaging, while the public key is held by the car and used to verify feature
// authenticity.
//
// Secret key saved as plain hex string, public key split bytewise
// in "0xXX, " format for use in C headers.

int main(int argc, char **argv) {
  // Check args
  if (argc < 3) {
    fprintf(stderr, "ERROR: Must provide output filenames for secret key and "
                    "public key.\n");
    return 1;
  }

  // Open files
  FILE *secret_key_file = fopen(argv[1], "w");
  FILE *public_key_file = fopen(argv[2], "w");

  // Initialize libhydrogen
  char context[] = "genkey";
  hydro_sign_keypair feature_authentication_keypair;
  hydro_init();

  // Generate a key pair for signing
  hydro_sign_keygen(&feature_authentication_keypair);

  // Save secret key to file
  char secret_key_str[1024];
  hydro_bin2hex(secret_key_str, 1024, feature_authentication_keypair.sk,
                hydro_sign_SECRETKEYBYTES);
  fprintf(secret_key_file, "%s", secret_key_str);

  char public_key_str[1024];
  hydro_bin2hex(public_key_str, 1024, feature_authentication_keypair.pk,
                hydro_sign_PUBLICKEYBYTES);
  fprintf(public_key_file, "%s", public_key_str);

  // Save public key to file
  /* for (int i = 0; i < hydro_sign_PUBLICKEYBYTES; i++) { */
  /*   fprintf(public_key_file, "0x%x,", feature_authentication_keypair.pk[i]);
   */
  /* } */

  // Close files
  fclose(secret_key_file);
  fclose(public_key_file);
}

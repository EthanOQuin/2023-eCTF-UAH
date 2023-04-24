#!/usr/bin/python3 -u

# @file gen_secret
# @author Jake Grycel
# @brief Example script to generate header containing secrets for the car
# @date 2023
#
# This source file is part of an example system for MITRE's 2023 Embedded CTF (eCTF).
# This code is being provided only for educational purposes for the 2023 MITRE eCTF
# competition,and may not meet MITRE standards for quality. Use this code at your
# own risk!
#
# @copyright Copyright (c) 2023 The MITRE Corporation

import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--car-id", type=int, required=True)
    parser.add_argument("--secret-key-file", type=Path, required=True)
    parser.add_argument("--signing-public-key-file", type=Path)
    parser.add_argument("--header-file", type=Path, required=True)
    args = parser.parse_args()

    # Open the secret file if it exists
    if args.secret_key_file.exists():
        with open(args.secret_key_file, "r") as f:
            secret_key = f.readline()
    else:
        raise RuntimeError

    if args.signing_public_key_file.exists():
        with open(args.signing_public_key_file, "r") as f:
            signing_public_key = f.readline()
    else:
        raise RuntimeError

    # Write to header file
    with open(args.header_file, "w") as f:
        f.write("#ifndef __CAR_SECRETS__\n")
        f.write("#define __CAR_SECRETS__\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include \"hydrogen.h\"\n\n")
        f.write(f'#define CAR_ID {args.car_id}\n\n')
        f.write(f'const uint8_t MESSAGE_KEY[hydro_secretbox_KEYBYTES] = {{{secret_key}}};\n\n')
        f.write(f'const uint8_t SIGNING_PUBLIC_KEY[hydro_sign_PUBLICKEYBYTES] = {{{signing_public_key}}};\n\n')
        f.write("#endif\n")


if __name__ == "__main__":
    main()

#!/usr/bin/python3 -u

# @file gen_secret
# @author Jake Grycel
# @brief Example script to generate header containing secrets for the fob
# @date 2023
#
# This source file is part of an example system for MITRE's 2023 Embedded CTF (eCTF).
# This code is being provided only for educational purposes for the 2023 MITRE eCTF
# competition, and may not meet MITRE standards for quality. Use this code at your
# own risk!
#
# @copyright Copyright (c) 2023 The MITRE Corporation

import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--car-id", type=int)
    parser.add_argument("--pair-pin", type=str)
    parser.add_argument("--secret-key-file", type=Path)
    parser.add_argument("--signing-public-key-file", type=Path)
    parser.add_argument("--header-file", type=Path)
    parser.add_argument("--paired", action="store_true")
    args = parser.parse_args()

    if args.signing_public_key_file.exists():
        with open(args.signing_public_key_file, "r") as f:
            signing_public_key = f.readline()
    else:
        raise RuntimeError

    if args.paired:
        # Open the secret file, get the car's secret
        with open(args.secret_key_file, "r") as f:
            secret_key = f.readline()

        # Write to header file
        with open(args.header_file, "w") as f:
            f.write("#ifndef __FOB_SECRETS__\n")
            f.write("#define __FOB_SECRETS__\n\n")
            f.write("#define PAIRED 1\n")
            f.write(f'#define PAIR_PIN "{args.pair_pin}"\n')
            f.write(f'#define CAR_ID {args.car_id}\n\n')
            f.write(f'const uint8_t MESSAGE_KEY[hydro_secretbox_KEYBYTES] = {{{secret_key}}};\n\n')
            f.write(f'const uint8_t SIGNING_PUBLIC_KEY[hydro_sign_PUBLICKEYBYTES] = {{{signing_public_key}}};\n\n')
            f.write("#endif\n")
    else:
        # Write to header file
        with open(args.header_file, "w") as f:
            f.write("#ifndef __FOB_SECRETS__\n")
            f.write("#define __FOB_SECRETS__\n\n")
            f.write("#include <stdint.h>\n")
            f.write("#include \"hydrogen.h\"\n")
            f.write("#define PAIRED 0\n")
            f.write('#define PAIR_PIN "000000"\n')
            f.write('#define CAR_ID 0\n\n')
            f.write('const uint8_t MESSAGE_KEY[hydro_secretbox_KEYBYTES] = {0x00};\n\n')
            f.write(f'const uint8_t SIGNING_PUBLIC_KEY[hydro_sign_PUBLICKEYBYTES] = {{{signing_public_key}}};\n\n')
            f.write("#endif\n")


if __name__ == "__main__":
    main()

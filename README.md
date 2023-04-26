# UAH 496 Senior Design Project: 2023 MITRE eCTF
This repository contains our entry into the MITRE Embedded Systems Capture the Flag competition (see https://ectf.mitre.org/ for more details).

The code is intended to be run on a Texas Instruments Tiva-C Microcontroller (TM4C123GH6PM) flashed with the bootloader provided by MITRE.

Our design is based off of the reference design in C provided by the competition organizers, which was used as a scaffold for our own design, however we reimplemented the main features, taking security into account. A brief overview of the security features we developed is as follows:

- Communication between the car and the key fob is encrypted using libhydrogen's secretbox functionality, which is based on the Gimli permutation and Curve25519, authenticating and validating the connection.
- The unlock & start sequence requires performing a handshake to validate the connection before the commands to unlock and start the car are accepted. The handshake involves generating a random nonce which must be used to generate the unlock command. The nonce is valid for only one unlock attempt and only when specifically requested by the car, thereby preventing replay attacks.
- The feature packaging and enablement procedure generates and appends a cryptographic signature generated using the manufacturer's secret key. This signature is checked on the fob and car in order to ensure the authenticity and integrity of the feature enablement packets.
- The pairing sequence has been expanded to take into account the additional data that has to be transferred for the communication encryption key and the feature data. 

## Design Structure
- `car` - source code for building car devices
- `deployment` - source code for generating deployment-wide secrets
- `docker_env` - source code for creating docker build environment
- `fob` - source code for building key fob devices
- `host_tools` - source code for the host tools
- `scripts` - useful scripts for automating the build process and other common tasks

# Building and Installation
## Prereqs
You should be running on a Linux system (or in WSL) on a user with access to docker without requiring sudo. The system should also have Python 3.8 or newer.
 
The code is intended to be run on a Texas Instruments Tiva-C Microcontroller (TM4C123GH6PM) flashed with the bootloader provided by MITRE. You should have two boards, both connected to the host machine over USB and with their UART 1 pins tied together with jumper wires (see 2023 competition docs for more details).

The provided convenience scripts assume that the boards you are using will be available at `/dev/board1` and `/dev/board2`. This can be accomplished by adding a udev rule into a .conf file in `/etc/udev/rules.d/` with lines in the following format, where `<SERIALNUM>` is replaced with the boards' serial numbers as reported by `lshw`:
```
ACTION=="add", ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="00fd", ATTRS{serial}=="<SERIALNUM>", SYMLINK+="board1"
ACTION=="add", ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="00fd", ATTRS{serial}=="<SERIALNUM>", SYMLINK+="board2"
```


## Preperation 
First, clone the tools repository.

```bash
git clone https://github.com/mitre-cyber-academy/2023-ectf-tools.git
```

Follow the steps described in that repository's README to install and configure the host tools. This requires having Docker configured to allow starting the environment required for running the commands. Docker commands must be able to run as your local user (by adding your user to the `docker` group), although this is generally considered to be as insecure as it effectively grants your account paswordless root access. Ensure that the tools are functioning before proceeding. 

Follow the steps described there to flash the provided bootloader files using TI's Uniflash utility.

## Building our Code
Clone this repository:

```bash
git clone https://github.com/EthanOQuin/2023-eCTF-UAH --recurse-submodules
```

`cd` to the root of the repository, and run `./scripts/build.sh` to build the code, or follow the steps described in the MITRE 2023-ectf-tools repository.

# Loading the Firmware
On a board with the provided bootloader already flashed, use either the `./scripts/load_car_and_paired_fob.sh` script to load the firmware files for a paired key fob and a car onto a pair of boards that have been put into bootloader mode (see tools repository for more details). Additional scripts are provided to automate loading different combinations of the firmware files on to the boards. 
See the instructions in the linked tools repository for more details, including how to perform these steps manually. 

# Usage
Once you have loaded a car and fob loaded and in the correct hardware configuration (UART 1 connected between the boards), the car unlock sequence can be triggered by pressing SW1 on the key fob, which is the board with the white status LED. The car should unlock and start, with its status LED changing from red to green in the process.

The following scripts require running the `./run_bridges_boards_1_2.sh` script to create the tunnel required for allowing the UART communication to be tunneled into the tools' docker container.

To package and enable a feature, use the `./scripts/package_and_enable_feat.sh` script. To pair an unpaired key fob, use the `./scripts/pair_fob.sh` script. See the 2023-ectf-tools repository for more information on how to perform these operations manually. 

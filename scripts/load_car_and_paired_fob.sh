#!/usr/bin/env bash

# Load Car FW
python3 -m ectf_tools device.load_hw --dev-in ./outputs/test_car --dev-name car --dev-serial /dev/board1 &

# Load Paired Fob FW
python3 -m ectf_tools device.load_hw --dev-in ./outputs/test_paired_fob --dev-name paired_fob --dev-serial /dev/board2 &

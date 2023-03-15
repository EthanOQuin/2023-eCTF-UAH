#!/usr/bin/bash

python -m ectf_tools device.bridge --bridge-id 2000 --dev-serial /dev/board1 &
python -m ectf_tools device.bridge --bridge-id 2001 --dev-serial /dev/board2 &

trap 'kill $(jobs -pr)' SIGINT SIGTERM EXIT

sleep 100000000

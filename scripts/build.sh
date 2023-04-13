#!/usr/bin/env bash

# Build Docker Container
python3 -m ectf_tools build.env --design ../2023-eCTF-UAH --name our_design

# Build Tools
python3 -m ectf_tools build.tools --design ../2023-eCTF-UAH --name our_design

# Build Deployment
python3 -m ectf_tools build.depl --design ../2023-eCTF-UAH --name our_design --deployment test_deployment

# Build Car and Paired Fob
python3 -m ectf_tools build.car_fob_pair --design ../2023-eCTF-UAH --name our_design --deployment test_deployment --car-out ./outputs/test_car --fob-out ./outputs/test_paired_fob --car-name car --fob-name paired_fob --car-id 1000 --pair-pin 001234 --car-feature1-secret "TEST"

# Build Unpaired Fob
python3 -m ectf_tools build.fob --design ../2023-eCTF-UAH --name our_design --deployment test_deployment --fob-out ./outputs/test_unpaired_fob --fob-name unpaired_fob

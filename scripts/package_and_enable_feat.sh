#!/usr/bin/env sh

# Package features
python3 -m ectf_tools run.package --name test_design --deployment test_deployment --package-out ./outputs --package-name feature1.bin --car-id 1000 --feature-number 1
python3 -m ectf_tools run.package --name test_design --deployment test_deployment --package-out ./outputs --package-name feature2.bin --car-id 1000 --feature-number 2
python3 -m ectf_tools run.package --name test_design --deployment test_deployment --package-out ./outputs --package-name feature3.bin --car-id 1000 --feature-number 3

# Enable features
python3 -m ectf_tools run.enable --name test_design --fob-bridge 2000 --package-in ./outputs --package-name feature1.bin
python3 -m ectf_tools run.enable --name test_design --fob-bridge 2000 --package-in ./outputs --package-name feature2.bin
python3 -m ectf_tools run.enable --name test_design --fob-bridge 2000 --package-in ./outputs --package-name feature3.bin

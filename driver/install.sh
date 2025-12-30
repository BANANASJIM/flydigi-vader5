#!/bin/bash
set -e

cd "$(dirname "$0")"

echo "Building driver..."
make

echo "Unloading old driver..."
sudo rmmod hid-flydigi 2>/dev/null || true

echo "Loading new driver..."
sudo insmod hid-flydigi.ko

echo "Checking logs..."
sudo dmesg | grep -i flydigi | tail -5

echo "Done. Run 'evtest' to test input."
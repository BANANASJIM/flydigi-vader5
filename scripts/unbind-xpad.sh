#!/bin/bash
# Unbind xpad driver from Vader 5 Pro to allow hidraw access

for dev in /sys/bus/usb/drivers/xpad/*/; do
    if [[ -d "$dev" ]]; then
        devname=$(basename "$dev")
        if [[ "$devname" =~ ^[0-9]+-[0-9]+ ]]; then
            echo "Unbinding $devname from xpad"
            echo "$devname" > /sys/bus/usb/drivers/xpad/unbind 2>/dev/null
        fi
    fi
done

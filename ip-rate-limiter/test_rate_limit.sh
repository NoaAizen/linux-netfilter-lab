#!/bin/bash

MODULE_NAME="netfilter_module"
PING_COUNT=200  # Total number of pings to simulate traffic
TARGET_IP="127.0.0.1"  # Replace with a test IP (localhost or another machine)

echo "Testing rate-limiting kernel module..."

# Step 1: Load the kernel module
sudo insmod $MODULE_NAME.ko
if [ $? -ne 0 ]; then
    echo "Failed to load kernel module!"
    exit 1
fi
echo "Kernel module loaded."

# Step 2: Generate network traffic using a for loop to send ping requests quickly
echo "Generating traffic using a for loop to send pings rapidly..."
for i in $(seq 1 $PING_COUNT); do
    ping -c 1 -q $TARGET_IP > /dev/null
done

# Optionally, you can use `hping3` to generate more customizable traffic, like:
# sudo hping3 -c $PING_COUNT -i u1000 -S -p 80 $TARGET_IP > /dev/null

# Step 3: Check kernel logs for rate-limiting messages (requires sudo)
echo "Checking kernel logs for dropped packets..."
sudo dmesg | tail -n 20 | grep "Rate limiting:"

# Step 4: Unload the kernel module
echo "Unloading kernel module..."
sudo rmmod $MODULE_NAME
if [ $? -ne 0 ]; then
    echo "Failed to unload kernel module!"
    exit 1
fi
echo "Kernel module unloaded."

# Step 5: Show the final kernel logs (requires sudo)
echo "Final kernel logs:"
sudo dmesg | tail -n 20 | grep "Rate limiting:"

#!/bin/bash

PORT="/dev/ttyUSB0"
BAUD=115200

stty -F "$PORT" "$BAUD" cs8 -cstopb -parenb -echo

# Hold port open with a file descriptor
exec 3<>"$PORT"

sleep 2

TIME_STR=$(date +"SET %Y %m %d %H %M %S %u")
echo "Sending: $TIME_STR"
echo "$TIME_STR" >&3

# Read response
read -t 3 RESPONSE <&3
echo "Response: $RESPONSE"

# Close file descriptor
exec 3>&-

/*
#!/bin/bash
# On host computer run this script with correct PORT after flashing this to desired device

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
*/

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    if (!rtc.begin()) {
        Serial.println("ERROR: DS3231 not found");
        while (1) delay(10);
    }

    Serial.println("READY");
}

void loop() {
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.startsWith("SET")) {
            int year, month, day, hour, minute, second, dow;
            int parsed = sscanf(line.c_str(), "SET %d %d %d %d %d %d %d",
                                &year, &month, &day, &hour, &minute, &second, &dow);

            if (parsed >= 6) {
                rtc.adjust(DateTime(year, month, day, hour, minute, second));
                Serial.print("OK: RTC set to ");
                DateTime now = rtc.now();
                Serial.print(now.day()); Serial.print("-");
                Serial.print(now.month()); Serial.print("-");
                Serial.print(now.year()); Serial.print(" ");
                Serial.print(now.hour()); Serial.print(":");
                Serial.print(now.minute()); Serial.print(":");
                Serial.println(now.second());
            } else {
                Serial.println("ERROR: Parse failed");
            }
        }
    }

    // Optional: periodically print current RTC time
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 5000) {
        lastPrint = millis();
        DateTime now = rtc.now();
        Serial.print("RTC: ");
        Serial.print(now.year()); Serial.print("-");
        Serial.print(now.month()); Serial.print("-");
        Serial.print(now.day()); Serial.print(" ");
        Serial.print(now.hour()); Serial.print(":");
        Serial.print(now.minute()); Serial.print(":");
        Serial.println(now.second());
    }
}

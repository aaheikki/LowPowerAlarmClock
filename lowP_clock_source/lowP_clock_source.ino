#include <RTClib.h>


RTC_DS3231 rtc; // define rtc object

#define CLOCK_INTERRUPT_PIN 2

volatile bool CLOCK_INT_ALARM = false;

void onClock_Interrupt(void) { //Interrupt to change flag -> main work in loop()
    clock_flag = true; 
}

void setup() {
  Serial.begin(115200);

  //Setup RTC
  if(!rtc.begin()){
    Serial.println("Could not find RTC!");
  }
  if(rtc.lostPower()) {
    Serial.println("RTC lost power, set time!")
  }
  rtc.disable32K();                         // Disable 32K pin
  rtc.clearAlarm(1);rtc.clearAlarm(2);      // Clear and disable alarms 1 and 2, since they are not reset with MCU board
  rtc.disableAlarm(1); rtc.disableAlarm(2);
  rtc.writeSqwPinMode(DS3231_OFF);          // Stop INT/SQW pin
  
  if(!rtc.setAlarm1(rtc.now() + TimeSpan(10),DS3231_A1_Second)) { // schedule an alarm 10 seconds in the future
      Serial.println("Error, alarm wasn't set!");
  }
}


  //Interrut pins
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP)
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onClock_Interrupt, FALLING);
}

void loop() {
  char time[10] = "hh:mm:ss"; // Initialize time char
  
  // print current time if x time has passed
  if(clock_flag){  
    clock_flag = false;
    rtc.now().toString(time);
    Serial.print(time);  
  }
  

}
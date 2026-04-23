#include <RTClib.h>

RTC_DS3231 rtc; // define rtc object

#define CLOCK_INTERRUPT_PIN 2
#define ALARM_INTERVAL 5

volatile bool clock_flag = false;
DateTime time_now;


void onClock_Interrupt(void) { //Interrupt to set tiem and to change flag -> main work in loop()
  clock_flag = true; 
}

void setup() {
  Serial.begin(115200);

  //Setup RTC
  if(!rtc.begin()){
    Serial.println("Could not find RTC!");
  }
  if(rtc.lostPower()) {
    Serial.println("RTC lost power, set time!");
  }
  rtc.disable32K();                         // Disable 32K pin
  rtc.clearAlarm(1);rtc.clearAlarm(2);      // Clear and disable alarms 1 and 2, since they are not reset with MCU board
  rtc.disableAlarm(1); rtc.disableAlarm(2);
  rtc.writeSqwPinMode(DS3231_OFF);          // Stop INT/SQW pin
  
  if(!rtc.setAlarm1(rtc.now() + TimeSpan(ALARM_INTERVAL),DS3231_A1_Second)) { // schedule an alarm ALARM_INTERVAL seconds in the future
      Serial.println("Error, alarm wasn't set!");
  }

  //Interrut pins
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onClock_Interrupt, FALLING);
}

void loop() {
  
  // Handel clock interrupt
  if(clock_flag){  
    clock_flag = false;
    time_now = rtc.now();
    char time_char[10] = "hh:mm:ss"; // Initialize time char
    time_now.toString(time_char);
    //rtc.now().toString(time_char);
    Serial.println(time_char);
    rtc.clearAlarm(1);
    rtc.setAlarm1(time_now + TimeSpan(ALARM_INTERVAL),DS3231_A1_Second); //set alarm ALARM_INTERVAL seconds to the future
  }  

}
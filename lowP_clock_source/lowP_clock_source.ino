#include <RTClib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <nrf_gpio.h>

RTC_DS3231 rtc; // define rtc object

#define CLOCK_INTERRUPT_PIN 2
#define ALARM_INTERVAL 5
#define POWER_CONTROL_PIN 30

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


volatile bool clock_flag = false;
DateTime time_now;

void onClock_Interrupt(void) { //Interrupt to set tiem and to change flag -> main work in loop()
  clock_flag = true; 
}

void refresh_time(){
  time_now = rtc.now();
  char time_char[10] = "hh:mm:ss"; // Initialize time char
  char date_char[12] = "DD:MM:YYYY";
  time_now.toString(time_char);
  time_now.toString(date_char);
  //rtc.now().toString(time_char);
  // Print on serial and update motitor
  Serial.println(time_char); 
  display.clearDisplay();
  display.setTextSize(2);              // 1 = 6x8 pixels per character
  display.setTextColor(SSD1306_WHITE); // WHITE on BLACK background
  display.setCursor(16, 10);             // Top-left corner
  display.println(time_char);
  display.setTextSize(1); display.setCursor(34,40);display.println(date_char);
  display.display();
      
  // Set new alarm
  rtc.clearAlarm(1);
  rtc.setAlarm1(time_now + TimeSpan(ALARM_INTERVAL),DS3231_A1_Second); //set alarm ALARM_INTERVAL seconds to the future  
  delay(2000);
}

void setup() {
  pinMode(POWER_CONTROL_PIN, OUTPUT);
  digitalWrite(POWER_CONTROL_PIN, LOW);
  Serial.begin(115200);

  // --------- Setup Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    Serial.println(F("SSD1306 allocation failed"));    
  }
  display.clearDisplay(); // clear display buffer


  // --------- Setup RTC
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
  
  refresh_time();
}

void loop() {  
  /*
  // Handel clock interrupt
  if(clock_flag){  
    clock_flag = false;
    refresh_time();
  }  
  */
  
  //Deep sleep
  Serial.println("Entering deep sleep");
  Serial.println();
  digitalWrite(POWER_CONTROL_PIN, HIGH);
  uint32_t nrfPin = g_ADigitalPinMap[CLOCK_INTERRUPT_PIN];
  nrf_gpio_cfg_sense_input(nrfPin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  NRF_POWER->SYSTEMOFF = 1;
  

}
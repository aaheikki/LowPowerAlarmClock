// Clock and screen
#include <RTClib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <nrf_gpio.h>

RTC_DS3231 rtc; // define rtc object

#define CLOCK_INTERRUPT_PIN 2
#define ALARM_INTERVAL 60
#define POWER_CONTROL_PIN 30

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
volatile bool clock_flag = false;
DateTime time_now;
DateTime time_last_interaction ;

void onClock_Interrupt(void) { //Interrupt to set tiem and to change flag -> main work in loop()
  clock_flag = true; 
}

// Encoder
#include <Adafruit_seesaw.h>
#include <seesaw_neopixel.h>

#define SS_SWITCH        24
#define SS_NEOPIX        6
#define SEESAW_ADDR      0x36
#define BUZZER_PIN 27
#define ENCODER_INTERRUPT_PIN 3
#define INTERACTION_INTERVAL 30
#define BUTTON_INT_PIN 4

Adafruit_seesaw ss;
seesaw_NeoPixel sspixel = seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800);
int32_t encoder_position;
volatile bool encoder_flag = false;

void onEncoder_Interrupt(void) { //Interrupt to set tiem and to change flag -> main work in loop()
  encoder_flag = true; 
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
  Serial.print("Next wake up alarm: ");
  print_time(time_now + TimeSpan(ALARM_INTERVAL));
}

void print_time(DateTime time){  
  char time_char[10] = "hh:mm:ss"; // Initialize time char  
  time.toString(time_char);  
  Serial.println(time_char); 
}

void refresh_position(){  
    //Serial.println("Encoder Interrupt!");
    time_last_interaction = rtc.now();
    char time_char[10] = "hh:mm:ss"; // Initialize time char  
    time_last_interaction.toString(time_char);
    Serial.print("last_interaction: ");
    Serial.println(time_char); 
    time_now.toString(time_char);
    // Only now do we talk to the seesaw over I2C
    if (!ss.digitalRead(SS_SWITCH)) {
        Serial.println("Button pressed!");        
        digitalWrite(BUZZER_PIN, HIGH); // Alarm on
    } else {
        digitalWrite(BUZZER_PIN, LOW); // Alarm off
    }

    int32_t new_position = ss.getEncoderPosition();
    if (encoder_position != new_position) {
        Serial.println(new_position);
        sspixel.setPixelColor(0, Wheel(new_position & 0xFF));
        sspixel.show();
        encoder_position = new_position;
    }
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return sspixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return sspixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return sspixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setup() {  
  pinMode(POWER_CONTROL_PIN, OUTPUT);
  digitalWrite(POWER_CONTROL_PIN, LOW);
  uint32_t buttonPin = g_ADigitalPinMap[BUTTON_INT_PIN];
  nrf_gpio_cfg_sense_input(buttonPin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  
  Serial.begin(115200);
  Serial.println("Start of setup");
  
  if (!digitalRead(BUTTON_INT_PIN)){
    encoder_flag = true;    
    Serial.println("Button press detected!");
  };

  // --------- Setup Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    Serial.println(F("SSD1306 allocation failed"));    
  }
  display.clearDisplay(); // clear display buffer
  //Serial.println("Screen found");

  // --------- Setup RTC
  if(!rtc.begin()){
    Serial.println("Could not find RTC!");
  }
  //Serial.println("Clock found");
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

  //Clock interrut pin
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onClock_Interrupt, FALLING);
  
  // Setup encoder
  refresh_time();
  if (! ss.begin(SEESAW_ADDR) || ! sspixel.begin(SEESAW_ADDR)) {
    Serial.println("Couldn't find seesaw on default address");
    while(1) delay(10);
  }
  // Serial.println("seesaw started");


  // set not so bright!
  sspixel.setBrightness(20);
  sspixel.show();
  
  // use a pin for the built in encoder switch
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);

  // get starting position
  encoder_position = ss.getEncoderPosition();

  delay(10);
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();

  // Buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);   // configure pin BUZZER_PIN as output
  digitalWrite(BUZZER_PIN,LOW);

  // Encoder interrut pin
  pinMode(ENCODER_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_INTERRUPT_PIN), onEncoder_Interrupt, FALLING);
  
}

void loop() {  
  // Handel clock interrupt
  while( clock_flag || encoder_flag || (rtc.now() <= (time_last_interaction + TimeSpan(INTERACTION_INTERVAL) ))){
    
    /* depugging
    Serial.print("Button intterrupt state before sleep: ")  ;
    Serial.println(digitalRead(BUTTON_INT_PIN));
    delay(500);
    Serial.print("time_last_interaction_bool: ");
    Serial.println((rtc.now() <= (time_last_interaction + TimeSpan(INTERACTION_INTERVAL) )));
    DateTime timeint = (time_last_interaction + TimeSpan(INTERACTION_INTERVAL));
    char time_char[10] = "hh:mm:ss"; // Initialize time char  
    //timeint.toString(time_char);
    //Serial.println(time_char);
    */
    if(clock_flag){  
      clock_flag = false; refresh_time();
    }
    if(encoder_flag){  
      encoder_flag = false; refresh_position();
    }
  }
  
  delay(5000);
  //Deep sleep
  Serial.println("Entering deep sleep");
  Serial.println();
  digitalWrite(POWER_CONTROL_PIN, HIGH);  
  uint32_t nrfPin = g_ADigitalPinMap[CLOCK_INTERRUPT_PIN];
  nrf_gpio_cfg_sense_input(nrfPin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  
  delay(10);
  NRF_POWER->SYSTEMOFF = 1;
  

}
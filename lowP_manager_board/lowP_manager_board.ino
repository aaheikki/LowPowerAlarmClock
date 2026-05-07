#define CONTROL_PIN 27
#define TIME_DELAY 10000
#define CLOCK_INTERRUPT_PIN 2

void setup() {
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);

  Serial.begin(115200);
  Serial.println("POWER ON");
}

void loop() {

  delay(TIME_DELAY);

  //Deep sleep
  digitalWrite(CONTROL_PIN, LOW);
  Serial.print("Read interrupt pin: ");
  Serial.println(digitalRead(CLOCK_INTERRUPT_PIN));
  Serial.println("POWER OFF");
  Serial.println("Entering deep sleep");
  Serial.println();
  uint32_t nrfPin = g_ADigitalPinMap[CLOCK_INTERRUPT_PIN];
  nrf_gpio_cfg_sense_input(nrfPin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  NRF_POWER->SYSTEMOFF = 1;
}
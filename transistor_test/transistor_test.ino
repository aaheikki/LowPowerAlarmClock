#define CONTROL_PIN 27
#define TIME_DELAY 2000
int loop_count;
void setup() {
  pinMode(CONTROL_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Initial wait, PIN NOT LOW OR HIGH");
  delay(2000);
}

void loop() {    
  digitalWrite(CONTROL_PIN, LOW);
  Serial.print("Gate: ");
  Serial.println(digitalRead(CONTROL_PIN));
  delay(TIME_DELAY);
  digitalWrite(CONTROL_PIN, HIGH);
  Serial.print("Gate: ");
  Serial.println(digitalRead(CONTROL_PIN));
  delay(TIME_DELAY);
  loop_count += 1;

  if(loop_count > 9){
    Serial.println("SYSTEMOFF");
    Serial.println();
    //nrf_gpio_cfg_sense_input(CONTROL_PIN, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    NRF_POWER->SYSTEMOFF = 1;
  }
}

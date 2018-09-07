#define BASE_PLUS_PIN 1
#define BASE_MINUS_PIN 0
#define BASE_SCREW_PIN 5

#define SEL_BASE_PIN 3
#define READ_PIN 7

#define NUM_SWITCHES 5

#define MULTIPLEXER_PINS 4
void setup() {
  Serial.begin(9600);
  for(int  i = 0; i < MULTIPLEXER_PINS; i++){
    pinMode(SEL_BASE_PIN + i, OUTPUT);
  }
  pinMode(READ_PIN, INPUT);
}

void loop() {
  int val = 99;
  for(int i = 15; i > 15-NUM_SWITCHES*2; i--) {
    if (i!= 14 && i!=10&&i!=13) {
      val = read_pin(i);
      Serial.println(" PIN " + String(i) +" VAL IS " + String(val));
    }
  }
  delay(1000);
  
}

int read_pin(int pin) {
  //LSB to MSB
  for(int i = 0; i < MULTIPLEXER_PINS; i++) {
    digitalWrite(SEL_BASE_PIN+i, pin % 2);
    pin = pin / 2;
  }
  return digitalRead(READ_PIN);
}

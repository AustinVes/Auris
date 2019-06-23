int gatePin = 2;
int ledPin = LED_BUILTIN;

void setup() {
  
  pinMode(gatePin, INPUT);
  pinMode(ledPin, OUTPUT);
  
}

void loop() {

  digitalWrite(ledPin, digitalRead(gatePin));

}

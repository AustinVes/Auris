int buttonPin = 3;
int ledPin = LED_BUILTIN;

volatile int buttonState = 0;

void setup() {

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, CHANGE);
  
}

void loop() {

}

void buttonISR() {
  buttonState = digitalRead(buttonPin);
  digitalWrite(ledPin, buttonState);
}

int envelopePin = A1;
int ledPin = LED_BUILTIN;

int envelopeValue;

void setup() {

  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  
}

void loop() {

  if (Serial) {

    digitalWrite(ledPin, HIGH);
    
    envelopeValue = analogRead(envelopePin);
    Serial.println(envelopeValue);
    
  } else {

    digitalWrite(ledPin, LOW);
    
  }

  delay(10);

}

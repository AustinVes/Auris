int audioPin = A0;
int ledPin = LED_BUILTIN;

int audioValue;

void setup() {

  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  
}

void loop() {

  if (Serial) {

    digitalWrite(ledPin, HIGH);
    
    audioValue = analogRead(audioPin);
    Serial.println(audioValue);
    
  } else {

    digitalWrite(ledPin, LOW);
    
  }

  delay(1);

}

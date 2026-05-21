const int ledPin = 13;  // Built-in LED

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);  // Start serial communication
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == '1') {
      digitalWrite(ledPin, HIGH);  // Turn LED ON
    } 
    else if (command == '0') {
      digitalWrite(ledPin, LOW);   // Turn LED OFF
    }
  }
}
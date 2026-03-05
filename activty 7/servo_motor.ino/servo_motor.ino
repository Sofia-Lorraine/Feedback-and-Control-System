#include <Servo.h>

const int SERVO_PIN = 9;

Servo myServo;

String readString = "";
int currentServoAngle = 90;

void setup() {
  Serial.begin(9600);

  myServo.attach(SERVO_PIN);
  myServo.write(currentServoAngle);

  Serial.println("SYSTEM_READY");
}

void loop() {

  while (Serial.available()) {

    char c = Serial.read();

    if (c == '\n') {

      readString.trim();

      if (readString.length() > 0) {

        int desiredAngle = readString.toInt();

        if (desiredAngle >= 0 && desiredAngle <= 180) {

          myServo.write(desiredAngle);

          Serial.print("OK:");
          Serial.println(desiredAngle);

        } else {

          Serial.println("ERROR:OUT_OF_RANGE");
        }
      }

      readString = "";
    }
    else if (c != '\r') {
      readString += c;
    }
  }
}

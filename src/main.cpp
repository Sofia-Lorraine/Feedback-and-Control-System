#include <Arduino.h> 
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

// Pin definitions
const int PIR_PIN = 2;
const int BUZZER_PIN = 8;

// LED Pin definitions
const int LED_GREEN = 5;
const int LED_BLUE = 6;
const int LED_RED = 7;

// Active security monitoring time (24-hour format)
int startHour = 0;   // 12:00 AM
int endHour = 24;    // 1:00 PM

// To prevent continuous buzzing loop trigger
bool motionState = false;

// --- Function Prototypes (This tells the compiler they exist below!) ---
bool isWithinActiveHours(int hour);
void triggerAlert(String message);

void setup() {
  Serial.begin(9600);

  pinMode(PIR_PIN, INPUT); 
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialize LED Pins
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }

  // System Starts with Green Light On
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_RED, LOW);

  Serial.println("SecureAgent System Initialized.");
  Serial.println("Waiting 5 seconds for PIR sensor to warm up...");
  delay(5000); 
}

void loop() {
  DateTime now = rtc.now();

  int currentHour = now.hour();
  bool isSecurityHours = isWithinActiveHours(currentHour);
  
  // Keep sending the structured data stream to your Python AI
  String currentMode = isSecurityHours ? "SECURITY" : "SAFE";

  int motionDetected = digitalRead(PIR_PIN);

  // Send data string to Python Agent
  Serial.print("TIME=");
  Serial.print(now.timestamp());
  Serial.print("|MOTION=");
  Serial.print(motionDetected);
  Serial.print("|MODE=");
  Serial.println(currentMode);

  // Local hardware reactions
  if (motionDetected == HIGH && motionState == false) {
    motionState = true;
    
    // RESTORED PREVIOUS LOGIC: Only buzz and flash police lights if inside the security window
    if (isSecurityHours) {
       triggerAlert("Security Breach!");
    } else {
       // Safe/Work Hours: No buzzer! Just switch from Green to Blue light
       digitalWrite(LED_GREEN, LOW);
       digitalWrite(LED_BLUE, HIGH); 
       digitalWrite(LED_RED, LOW);
    }
  }

  if (motionDetected == LOW) {
    motionState = false;
    // Restore default safe state green light when area clears up
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_RED, LOW);
  }

  // Listen for emergency commands coming BACK from the Python AI Agent
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "TRIGGER_ALARM") {
      triggerAlert("AI Remote Triggered Override!");
    }
  }

  delay(1000); 
}

// Full function implementation
bool isWithinActiveHours(int hour) {
  if (startHour > endHour) {
    return (hour >= startHour || hour < endHour);
  } else {
    return (hour >= startHour && hour < endHour);
  }
}

// Full function implementation
void triggerAlert(String message) {
  digitalWrite(LED_GREEN, LOW);

  // Police style strobe light and siren effect
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_BLUE, LOW);
    tone(BUZZER_PIN, 1500); 
    delay(150);
    
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, HIGH);
    tone(BUZZER_PIN, 1000); 
    delay(150);
  }

  noTone(BUZZER_PIN);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
}
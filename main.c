#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// Set the LCD address
const int lcdColumns = 16;
const int lcdRows = 2;
const int lcdAddress = 0x27;

hd44780_I2Cexp lcd;

// Replace with your network credentials
char auth[] = "CGIk7YvhyO_L6GmaCaTaUGdQKJxxmQUS";
char ssid[] = "GlobeAtHome_B9F10";
char pass[] = "708AB2E1";

// Virtual pin for the button in the Blynk app
int buttonPin = V1;
int rotationSpeed = 10;
WiFiClient client;

// Servo pin
int servoPin = D7;

// Servo object
Servo servo;

// LED pin
int ledPin = D8;

// Feeding time variables
int feedingHour = 12;
int feedingMinute = 0;
bool isFeedingScheduled = false;

// RTC widget
WidgetRTC rtc;

void setup() {
  lcd.begin(lcdColumns, lcdRows);
  lcd.print("Smart Pet Feeder");
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  setSyncInterval(10 * 60); // Sync time every 10 minutes
  servo.attach(servoPin);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Set LED pin as OUTPUT
  pinMode(ledPin, OUTPUT);

  // Turn off the LED initially
  turnOffLED();

  // Set initial feeding time
  setFeedingTime(12, 0); // Set the feeding time to 12:00 PM
}

void loop() {
  Blynk.run();
  rtc.begin();
  processFeedingSchedule();
  displayNextFeedingTime();
}

BLYNK_WRITE(V4) {
  if (param.asInt() == 180) {
    // Turn on the LED
    turnOnLED();
  } else {
    // Turn off the LED
    turnOffLED();
  }

  servo.write(param.asInt());
}

void turnOnLED() {
  digitalWrite(ledPin, HIGH);
}

void turnOffLED() {
  digitalWrite(ledPin, LOW);
}

void setFeedingTime(int hour, int minute) {
  feedingHour = hour;
  feedingMinute = minute;
  isFeedingScheduled = true;
  Serial.print("Feeding time set to: ");
  Serial.print(feedingHour);
  Serial.print(":");
  Serial.println(feedingMinute);
}

void processFeedingSchedule() {
  if (isFeedingScheduled) {
    // Get the current time
    time_t currentTime = now();

    // Get the current hour and minute
    int currentHour = hour(currentTime);
    int currentMinute = minute(currentTime);

    // Check if it's time for feeding
    if (currentHour == feedingHour && currentMinute == feedingMinute) {
      // Perform feeding action
      feedPets();
    }
  }
}

void feedPets() {
  // TODO: Implement feeding logic here
  // This function will be called when it's time to feed the pets
  Serial.println("Feeding the pets!");
}

void displayNextFeedingTime() {
  if (isFeedingScheduled) {
    // Get the current time
    time_t currentTime = now();

    // Calculate the current time in seconds since midnight
    int currentSeconds = hour(currentTime) * 3600 + minute(currentTime) * 60 + second(currentTime);

    // Calculate the scheduled feeding time in seconds since midnight
    int scheduledSeconds = feedingHour * 3600 + feedingMinute * 60;

    // Calculate the remaining time until the next feeding
    int remainingSecs = scheduledSeconds - currentSeconds;

    // Calculate the hours, minutes, and seconds remaining
    int remainingHours = remainingSecs / 3600;
    int remainingMinutes = (remainingSecs % 3600) / 60;
    remainingSecs = remainingSecs % 60;

    // Display the next feeding time on the LCD
    lcd.setCursor(0, 1);
    lcd.print("Next feeding: ");
    lcd.print(remainingHours);
    lcd.print(":");
    if (remainingMinutes < 10) {
      lcd.print("0");
    }
    lcd.print(remainingMinutes);
    lcd.print(":");
    if (remainingSecs < 10) {
      lcd.print("0");
    }
    lcd.print(remainingSecs);
    Blynk.virtualWrite(V7, remainingSecs); // Update virtual display on Blynk app
    delay(3000);
  } else {
    // No feeding time scheduled
    lcd.setCursor(0, 1);
    lcd.print("No feed time");
    Blynk.virtualWrite(V7, "No feed time"); // Update virtual display on Blynk app
    delay(3000);
  }
}

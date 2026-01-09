// SMART DUSTBIN SYSTEM WITH OLED DISPLAY
// Ultrasonic + Servo + Buzzer + LED + OLED

#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ================= OLED ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ================= PINS ================= */
#define TRIG_PIN   9
#define ECHO_PIN   10
#define SERVO_PIN  6
#define BUZZER_PIN 8
#define LED_PIN    13

Servo lidServo;

/* ================= SETTINGS ================= */
const int DETECTION_DISTANCE = 30;  // cm
const int LID_OPEN_ANGLE = 90;
const int LID_CLOSE_ANGLE = 0;
const unsigned long LID_OPEN_TIME = 5000;

/* ================= VARIABLES ================= */
bool lidOpen = false;
unsigned long lidOpenedAt = 0;
int distanceCM = 0;

/* ================= SETUP ================= */
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  lidServo.attach(SERVO_PIN);
  lidServo.write(LID_CLOSE_ANGLE);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED FAILED"));
    while (1);
  }

  displayStartup();
  delay(2000);

  Serial.println(F("SMART DUSTBIN SYSTEM READY"));
}

/* ================= LOOP ================= */
void loop() {
  distanceCM = measureDistance();
  updateDisplay();

  Serial.print(F("Distance: "));
  Serial.print(distanceCM);
  Serial.print(F(" cm | "));

  if (distanceCM > 0 && distanceCM <= DETECTION_DISTANCE) {
    Serial.println(F("PERSON DETECTED"));

    if (!lidOpen) {
      openLid();
    } else {
      lidOpenedAt = millis(); // reset timer
    }
  }
  else {
    Serial.println(F("NO PERSON"));

    if (lidOpen && millis() - lidOpenedAt >= LID_OPEN_TIME) {
      closeLid();
    }
  }

  delay(150);
}

/* ================= ULTRASONIC ================= */
int measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return -1;

  int cm = duration * 0.034 / 2;
  if (cm > 400) return -1;

  return cm;
}

/* ================= DISPLAY ================= */
void displayStartup() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 15);
  display.println(F("SMART"));
  display.setCursor(10, 40);
  display.println(F("DUSTBIN"));
  display.display();
}

void updateDisplay() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println(F("SMART DUSTBIN"));
  display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print(F("Distance: "));
  if (distanceCM > 0) {
    display.print(distanceCM);
    display.print(F(" cm"));
  } else {
    display.print(F("---"));
  }

  display.setTextSize(2);
  display.setCursor(0, 32);
  display.print(F("LID: "));
  display.println(lidOpen ? F("OPEN") : F("CLOSED"));

  display.setTextSize(1);
  display.setCursor(0, 54);
  display.println(
    (distanceCM > 0 && distanceCM <= DETECTION_DISTANCE)
    ? F(">> PERSON NEAR <<")
    : F("Waiting...")
  );

  display.display();
}

/* ================= LID CONTROL ================= */
void openLid() {
  Serial.println(F("OPENING LID"));

  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000, 200);

  for (int a = LID_CLOSE_ANGLE; a <= LID_OPEN_ANGLE; a += 2) {
    lidServo.write(a);
    delay(12);
  }

  lidOpen = true;
  lidOpenedAt = millis();
}

void closeLid() {
  Serial.println(F("CLOSING LID"));

  tone(BUZZER_PIN, 800, 100);
  delay(150);
  tone(BUZZER_PIN, 800, 100);

  for (int a = LID_OPEN_ANGLE; a >= LID_CLOSE_ANGLE; a -= 2) {
    lidServo.write(a);
    delay(12);
  }

  digitalWrite(LED_PIN, LOW);
  lidOpen = false;
}

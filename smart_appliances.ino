#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <Servo.h>

// LCD setup (I2C)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust address if needed

// IR Receiver pin
#define IR_RECV_PIN 11
IRrecv irrecv(IR_RECV_PIN);
decode_results results;

// Device pins
#define FAN_ENA 10  // PWM for fan speed
#define FAN_IN1 9
#define FAN_IN2 8

#define LED_PIN 12   // PWM for LED
#define SERVO_PIN 13
#define BUZZER_PIN A0
#define WATER_SENSOR A1

// Objects
Servo myServo;

// States
int menuIndex = 0;
bool inSubMenu = false;
bool servoLocked = false;
int fanSpeed = 150;
int ledBrightness = 128;
unsigned long previousMillis = 0;
unsigned long simulatedTime = 0;
unsigned long alarmTime = 60000; // Alarm after 60 seconds

// Servo
int servoAngle = 90;

// Menu
String mainMenu[] = {"Fan", "LED", "Servo", "Clock"};
const int menuCount = sizeof(mainMenu) / sizeof(mainMenu[0]);

// IR Codes (map these by testing your remote later)
#define KEY_UP 0xFF18E7
#define KEY_DOWN 0xFF4AB5
#define KEY_OK 0xFF38C7
#define KEY_BACK 0xFF10EF
#define KEY_1 0xFF30CF
#define KEY_2 0xFF18E7
#define KEY_3 0xFF7A85
#define KEY_4 0xFF10EF

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  irrecv.enableIRIn();

  // Set pin modes
  pinMode(FAN_IN1, OUTPUT);
  pinMode(FAN_IN2, OUTPUT);
  pinMode(FAN_ENA, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WATER_SENSOR, INPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(servoAngle);

  displayMenu();
}

void loop() {
  handleIR();
  simulateClock();
  checkWaterSensor();
  checkAlarm();
}

// ========== Display ==========
void displayMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("> " + mainMenu[menuIndex]);
  if (menuIndex + 1 < menuCount) {
    lcd.setCursor(0, 1);
    lcd.print("  " + mainMenu[menuIndex + 1]);
  }
}

void displaySubMenu(String title) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(title);
  lcd.setCursor(0, 1);
  lcd.print("OK=Toggle  #=Back");
}

// ========== IR Handling ==========
void handleIR() {
  if (irrecv.decode(&results)) {
    unsigned long key = results.value;
    irrecv.resume();

    // Shortcut keys
    if (key == KEY_1) menuIndex = 0, enterSubMenu();
    else if (key == KEY_2) menuIndex = 1, enterSubMenu();
    else if (key == KEY_3) menuIndex = 2, enterSubMenu();
    else if (key == KEY_4) menuIndex = 3, enterSubMenu();

    if (!inSubMenu) {
      if (key == KEY_DOWN && menuIndex < menuCount - 1) {
        menuIndex++;
        displayMenu();
      } else if (key == KEY_UP && menuIndex > 0) {
        menuIndex--;
        displayMenu();
      } else if (key == KEY_OK) {
        enterSubMenu();
      }
    } else {
      if (key == KEY_OK) {
        handleSubMenu(menuIndex);
      } else if (key == KEY_BACK) {
        inSubMenu = false;
        displayMenu();
      }
    }
  }
}

// ========== Submenu Logic ==========
void enterSubMenu() {
  inSubMenu = true;
  displaySubMenu(mainMenu[menuIndex]);
}

void handleSubMenu(int index) {
  switch (index) {
    case 0: // Fan
      digitalWrite(FAN_IN1, HIGH);
      digitalWrite(FAN_IN2, LOW);
      analogWrite(FAN_ENA, fanSpeed);
      break;
    case 1: // LED
      static bool ledOn = false;
      ledOn = !ledOn;
      analogWrite(LED_PIN, ledOn ? ledBrightness : 0);
      break;
    case 2: // Servo
      if (!servoLocked) {
        servoAngle = (servoAngle == 90) ? 150 : 90;
        myServo.write(servoAngle);
      }
      break;
    case 3: // Clock
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time:");
      lcd.setCursor(0, 1);
      lcd.print(simulatedTime / 1000);
      delay(2000);
      displaySubMenu("Clock");
      break;
  }
}

// ========== Clock ==========
void simulateClock() {
  unsigned long currentMillis = millis();
  simulatedTime = currentMillis;
}

void checkAlarm() {
  if (simulatedTime >= alarmTime) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    alarmTime += 60000; // Set next alarm in 1 min
  }
}

// ========== Water Sensor & Servo Lock ==========
void checkWaterSensor() {
  int value = analogRead(WATER_SENSOR);
  if (value > 400 && !servoLocked) {
    myServo.write(0);
    servoLocked = true;
  }
}

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTTON1 2
#define BUTTON2 3
#define BUTTON3 4
#define RESET_BUTTON 5
#define GREEN_LED 6
#define RED_LED 7
#define BUZZER 8

LiquidCrystal_I2C lcd(0x27, 16, 2);

int setPin[3];
int enteredPin[3];
int pinIndex = 0;
bool pinSet = false;

void setup() {
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  if (digitalRead(RESET_BUTTON) == LOW) {
    resetPin();
    delay(500);
  }

  if (!pinSet) {
    lcd.setCursor(0, 0);
    lcd.print("Set PIN:");
    getInput(setPin);
    if (pinIndex == 3) {
      pinSet = true;
      pinIndex = 0;
      lcd.clear();
      lcd.print("PIN Saved!");
      delay(1000);
      lcd.clear();
    }
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Enter PIN:");
    getInput(enteredPin);
    if (pinIndex == 3) {
      checkPin();
      pinIndex = 0;
    }
  }
}

void getInput(int arr[]) {
  if (digitalRead(BUTTON1) == LOW) {
    arr[pinIndex++] = 1;
    lcd.setCursor(pinIndex - 1, 1);
    lcd.print("*");
    delay(300);
    while (digitalRead(BUTTON1) == LOW);
  }

  if (digitalRead(BUTTON2) == LOW) {
    arr[pinIndex++] = 2;
    lcd.setCursor(pinIndex - 1, 1);
    lcd.print("*");
    delay(300);
    while (digitalRead(BUTTON2) == LOW);
  }

  if (digitalRead(BUTTON3) == LOW) {
    arr[pinIndex++] = 3;
    lcd.setCursor(pinIndex - 1, 1);
    lcd.print("*");
    delay(300);
    while (digitalRead(BUTTON3) == LOW);
  }
}

void checkPin() {
  lcd.clear();
  if (setPin[0] == enteredPin[0] && setPin[1] == enteredPin[1] && setPin[2] == enteredPin[2]) {
    lcd.print("PIN CORRECT!");
    digitalWrite(GREEN_LED, HIGH);
    delay(2000);
    digitalWrite(GREEN_LED, LOW);
  } else {
    lcd.print("INCORRECT PIN");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(2000);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
  }
  lcd.clear();
}

void resetPin() {
  pinSet = false;
  pinIndex = 0;
  lcd.clear();
  lcd.print("PIN Reset");
  delay(1000);
  lcd.clear();
}

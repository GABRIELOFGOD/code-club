#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTTON1 2
#define BUTTON2 3
#define BUTTON3 4
#define RESET_BUTTON 5
#define GREEN_LED 6
#define RED_LED 7
#define BUZZER 8

// Try 0x3F if 0x27 doesn't work
LiquidCrystal_I2C lcd(0x27, 16, 2);

int setPin[3];
int enteredPin[3];
int pinIndex = 0;
bool pinSet = false;
bool displayUpdated = false;

void setup() {
  Serial.begin(9600); // For debugging
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Test display
  lcd.setCursor(0, 0);
  lcd.print("LCD Test");
  lcd.setCursor(0, 1);
  lcd.print("Working!");
  delay(2000);

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  Serial.println("System Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  // Check reset button
  if (digitalRead(RESET_BUTTON) == LOW) {
    resetPin();
    delay(500);
    return;
  }

  if (!pinSet) {
    // Display "Set PIN:" only once
    if (!displayUpdated) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Set PIN:");
      displayUpdated = true;
      Serial.println("Waiting for PIN setup...");
    }
    
    getInput(setPin);
    
    if (pinIndex >= 3) {
      pinSet = true;
      pinIndex = 0;
      displayUpdated = false;
      lcd.clear();
      lcd.print("PIN Saved!");
      Serial.println("PIN Saved!");
      delay(1500);
      lcd.clear();
    }
  } else {
    // Display "Enter PIN:" only once
    if (!displayUpdated) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
      displayUpdated = true;
      Serial.println("Waiting for PIN entry...");
    }
    
    getInput(enteredPin);
    
    if (pinIndex >= 3) {
      checkPin();
      pinIndex = 0;
      displayUpdated = false;
    }
  }
}

void getInput(int arr[]) {
  static unsigned long lastButtonPress = 0;
  unsigned long currentTime = millis();
  
  // Debounce delay
  if (currentTime - lastButtonPress < 200) {
    return;
  }

  if (digitalRead(BUTTON1) == LOW) {
    arr[pinIndex] = 1;
    lcd.setCursor(pinIndex, 1);
    lcd.print("*");
    pinIndex++;
    lastButtonPress = currentTime;
    Serial.print("Button 1 pressed, index: ");
    Serial.println(pinIndex);
    
    // Wait for button release
    while (digitalRead(BUTTON1) == LOW) {
      delay(10);
    }
  }
  
  else if (digitalRead(BUTTON2) == LOW) {
    arr[pinIndex] = 2;
    lcd.setCursor(pinIndex, 1);
    lcd.print("*");
    pinIndex++;
    lastButtonPress = currentTime;
    Serial.print("Button 2 pressed, index: ");
    Serial.println(pinIndex);
    
    // Wait for button release
    while (digitalRead(BUTTON2) == LOW) {
      delay(10);
    }
  }
  
  else if (digitalRead(BUTTON3) == LOW) {
    arr[pinIndex] = 3;
    lcd.setCursor(pinIndex, 1);
    lcd.print("*");
    pinIndex++;
    lastButtonPress = currentTime;
    Serial.print("Button 3 pressed, index: ");
    Serial.println(pinIndex);
    
    // Wait for button release
    while (digitalRead(BUTTON3) == LOW) {
      delay(10);
    }
  }
}

void checkPin() {
  lcd.clear();
  bool correct = true;
  
  for (int i = 0; i < 3; i++) {
    if (setPin[i] != enteredPin[i]) {
      correct = false;
      break;
    }
  }
  
  if (correct) {
    lcd.print("PIN CORRECT!");
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("PIN CORRECT!");
    delay(2000);
    digitalWrite(GREEN_LED, LOW);
  } else {
    lcd.print("INCORRECT PIN");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    Serial.println("INCORRECT PIN");
    delay(2000);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
  }
  
  lcd.clear();
  
  // Clear entered PIN array
  for (int i = 0; i < 3; i++) {
    enteredPin[i] = 0;
  }
}

void resetPin() {
  pinSet = false;
  pinIndex = 0;
  displayUpdated = false;
  lcd.clear();
  lcd.print("PIN Reset");
  Serial.println("PIN Reset");
  
  // Clear both arrays
  for (int i = 0; i < 3; i++) {
    setPin[i] = 0;
    enteredPin[i] = 0;
  }
  
  delay(1500);
  lcd.clear();
}

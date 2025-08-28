#include <IRremote.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Pin definitions
const int IR_RECEIVE_PIN = 2;
const int RELAY_PIN = 7;
const int SERVO_PIN = 9;
const int FAN_PIN = 3;      // PWM pin for fan speed control
const int LED_PIN = 5;      // PWM pin for LED brightness control
const int BUZZER_PIN = 8;   // Buzzer pin

// IR remote button codes (update these with your remote's codes)
const unsigned long POWER_BUTTON = 0xD;
const unsigned long UP_BUTTON = 0x18;
const unsigned long DOWN_BUTTON = 0x52;
const unsigned long LEFT_BUTTON = 0x8;
const unsigned long RIGHT_BUTTON = 0x5A;
const unsigned long OK_BUTTON = 0x1C;
const unsigned long BACK_BUTTON = 0x16;
const unsigned long NUM_1 = 0x45;
const unsigned long NUM_2 = 0x46;
const unsigned long NUM_3 = 0x47;
const unsigned long NUM_4 = 0x44;
const unsigned long NUM_5 = 0x40;
const unsigned long NUM_0 = 0x19;

// LCD setup (address 0x27 is common, but may vary)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Menu system
enum MenuState {
  MAIN_MENU,
  FAN_CONTROL,
  SERVO_CONTROL,
  LED_CONTROL,
  STOPWATCH,
  RELAY_CONTROL
};

enum MainMenuItems {
  MENU_FAN = 0,
  MENU_SERVO = 1,
  MENU_LED = 2,
  MENU_STOPWATCH = 3,
  MENU_RELAY = 4,
  MENU_COUNT = 5
};

// System state variables
bool systemPower = false;
MenuState currentMenu = MAIN_MENU;
int currentMenuItem = 0;
bool inSubMenu = false;

// Component states
int fanSpeed = 0;           // 0-255
int ledBrightness = 0;      // 0-255
int servoPosition = 90;     // 0-180 degrees
bool relayState = false;

// Stopwatch variables
unsigned long stopwatchTime = 0;  // in seconds
bool stopwatchRunning = false;
unsigned long lastStopwatchUpdate = 0;
bool stopwatchSetMode = false;

// Menu strings
const char* mainMenuItems[] = {
  "1.Fan Control",
  "2.Servo Control", 
  "3.LED Control",
  "4.Stopwatch",
  "5.Relay Control"
};

Servo windowServo;

void setup() {
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Initialize IR receiver (IRremote 4.x syntax)
  IrReceiver.begin(IR_RECEIVE_PIN);
  
  // Initialize servo
  windowServo.attach(SERVO_PIN);
  windowServo.write(servoPosition);
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initial states
  digitalWrite(RELAY_PIN, LOW);
  analogWrite(FAN_PIN, 0);
  analogWrite(LED_PIN, 0);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Welcome screen
  lcd.setCursor(0, 0);
  lcd.print("Smart Appliance");
  lcd.setCursor(0, 1);
  lcd.print("Controller V2.0");
  delay(2000);
  
  showPowerOffScreen();
  
  Serial.println("Smart Appliance Controller with LCD Ready!");
}

void loop() {
  // Handle IR remote input
  if (IrReceiver.decode()) {
    unsigned long command = IrReceiver.decodedIRData.command;
    
    // Handle repeat codes for continuous control
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      // Handle repeat for continuous actions like volume/brightness
      if (command == UP_BUTTON || command == DOWN_BUTTON || 
          command == LEFT_BUTTON || command == RIGHT_BUTTON) {
        processIRCommand(command);
      }
    } else {
      processIRCommand(command);
    }
    
    IrReceiver.resume();
  }
  
  // Update stopwatch if running
  if (systemPower && stopwatchRunning && currentMenu == STOPWATCH) {
    updateStopwatch();
  }
  
  delay(50);
}

void processIRCommand(unsigned long command) {
  Serial.print("IR Command: 0x");
  Serial.println(command, HEX);
  
  switch(command) {
    case POWER_BUTTON:
      toggleSystemPower();
      break;
      
    case NUM_1:
      if (systemPower) {
        quickAccessMenu(FAN_CONTROL);
      }
      break;
      
    case NUM_2:
      if (systemPower) {
        quickAccessMenu(SERVO_CONTROL);
      }
      break;
      
    case NUM_3:
      if (systemPower) {
        quickAccessMenu(LED_CONTROL);
      }
      break;
      
    case NUM_4:
      if (systemPower) {
        quickAccessMenu(STOPWATCH);
      }
      break;
      
    case NUM_5:
      if (systemPower) {
        quickAccessMenu(RELAY_CONTROL);
      }
      break;
      
    case UP_BUTTON:
      if (systemPower) {
        handleUpButton();
      }
      break;
      
    case DOWN_BUTTON:
      if (systemPower) {
        handleDownButton();
      }
      break;
      
    case LEFT_BUTTON:
      if (systemPower) {
        handleLeftButton();
      }
      break;
      
    case RIGHT_BUTTON:
      if (systemPower) {
        handleRightButton();
      }
      break;
      
    case OK_BUTTON:
      if (systemPower) {
        handleOKButton();
      }
      break;
      
    case BACK_BUTTON:
      if (systemPower) {
        handleBackButton();
      }
      break;
      
    case NUM_0:
      if (systemPower && currentMenu == STOPWATCH) {
        toggleStopwatch();
      }
      break;
  }
}

void toggleSystemPower() {
  systemPower = !systemPower;
  
  if (systemPower) {
    currentMenu = MAIN_MENU;
    currentMenuItem = 0;
    inSubMenu = false;
    showMainMenu();
    beep(2, 100); // Power on beep
  } else {
    // Turn off all components
    fanSpeed = 0;
    ledBrightness = 0;
    analogWrite(FAN_PIN, 0);
    analogWrite(LED_PIN, 0);
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    servoPosition = 90;
    windowServo.write(servoPosition);
    stopwatchRunning = false;
    
    showPowerOffScreen();
    beep(1, 200); // Power off beep
  }
}

void quickAccessMenu(MenuState menu) {
  currentMenu = menu;
  inSubMenu = true;
  updateDisplay();
}

void handleUpButton() {
  switch(currentMenu) {
    case MAIN_MENU:
      if (!inSubMenu) {
        currentMenuItem = (currentMenuItem - 1 + MENU_COUNT) % MENU_COUNT;
        showMainMenu();
      }
      break;
      
    case FAN_CONTROL:
      increaseFanSpeed();
      break;
      
    case LED_CONTROL:
      increaseLEDBrightness();
      break;
      
    case SERVO_CONTROL:
      moveServoUp();
      break;
      
    case STOPWATCH:
      if (stopwatchSetMode && !stopwatchRunning) {
        stopwatchTime += 60; // Add 1 minute
        if (stopwatchTime > 3599) stopwatchTime = 3599; // Max 59:59
        showStopwatchScreen();
      }
      break;
  }
}

void handleDownButton() {
  switch(currentMenu) {
    case MAIN_MENU:
      if (!inSubMenu) {
        currentMenuItem = (currentMenuItem + 1) % MENU_COUNT;
        showMainMenu();
      }
      break;
      
    case FAN_CONTROL:
      decreaseFanSpeed();
      break;
      
    case LED_CONTROL:
      decreaseLEDBrightness();
      break;
      
    case SERVO_CONTROL:
      moveServoDown();
      break;
      
    case STOPWATCH:
      if (stopwatchSetMode && !stopwatchRunning) {
        if (stopwatchTime >= 60) {
          stopwatchTime -= 60; // Subtract 1 minute
        } else {
          stopwatchTime = 0;
        }
        showStopwatchScreen();
      }
      break;
  }
}

void handleLeftButton() {
  switch(currentMenu) {
    case SERVO_CONTROL:
      moveServoLeft();
      break;
      
    case STOPWATCH:
      if (stopwatchSetMode && !stopwatchRunning) {
        if (stopwatchTime > 0) {
          stopwatchTime--;
        }
        showStopwatchScreen();
      }
      break;
  }
}

void handleRightButton() {
  switch(currentMenu) {
    case SERVO_CONTROL:
      moveServoRight();
      break;
      
    case STOPWATCH:
      if (stopwatchSetMode && !stopwatchRunning) {
        stopwatchTime++;
        if (stopwatchTime > 3599) stopwatchTime = 3599;
        showStopwatchScreen();
      }
      break;
  }
}

void handleOKButton() {
  switch(currentMenu) {
    case MAIN_MENU:
      if (!inSubMenu) {
        currentMenu = (MenuState)(currentMenuItem + 1);
        inSubMenu = true;
        updateDisplay();
      }
      break;
      
    case RELAY_CONTROL:
      toggleRelay();
      break;
      
    case STOPWATCH:
      if (!stopwatchSetMode) {
        stopwatchSetMode = true;
        showStopwatchScreen();
      } else {
        stopwatchSetMode = false;
        showStopwatchScreen();
      }
      break;
  }
}

void handleBackButton() {
  if (inSubMenu) {
    inSubMenu = false;
    currentMenu = MAIN_MENU;
    stopwatchSetMode = false;
    showMainMenu();
  }
}

void updateDisplay() {
  switch(currentMenu) {
    case MAIN_MENU:
      showMainMenu();
      break;
    case FAN_CONTROL:
      showFanScreen();
      break;
    case SERVO_CONTROL:
      showServoScreen();
      break;
    case LED_CONTROL:
      showLEDScreen();
      break;
    case STOPWATCH:
      showStopwatchScreen();
      break;
    case RELAY_CONTROL:
      showRelayScreen();
      break;
  }
}

void showPowerOffScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System: OFF");
  lcd.setCursor(0, 1);
  lcd.print("Press PWR to ON");
}

void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">");
  lcd.print(mainMenuItems[currentMenuItem]);
  
  if (currentMenuItem < MENU_COUNT - 1) {
    lcd.setCursor(1, 1);
    lcd.print(mainMenuItems[currentMenuItem + 1]);
  }
}

void showFanScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fan Control");
  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print((fanSpeed * 100) / 255);
  lcd.print("%");
}

void showServoScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Servo Control");
  lcd.setCursor(0, 1);
  lcd.print("Pos: ");
  lcd.print(servoPosition);
  lcd.print(" deg");
}

void showLEDScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LED Control");
  lcd.setCursor(0, 1);
  lcd.print("Bright: ");
  lcd.print((ledBrightness * 100) / 255);
  lcd.print("%");
}

void showStopwatchScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (stopwatchSetMode) {
    lcd.print("SET Timer:");
  } else if (stopwatchRunning) {
    lcd.print("Timer Running:");
  } else {
    lcd.print("Timer Stopped:");
  }
  
  lcd.setCursor(0, 1);
  int minutes = stopwatchTime / 60;
  int seconds = stopwatchTime % 60;
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
  
  if (stopwatchSetMode) {
    lcd.print(" [SET]");
  }
}

void showRelayScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Main Appliance");
  lcd.setCursor(0, 1);
  lcd.print("Status: ");
  lcd.print(relayState ? "ON" : "OFF");
}

// Component control functions
void increaseFanSpeed() {
  if (fanSpeed < 255) {
    fanSpeed += 25;
    if (fanSpeed > 255) fanSpeed = 255;
    analogWrite(FAN_PIN, fanSpeed);
    showFanScreen();
  }
}

void decreaseFanSpeed() {
  if (fanSpeed > 0) {
    fanSpeed -= 25;
    if (fanSpeed < 0) fanSpeed = 0;
    analogWrite(FAN_PIN, fanSpeed);
    showFanScreen();
  }
}

void increaseLEDBrightness() {
  if (ledBrightness < 255) {
    ledBrightness += 25;
    if (ledBrightness > 255) ledBrightness = 255;
    analogWrite(LED_PIN, ledBrightness);
    showLEDScreen();
  }
}

void decreaseLEDBrightness() {
  if (ledBrightness > 0) {
    ledBrightness -= 25;
    if (ledBrightness < 0) ledBrightness = 0;
    analogWrite(LED_PIN, ledBrightness);
    showLEDScreen();
  }
}

void moveServoUp() {
  if (servoPosition < 180) {
    servoPosition += 15;
    if (servoPosition > 180) servoPosition = 180;
    windowServo.write(servoPosition);
    showServoScreen();
  }
}

void moveServoDown() {
  if (servoPosition > 0) {
    servoPosition -= 15;
    if (servoPosition < 0) servoPosition = 0;
    windowServo.write(servoPosition);
    showServoScreen();
  }
}

void moveServoLeft() {
  if (servoPosition > 0) {
    servoPosition -= 5;
    if (servoPosition < 0) servoPosition = 0;
    windowServo.write(servoPosition);
    showServoScreen();
  }
}

void moveServoRight() {
  if (servoPosition < 180) {
    servoPosition += 5;
    if (servoPosition > 180) servoPosition = 180;
    windowServo.write(servoPosition);
    showServoScreen();
  }
}

void toggleRelay() {
  relayState = !relayState;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  showRelayScreen();
  beep(1, 100);
}

void toggleStopwatch() {
  if (stopwatchSetMode) {
    stopwatchSetMode = false;
  }
  
  if (!stopwatchRunning && stopwatchTime > 0) {
    // Start countdown
    stopwatchRunning = true;
    lastStopwatchUpdate = millis();
    beep(1, 100);
  } else {
    // Stop countdown
    stopwatchRunning = false;
    beep(2, 50);
  }
  showStopwatchScreen();
}

void updateStopwatch() {
  if (millis() - lastStopwatchUpdate >= 1000) {
    lastStopwatchUpdate = millis();
    
    if (stopwatchTime > 0) {
      stopwatchTime--;
      showStopwatchScreen();
      
      if (stopwatchTime == 0) {
        // Timer finished - sound alarm
        stopwatchRunning = false;
        alarm();
      }
    }
  }
}

void alarm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TIME'S UP!");
  lcd.setCursor(0, 1);
  lcd.print("00:00 [ALARM]");
  
  // Sound alarm for 5 seconds
  for (int i = 0; i < 10; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(250);
    digitalWrite(BUZZER_PIN, LOW);
    delay(250);
  }
  
  showStopwatchScreen();
}

void beep(int count, int duration) {
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < count - 1) delay(duration);
  }
}

// Uncomment the line below in setup() to scan your remote buttons
// scanRemoteButtons();

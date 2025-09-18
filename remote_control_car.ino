#include <IRremote.hpp>

// ---------------- Pin definitions ----------------
const uint8_t RECV_PIN = 11;   // IR receiver pin
const uint8_t IN1 = 7;
const uint8_t IN2 = 6;
const uint8_t IN3 = 5;
const uint8_t IN4 = 4;

const uint8_t TRIG_PIN = 9;    // Ultrasonic trigger
const uint8_t ECHO_PIN = 10;   // Ultrasonic echo

// ---------------- IR button codes ----------------
const unsigned long POWER_BUTTON = 0xD;
const unsigned long UP_BUTTON    = 0x18; // forward
const unsigned long DOWN_BUTTON  = 0x52; // backward
const unsigned long LEFT_BUTTON  = 0x8;  // left
const unsigned long RIGHT_BUTTON = 0x5A; // right
const unsigned long OK_BUTTON    = 0x1C; // stop
const unsigned long BACK_BUTTON  = 0x16;
const unsigned long NUM_1        = 0x45;
const unsigned long NUM_2        = 0x46;
const unsigned long NUM_3        = 0x47;
const unsigned long NUM_4        = 0x44;
const unsigned long NUM_5        = 0x40;
const unsigned long NUM_0        = 0x19; // toggle auto/manual

// ---------------- State ----------------
bool autoMode = false; // false = manual, true = autonomous

// ---------------- Setup ----------------
void setup() {
  Serial.begin(9600);
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  stopMotors();
  Serial.println(F("Ready... Press 0 to toggle auto/manual mode"));
}

// ---------------- Loop ----------------
void loop() {
  if (IrReceiver.decode()) {
    unsigned long code = IrReceiver.decodedIRData.command; // directly use .command
    Serial.print("IR code: ");
    Serial.println(code, HEX);

    if (code == NUM_0) {
      autoMode = !autoMode;
      stopMotors();
      Serial.print("Mode -> ");
      Serial.println(autoMode ? "AUTONOMOUS" : "MANUAL");
      delay(300);
    }

    if (!autoMode) {
      // Manual mode
      if (code == UP_BUTTON) forward();
      else if (code == DOWN_BUTTON) backward();
      else if (code == LEFT_BUTTON) left();
      else if (code == RIGHT_BUTTON) right();
      else if (code == OK_BUTTON) stopMotors();
    }

    IrReceiver.resume();
  }

  if (autoMode) {
    obstacleAvoidance();
  }
}

// ---------------- Motor control ----------------
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ---------------- Ultrasonic ----------------
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000UL); // timeout ~30ms
  if (duration == 0) return -1;
  return duration * 0.034 / 2; // cm
}

// ---------------- Obstacle avoidance ----------------
void obstacleAvoidance() {
  long distance = getDistance();
  if (distance == -1) {
    forward();
    return;
  }

  if (distance > 20) {
    forward();
  } else {
    stopMotors();
    delay(200);
    backward();
    delay(400);
    stopMotors();
    delay(200);
    left();
    delay(400);
    stopMotors();
  }
}

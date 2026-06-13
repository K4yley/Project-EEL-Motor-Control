#include <Arduino.h>

// ================= PINS =================
const int encoderPin = 39;
const int pwmPin = 22;
const int dirPin = 21;

// ================= ENCODER =================
volatile long pulseCount = 0;

// ================= STATE =================
long position = 0;

float speed = 0;
float filteredSpeed = 0;

float targetSpeed = 0;
long targetPosition = 2000;

// ================= TIMING =================
const int controlPeriod = 20;   // 50 Hz (important for stability)
unsigned long lastTime = 0;

// ================= LIMITS =================
float maxSpeed = 300.0;

// ================= PID =================
struct PID {
  float kp, ki, kd;
  float i;
  float last;
};

PID positionPID = {0.18, 0.0, 0.01, 0, 0};
PID speedPID    = {0.5, 0.06, 0.003, 0, 0};

// ================= INTERRUPT =================
void IRAM_ATTR encoderISR() {
  pulseCount++;
}

// ================= PID =================
float computePID(PID &p, float e) {
  float dt = controlPeriod / 1000.0;

  p.i += e * dt;
  p.i = constrain(p.i, -80, 80);

  float d = (e - p.last) / dt;
  p.last = e;

  return p.kp * e + p.ki * p.i + p.kd * d;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(encoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin), encoderISR, RISING);

  pinMode(dirPin, OUTPUT);

  ledcAttach(pwmPin, 20000, 8);

  lastTime = millis();
}

// ================= LOOP =================
void loop() {

  if (millis() - lastTime >= controlPeriod) {

    // ================= READ PULSES (ONLY SOURCE OF SPEED) =================
    noInterrupts();
    long pulses = pulseCount;
    pulseCount = 0;
    interrupts();

    // ================= POSITION =================
    position += pulses;

    // ================= SPEED (CORRECT METHOD) =================
    speed = pulses * (1000.0 / controlPeriod);

    // filter (VERY IMPORTANT)
    filteredSpeed = 0.85 * filteredSpeed + 0.15 * speed;
    speed = filteredSpeed;

    // ================= POSITION CONTROL =================
    float posError = targetPosition - position;

    float posOutput = computePID(positionPID, posError);

    // convert position error → speed request
    targetSpeed = constrain(posOutput, -maxSpeed, maxSpeed);

    // dead zone near target
    if (abs(posError) < 20) {
      targetSpeed = 0;
    }

    // ================= SPEED CONTROL =================
    float speedError = targetSpeed - speed;

    float control = computePID(speedPID, speedError);

    // ================= OUTPUT =================
    if (control >= 0) {
      digitalWrite(dirPin, HIGH);
    } else {
      digitalWrite(dirPin, LOW);
      control = -control;
    }

    control = constrain(control, 0, 255);

    if (control < 35 && targetSpeed != 0) control = 35;

    ledcWrite(pwmPin, (int)control);

    // ================= DEBUG =================
    Serial.print("Pos:");
    Serial.print(position);

    Serial.print(" Target:");
    Serial.print(targetPosition);

    Serial.print(" Speed:");
    Serial.print(speed);

    Serial.print(" TargetSpeed:");
    Serial.print(targetSpeed);

    Serial.print(" PWM:");
    Serial.println(control);

    lastTime = millis();
  }
}
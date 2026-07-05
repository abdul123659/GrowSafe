#include <Servo.h>

const int trigPin = 10;
const int echoPin = 11;
const int servoPin = 12;
const int buzzerPin = 8;

const int greenLED = 3;
const int blueLED = 4;
const int redLED = 13;

Servo myServo;

int distance = 50;
int stableDistance = 50;

int currentState = 0;
// 0 = Grün, 1 = Blau, 2 = Rot

unsigned long lastBeepTime = 0;
bool buzzerOn = false;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  myServo.attach(servoPin);
}

void loop() {
  scanForward();
  scanBackward();
}

void scanForward() {
  for (int angle = 15; angle <= 165; angle += 2) {
    myServo.write(angle);
    delay(25);

    distance = getStableDistance();
    updateState(distance);
    updateLEDs();
    updateBuzzer();
    sendData(angle, distance);
  }
}

void scanBackward() {
  for (int angle = 165; angle >= 15; angle -= 2) {
    myServo.write(angle);
    delay(25);

    distance = getStableDistance();
    updateState(distance);
    updateLEDs();
    updateBuzzer();
    sendData(angle, distance);
  }
}

int getStableDistance() {
  int values[7];

  for (int i = 0; i < 7; i++) {
    values[i] = readDistanceOnce();
    delay(5);
  }

  for (int i = 0; i < 6; i++) {
    for (int j = i + 1; j < 7; j++) {
      if (values[j] < values[i]) {
        int temp = values[i];
        values[i] = values[j];
        values[j] = temp;
      }
    }
  }

  int median = values[3];

  stableDistance = (stableDistance * 3 + median) / 4;

  return stableDistance;
}

int readDistanceOnce() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 15000);

  if (duration == 0) {
    return 50;
  }

  int cm = duration * 0.0343 / 2;

  if (cm > 50) cm = 50;
  if (cm < 2) cm = 2;

  return cm;
}

void updateState(int d) {
  // Hysterese: verhindert Flackern zwischen Farben

  if (currentState == 0) {
    // Grün bleibt ruhig, bis Objekt klar näher als 20 cm ist
    if (d <= 20) currentState = 1;
  }

  else if (currentState == 1) {
    // Blau
    if (d <= 10) currentState = 2;
    else if (d >= 25) currentState = 0;
  }

  else if (currentState == 2) {
    // Rot bleibt, bis Abstand wieder sicher über 13 cm ist
    if (d >= 13) currentState = 1;
  }
}

void updateLEDs() {
  if (currentState == 0) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(blueLED, LOW);
    digitalWrite(redLED, LOW);
  }

  else if (currentState == 1) {
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH);
    digitalWrite(redLED, LOW);
  }

  else if (currentState == 2) {
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

void updateBuzzer() {
  unsigned long now = millis();

  if (currentState == 0) {
    noTone(buzzerPin);
    buzzerOn = false;
    return;
  }

  int interval;
  int frequency;

  if (currentState == 1) {
    interval = 450;
    frequency = 1000;
  } 
  else {
    interval = 100;
    frequency = 2500;
  }

  if (now - lastBeepTime >= interval) {
    lastBeepTime = now;
    buzzerOn = !buzzerOn;

    if (buzzerOn) {
      tone(buzzerPin, frequency);
    } else {
      noTone(buzzerPin);
    }
  }
}

void sendData(int angle, int distance) {
  Serial.print(angle);
  Serial.print(",");
  Serial.print(distance);
  Serial.print(".");
}

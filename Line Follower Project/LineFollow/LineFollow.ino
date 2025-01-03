// Pin sensor garis
const int sensorPin[5] = { 3, 4, 12, 8, 7 };

// Pin motor
const int motorKiriPWM = 5;
const int motorKananPWM = 10;
const int motorKiriDIR = 6;
const int motorKananDIR = 9;

// Pin push button
const int buttonPin = 2;

// Pin LED indikator
const int ledPin = 13;

// Variabel kontrol
bool isRunning = false;
bool lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Variabel LED blink
unsigned long lastLedBlinkTime = 0;
bool ledState = LOW;
const unsigned long ledBlinkInterval = 500;  // Interval kedipan LED dalam milidetik

// Variabel PID
float Kp = 0.7;
float Ki = 0.0;
float Kd = 0.01;
float previousError = 0;
float integral = 0;
int baseSpeed = 175;
int maxSpeed = 255;
int turnSpeed = 225;

void setup() {
  // Inisialisasi sensor garis sebagai input
  for (int i = 0; i < 5; i++) {
    pinMode(sensorPin[i], INPUT);
  }

  // Inisialisasi pin motor, button, LED, dan fire sensor
  pinMode(motorKiriPWM, OUTPUT);
  pinMode(motorKananPWM, OUTPUT);
  pinMode(motorKiriDIR, OUTPUT);
  pinMode(motorKananDIR, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Inisialisasi pin LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Mematikan LED di awal
}

void loop() {

  /*
  // Membaca status push button dengan debounce berbasis millis()
  bool buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && lastButtonState == HIGH) {
      isRunning = !isRunning;  // Toggle status isRunning
    }
  }
  lastButtonState = buttonState;
  */

  lineFollowing();
  /*
  analogWrite(motorKiriPWM, 0);
  analogWrite(motorKiriDIR, turnSpeed);  // Maju
  analogWrite(motorKananPWM, 0);
  analogWrite(motorKananDIR, turnSpeed);  // Mundur
  */
}
// Tambahkan variabel fuzzy
float fuzzyCorrection = 0;

// Fungsi untuk menghitung nilai fuzzy correction
float calculateFuzzyCorrection(int position) {
  float correction = 0;

  // Tingkatkan koreksi untuk posisi jauh dari tengah
  if (position <= -2) {
    correction = 4.0;  // Koreksi lebih besar saat sangat jauh ke kiri
  } else if (position < 0 && position > -2) {
    correction = 4.0;  // Koreksi sedang saat agak jauh ke kiri
  } else if (position == 0) {
    correction = 1.0;  // Koreksi nol saat di tengah
  } else if (position > 0 && position < 2) {
    correction = 4.0;  // Koreksi sedang saat agak jauh ke kanan
  } else if (position >= 2) {
    correction = 4.0;  // Koreksi besar saat sangat jauh ke kanan
  }

  return correction;
}

// Tambahkan variabel untuk menyimpan sensor terakhir yang mendeteksi garis
int lastDetectedPosition = 0;

void lineFollowing() {
  int sensorValue[5];
  sensorValue[0] = digitalRead(sensorPin[0]);
  sensorValue[1] = digitalRead(sensorPin[1]);
  sensorValue[2] = digitalRead(sensorPin[2]);
  sensorValue[3] = digitalRead(sensorPin[3]);
  sensorValue[4] = digitalRead(sensorPin[4]);

  bool allSensorsDetectedLine = true;
  for (int i = 0; i < 5; i++) {
    if (sensorValue[i] == 0) {
      allSensorsDetectedLine = false;
      break;
    }
  }

  if (allSensorsDetectedLine) {
    berhenti();
  } else {
    int position = 0;
    int sensorWeight[5] = { -2, -2, 0, 2, 2 };
    int activeSensors = 0;
    bool lineDetected = false;

    for (int i = 0; i < 5; i++) {
      if (sensorValue[i] == 1) {
        position += sensorWeight[i];
        activeSensors++;
        lineDetected = true;
      }
    }

    // Periksa jika garis terdeteksi, simpan posisi sensor terakhir
    if (lineDetected) {
      if (activeSensors > 0) {
        position /= activeSensors;
      } else {
        position = 0;
      }
      lastDetectedPosition = position;
    } else {
      // Jika garis hilang, gunakan posisi terakhir untuk mencari garis
      if (lastDetectedPosition < 0) {
        // Gerakkan motor ke kiri jika garis terakhir terdeteksi di kiri
        digitalWrite(motorKiriPWM, LOW);
        analogWrite(motorKiriDIR, turnSpeed);  // Mundur
        analogWrite(motorKananPWM, turnSpeed);
        digitalWrite(motorKananDIR, LOW);  // Maju
      } else if (lastDetectedPosition > 0) {
        // Gerakkan motor ke kanan jika garis terakhir terdeteksi di kanan
        analogWrite(motorKiriPWM, turnSpeed);
        digitalWrite(motorKiriDIR, LOW);  // Maju
        digitalWrite(motorKananPWM, LOW);
        analogWrite(motorKananDIR, turnSpeed);  // Mundur
      }
      return;
    }


    // PID kontrol dan fuzzy correction jika garis masih terdeteksi
    float error = position * 10;
    integral += error;
    float derivative = error - previousError;
    float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
    previousError = error;

    fuzzyCorrection = calculateFuzzyCorrection(position);
    correction *= fuzzyCorrection;

    int speedLeft = baseSpeed + correction;
    int speedRight = baseSpeed - correction;

    speedLeft = constrain(speedLeft, 0, maxSpeed);
    speedRight = constrain(speedRight, 0, maxSpeed);

    analogWrite(motorKiriPWM, speedLeft);
    digitalWrite(motorKiriDIR, LOW);
    analogWrite(motorKananPWM, speedRight);
    digitalWrite(motorKananDIR, LOW);
  }
}


void berhenti() {
  // Hentikan motor dan aktifkan LED blink
  analogWrite(motorKiriPWM, 0);
  analogWrite(motorKananPWM, 0);
  ledBlink();
}

void ledBlink() {
  // Fungsi non-blocking untuk kedipan LED
  if (millis() - lastLedBlinkTime >= ledBlinkInterval) {
    lastLedBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}
#include <Servo.h>  // Servo motor library

// Define pin connections and initial variables
// catatan untuk lihat kiri kana lihat dari belakang sensor bukan  sensor
const int ldrtopr = A3;  
const int ldrtopl = A2;  
const int ldrbotr = A1;  
const int ldrbotl = A0;  
const int kipas = 7;
int topl = 0, topr = 0, botl = 0, botr = 0;
int threshold_value = 20;  // Measurement sensitivity threshold
int threshold = 900;

Servo servo_updown;       // Servo motor for up-down movement
Servo servo_rightleft;    // Servo motor for right-left movement

void setup() {
  Serial.begin(9600);       // Serial connection setup (for debugging, optional)
  pinMode(kipas, OUTPUT);
  servo_updown.attach(6);   // Servo motor pin for up-down movement
  servo_rightleft.attach(5); // Servo motor pin for right-left movement
  digitalWrite(kipas, HIGH);
  servo_updown.write(90);
  servo_rightleft.write(90);
}

void loop() {
  automaticsolartracker();
  Serial.print(analogRead(A0));
  Serial.print("\t");
  Serial.print(analogRead(A1));
  Serial.print("\t");
  Serial.print(analogRead(A2));
  Serial.print("\t");
  Serial.print(analogRead(A3));
  Serial.print("\n");
  pemadam();
}

void pemadam() {
    if (ldrtopr < threshold || ldrtopl < threshold || ldrbotr < threshold || ldrbotl < threshold) {
    digitalWrite(kipas, LOW); // Kipas menyala (relay aktif)
    
  } else {
    digitalWrite(kipas, HIGH); // Kipas mati (relay non-aktif)
  }
}

void automaticsolartracker() {
  // Read LDR values
  topr = analogRead(ldrtopr);
  topl = analogRead(ldrtopl);
  botr = analogRead(ldrbotr);
  botl = analogRead(ldrbotl);

  // Calculate average values for top, bottom, left, and right LDRs
  int avgtop = (topr + topl) / 2;
  int avgbot = (botr + botl) / 2;
  int avgleft = (topl + botl) / 2;
  int avgright = (topr + botr) / 2;

  // Calculate differences
  int diffelev = avgtop - avgbot;
  int diffazi = avgright - avgleft;

  // Left-right movement
  if (abs(diffazi) >= threshold_value) {
    if (diffazi > 0 && servo_rightleft.read() < 180) {
      servo_rightleft.write(servo_rightleft.read() + 2);
    } else if (diffazi < 0 && servo_rightleft.read() > 0) {
      servo_rightleft.write(servo_rightleft.read() - 2);
    }
  }

  // Up-down movement
  if (abs(diffelev) >= threshold_value) {
    if (diffelev < 0 && servo_updown.read() < 180) {
      servo_updown.write(servo_updown.read() + 2);
    } else if (diffelev > 0 && servo_updown.read() > 0) {
      servo_updown.write(servo_updown.read() - 2);
    }
  }
}
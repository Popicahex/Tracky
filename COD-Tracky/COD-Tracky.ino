#include "IR_remote.h"  // Include biblioteca pentru controlul telecomenzii IR
#include "keymap.h"     // Include biblioteca pentru maparea tastelor telecomenzii IR
#include <Servo.h>      // Include biblioteca pentru controlul servomotorului
#include "PS2X_lib.h"   //for v1.6


Servo   servoD7, servoD9, servoD8;
PS2X ps2x;                              // create PS2 Controller Class
IRremote ir(3);                         // controlul telecomenzii IR, inițializat pe pinul 3
volatile char IR_Car_Mode = ' ';        // Mod de control al robotului (primit prin IR)
volatile boolean IR_Mode_Flag = false;  // Flag pentru indicarea modului de control IR
volatile int Front_Distance;            // Variabilă pentru distanța frontală (senzor ultrasonic)
bool ultrasonicAvoidanceMode = false;   // Mod de evitare a obstacolelor cu senzor ultrasonic
bool ultrasonicFollowMode = false;      // Mod de urmărire a persoanelor cu senzor ultrasonic
volatile boolean continuous_mode = false; // Flag pentru indicarea modului continuu
volatile boolean ldrMode = false;       // Flag pentru modul LDR

const int ldrPin2 = A1; // Pinul pentru al doilea senzor LDR
const int ldrPin3 = A2; // Pinul pentru al treilea senzor LDR

const int LDR_MIN = 800; // Prag minim de lumină
const int LDR_MAX = 1023; // Prag maxim de lumină

// Variabile pentru stocarea unghiurilor servomotoarelor
int angleD8 = 90;
int angleD7 = 90;
int angleD9 = 90;

// Variabile pentru detectarea tranziției de la „neapăsat” la „apăsat”
bool lastButtonState_RED = false;
bool lastButtonState_PINK = false;
bool lastButtonState_GREEN = false;
bool lastButtonState_BLUE = false;

int error = 0;
byte type = 0;
byte vibrate = 0;
// Declarații funcții
void STOP();
void Move_Forward(int car_speed);
void Move_Backward(int car_speed);
void Rotate_Left(int car_speed);
void Rotate_Right(int car_speed);
void IR_remote_control();
float checkdistance();
void Ultrasonic_Avoidance();
void Ultrasonic_Follow();

#define BUZZER_PIN A3

void playTone(int frequency, int duration) {
  long period = 1000000L / frequency; 
  long cycles = (long)duration * 1000L / period; 

  for (long i = 0; i < cycles; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(period / 2);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(period / 2);
  }
}


void startupSound() {
  pinMode(BUZZER_PIN, OUTPUT);

playTone(440, 150); // A4 - Ton scurt de start
  delay(100);

  playTone(523, 150); // C5 - Ton scurt pentru confirmare
  delay(100);

  playTone(440, 150); // A4 - Ton scurt pentru finalizare
  delay(100);
}


void setup() {

 startupSound();

  servoD7.attach(7);
  servoD9.attach(9);
  servoD8.attach(8);
  

  servoD7.write(90);
  servoD9.write(90);
  servoD8.write(0);

 
  pinMode(2, OUTPUT);  
  pinMode(5, OUTPUT);  

  pinMode(4, OUTPUT);  
  pinMode(6, OUTPUT);  

  pinMode(12, OUTPUT);
  pinMode(13, INPUT);

  pinMode(ldrPin2, INPUT); 
  pinMode(ldrPin3, INPUT); 

  delay(0);
  STOP();  
  Serial.begin(115200);

 delay(500);

  error = ps2x.config_gamepad(11,18,14,10, true, true);

  if (error == 0) {
    Serial.println("Found Controller, configured successful");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
  }

  else if (error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");

  else if (error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if (error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");

  type = ps2x.readType();

  switch (type) {
    case 0:
      Serial.println("Unknown Controller type");
      break;
  }
}

void loop() {

   // Controlul cu telecomanda IR
  IR_remote_control();

  // Modul continuu
  if (continuous_mode) {
    Front_Distance = checkdistance(); 
    if (Front_Distance <= 10) { 
      Move_Backward(255);
      delay(200);
       playTone(1000, 200);
      if (50 >= random(1, 100)) {
        Rotate_Left(150);
      } else {
        Rotate_Right(150);
      }
      delay(500);
    } else if (Front_Distance >= 10 && Front_Distance <= 20) { 
      STOP();
      delay(200);
      playTone(800, 200); 
      if (50 >= random(1, 100)) {
        Rotate_Left(150);
      } else {
        Rotate_Right(150);
      }
      delay(200);
    } else if (Front_Distance > 20 && Front_Distance <= 30) { 
      Move_Forward(150);
      delay(150);
        playTone(600, 200); 
    } else if (Front_Distance > 30) { 
      STOP();
    }
  }

if (ldrMode) {
  int ldrLeft = analogRead(ldrPin2);
  int ldrRight = analogRead(ldrPin3);

  const int sensitivity = 50;

  if (ldrLeft > LDR_MIN || ldrRight > LDR_MIN) {
    int diff = ldrLeft - ldrRight;

    if (abs(diff) < sensitivity) {
      STOP(); 
    } else if (diff > 0) {
      Rotate_Left(150); 
    } else {
      Rotate_Right(150); 
    }
  } else {
    STOP(); 
  }

  delay(100);
}

  ps2x.read_gamepad(false, vibrate);  


  bool R1Pressed = ps2x.Button(PSB_R1);

  auto updateServo = [&R1Pressed](bool currentButton, bool &lastButtonState, int &angle, Servo &servo) {
    if (currentButton && !lastButtonState) {  
      if (R1Pressed) {
        angle = constrain(angle - 15, 0, 180); 
      } else {
        angle = constrain(angle + 15, 0, 180);  
      }
      servo.write(angle);
    }
    lastButtonState = currentButton;
  };
  

  updateServo(ps2x.Button(PSB_RED), lastButtonState_RED, angleD8, servoD8);
  updateServo(ps2x.Button(PSB_GREEN), lastButtonState_GREEN, angleD7, servoD7);
  updateServo(ps2x.Button(PSB_BLUE), lastButtonState_BLUE, angleD9, servoD9);

ps2x.read_gamepad(false, 0); // Citim datele de la controller

   if (ps2x.Button(PSB_PAD_UP)) {  
    delay(50);  // Întârziere pentru debounce
    if (ps2x.Button(PSB_PAD_UP)) {  Serial.println("UP is being held"); Move_Forward(255); delay(100); STOP();  }
  }

  if (ps2x.Button(PSB_PAD_RIGHT)) {
    delay(50);
    if (ps2x.Button(PSB_PAD_RIGHT)) { Serial.println("Right is being held"); Rotate_Right(255);  delay(100); STOP(); }
  }

  if (ps2x.Button(PSB_PAD_LEFT)) {
    delay(50);
    if (ps2x.Button(PSB_PAD_LEFT)) { Serial.println("LEFT is being held"); Rotate_Left(255); delay(100); STOP(); }
  }

  if (ps2x.Button(PSB_PAD_DOWN)) {
    delay(50);
    if (ps2x.Button(PSB_PAD_DOWN)) { Serial.println("DOWN is being held"); Move_Backward(255); delay(100); STOP(); }
  }


ps2x.read_gamepad(false, false); // Citim datele de la controller

  if (ps2x.NewButtonState()) 
  {
    if (ps2x.Button(PSB_L3))
      Serial.println("L3 pressed");
    if (ps2x.Button(PSB_R3))
      Serial.println("R3 pressed");
    if (ps2x.Button(PSB_L2))
      Serial.println("L2 pressed");
    if (ps2x.Button(PSB_R2))
      Serial.println("R2 pressed");
  }
  if (ps2x.Button(PSB_START)) {
  }
  delay(50); // Mică întârziere pentru stabilitate
}
void IR_remote_control() {
  int keyCode = ir.getIrKey(ir.getCode(), 1);

  if (keyCode == IR_KEYCODE_UP) IR_Car_Mode = 'f';
  if (keyCode == IR_KEYCODE_LEFT) IR_Car_Mode = 'l';
  if (keyCode == IR_KEYCODE_DOWN) IR_Car_Mode = 'b';
  if (keyCode == IR_KEYCODE_RIGHT) IR_Car_Mode = 'r';
  if (keyCode == IR_KEYCODE_OK) {
    continuous_mode = false;
    ldrMode = false;
    STOP();
  }

  if (keyCode == IR_KEYCODE_2) continuous_mode = true;
  if (keyCode == IR_KEYCODE_STAR) ldrMode = true;

  if (keyCode == IR_KEYCODE_2) {
    continuous_mode = true;
  }

 if (keyCode == IR_KEYCODE_5) { angleD9 += 15;
  if (angleD9 > 180) angleD9 = 180; servoD9.write(angleD9); delay(200);
 }
 if (keyCode == IR_KEYCODE_6) { angleD9 -= 15;
  if (angleD9 < 0) angleD9 = 0; servoD9.write(angleD9); delay(200);
 }
 if (keyCode == IR_KEYCODE_8) { angleD8 -= 15;
  if (angleD8 < 0) angleD8 = 0; servoD8.write(angleD8); delay(200);
 }
 if (keyCode == IR_KEYCODE_9) { angleD8 += 15;
  if (angleD8 > 180) angleD8 = 180; servoD8.write(angleD8); delay(200);
 }
 if (keyCode == IR_KEYCODE_0) { angleD7 += 15;
  if (angleD7 > 180) angleD7 = 180; servoD7.write(angleD7); delay(200);
 }  
 if (keyCode == IR_KEYCODE_POUND) { angleD7 -= 15;
  if (angleD7 < 0) angleD7 = 0; servoD7.write(angleD7); delay(200);
 }

  switch (IR_Car_Mode) {
    case 'b': Move_Backward(255); delay(100); STOP(); break;
    case 'f': Move_Forward(255); delay(100); STOP(); break;
    case 'l': Rotate_Left(255); delay(100); STOP(); break;
    case 'r': Rotate_Right(255); delay(100); STOP(); break;
    case 's': STOP(); break;
  }
  IR_Car_Mode = ' ';
}
void Move_Forward(int car_speed = 255) {
  // Motor A2 (stânga) - înainte
  digitalWrite(2, HIGH);      // Direcție înainte
  analogWrite(5, car_speed);  // Viteză motor A2
  // Motor B1 (dreapta) - înainte
  digitalWrite(4, LOW);       // Direcție înainte (inversat)
  analogWrite(6, car_speed);  // Viteză motor B1
}

void Move_Backward(int car_speed = 255) {
  // Motor A2 (stânga) - înapoi
  digitalWrite(2, LOW);       // Direcție înapoi
  analogWrite(5, car_speed);  // Viteză motor A2
  // Motor B1 (dreapta) - înapoi
  digitalWrite(4, HIGH);      // Direcție înapoi (inversat)
  analogWrite(6, car_speed);  // Viteză motor B1
}

void Rotate_Left(int car_speed = 255) {
  // Motor A2 (stânga) - înainte
  digitalWrite(2, HIGH);      // Direcție înainte
  analogWrite(5, car_speed);  // Viteză motor A2
  // Motor B1 (dreapta) - înapoi
  digitalWrite(4, HIGH);      // Direcție înapoi (inversat)
  analogWrite(6, car_speed);  // Viteză motor B1
}

void Rotate_Right(int car_speed = 255) {
  // Motor A2 (stânga) - înapoi
  digitalWrite(2, LOW);       // Direcție înapoi
  analogWrite(5, car_speed);  // Viteză motor A2
  // Motor B1 (dreapta) - înainte
  digitalWrite(4, LOW);       // Direcție înainte (inversat)
  analogWrite(6, car_speed);  // Viteză motor B1
}

void STOP() {
  // Oprire motor A2 (stânga)
  analogWrite(5, 0);     // Viteză 0
  digitalWrite(2, LOW);  // Direcție oprită
  // Oprire motor B1 (dreapta)
  analogWrite(6, 0);     // Viteză 0
  digitalWrite(4, LOW);  // Direcție oprită
}
float checkdistance() {
  digitalWrite(12, LOW); delayMicroseconds(2); digitalWrite(12, HIGH); delayMicroseconds(10); digitalWrite(12, LOW);
  unsigned long duration = pulseIn(13, HIGH);
  float distance = (duration * 0.0343) / 2;
  return distance;
}
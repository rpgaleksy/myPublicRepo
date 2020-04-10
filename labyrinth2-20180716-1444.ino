// ___________________________________________________________________________
// _______________________________START OF FILE_______________________________

// ___________________________________________________________________________
// _______________________________ToDo or ToFix_______________________________
/*
 - nothing at the momentz
*/


// ___________________________________________________________________________
// ________________________________CONNECTIONS________________________________

/* Bluetooth Connections:
 RX : TX
 TX : RX

 Motor Connections:
 A1 : 10
 A2 : 6
 B1 : 9
 B2 : 5

 Ultrasound Connections:
 U1 : 11
 U2 : 12
 U2 : 13

 LED Connections:
 ledLEFT : 2
 ledMID : 3
 ledRIGHT : 4
 ledSTUCKBOMB : 8
 
*/


// ___________________________________________________________________________
// ________________________________DEFINITIONS________________________________

// Motors
#define MOTORA2 6
#define MOTORA1 10
#define MOTORB2 5
#define MOTORB1 9

// Ultrasound
#define U1 11
#define U2 12
#define U3 13

// LEDs
#define ledLEFT 2
#define ledMID 3
#define ledRIGHT 4
#define ledSTUCKBOMB 8

// Bluetooth
// input buffer (of size BUF_SIZE). Cyclycally written and stored
#define BUF_SIZE 30
char charBuf[BUF_SIZE];
unsigned int bufPos = 0;
// buffer to store the 9 last chars (to test against "Hello\r\n"), '0' terminated
char testStringHello[9];

// mode:
// 0: waiting for hello
// 1: hello received, waiting for code
unsigned int mode = 0;

// temporary variable
char temp;


// ___________________________________________________________________________
// ___________________________________SETUP___________________________________

void setup() {
  Serial.begin(38400);
  pinMode(MOTORA1, OUTPUT);
  pinMode(MOTORA2, OUTPUT);
  pinMode(MOTORB1, OUTPUT);
  pinMode(MOTORB2, OUTPUT);
  pinMode(ledLEFT, OUTPUT);
  pinMode(ledMID, OUTPUT);
  pinMode(ledRIGHT, OUTPUT);
  pinMode(ledSTUCKBOMB, OUTPUT);

  // bluetooth setup
  // set AT Mode switch to "on" and excecute this once
  // set AT Mode switch to "off" and remove follwing line

  // digitalWrite(ledMID, HIGH);
  // programBluetooth();

  // clear the char buf
  for (int i = 0; i < BUF_SIZE; i++) {
    charBuf[i] = ' ';
  }

  /*
  lcd.begin(NUM_CHAR, NUM_LINES);
  lcd.print("system ready");
  delay(900);
  lcd.clear();
  */
  delay(1000);
}


// ___________________________________________________________________________
// _________________________________MAIN LOOP_________________________________

void loop() {
  // int analogValue = analogRead(A0);
  if(analogRead(A0) > 350 && analogRead(A0) < 550) {
    driveNoTouch();
  }
}


// ___________________________________________________________________________
// _________________________MAIN DRIVE DECIDE FUNCTION________________________

void driveNoTouch() {
  // If lpDEBUG == true motors won't turn to save power during debugging.
  bool lpDEBUG = false;
  
  uint16_t stuckcounter = 1;
  int allowedstucktime = 20;
  bool notstuck = true;
  
  while (true) {
    uint16_t distMID = measureDistanceMID();
    delay(25);
    uint16_t distLEFT = measureDistanceLEFT();
    uint16_t distRIGHT = measureDistanceRIGHT();

    /*
      // CONDITION SEQEUNCE:
      - In front of wall?
      - Moving towards a wall?
      - Moving towards wall with left side?
      - Moving towards wall with right side?
    */
    if (notstuck) {
      if (distMID < 12) {
        // Robot is directly in front of a wall.
        // Drive backward a bit.
        stuckcounter = +1;
        if (!lpDEBUG) {
          driveBackward(100, 50);
        }
        digitalWrite(ledLEFT, LOW);
        digitalWrite(ledMID, HIGH);
        digitalWrite(ledRIGHT, LOW);
        
      } else if (distMID < 35) {
        // Robot is moving towards a wall.
        // Drive according curve to avoid it.
        stuckcounter += 1;

        digitalWrite(ledMID, HIGH);
        
        if (distLEFT < distRIGHT) {
          if (!lpDEBUG) {
            driveCurveRight(100, 25);
          }
          digitalWrite(ledLEFT, LOW);
          digitalWrite(ledRIGHT, HIGH);
        } else {
          if (!lpDEBUG) {
            driveCurveLeft(100, 25);
          }
          digitalWrite(ledLEFT, HIGH);
          digitalWrite(ledRIGHT, LOW);
        }
        
      } else if (distLEFT <= 15 && distRIGHT > 15) {
        // Robot is too far left. Turn right a bit.
        stuckcounter += 1;
        if (!lpDEBUG) {
          driveCurveRight(100, 25);
        }
        
        digitalWrite(ledLEFT, LOW);
        digitalWrite(ledMID, LOW);
        digitalWrite(ledRIGHT, HIGH);
        
      } else if (distRIGHT <= 15 && distLEFT > 15) {
        // Robot is too far right. Turn left a bit.
        stuckcounter += 1;
        if (!lpDEBUG) {
          driveCurveLeft(100, 25);
        }

        digitalWrite(ledLEFT, HIGH);
        digitalWrite(ledMID, LOW);
        digitalWrite(ledRIGHT, LOW);
        
      } else {
        // Drive forward.
        stuckcounter = 1;
        if (!lpDEBUG) {
          driveForward(100, 25);
        }
        
        digitalWrite(ledLEFT, LOW);
        digitalWrite(ledMID, LOW);
        digitalWrite(ledRIGHT, LOW);
        
      }
    } else {
      // Drive back a bit if happen to be stuck.
      if (!lpDEBUG) {
        driveBackward(200, 100);
      }
      
      digitalWrite(ledLEFT, HIGH);
      digitalWrite(ledMID, HIGH);
      digitalWrite(ledRIGHT, HIGH);

      if (distLEFT < distRIGHT) {
          if (!lpDEBUG) {
            driveCurveRight(100, 50);
          }
          digitalWrite(ledLEFT, LOW);
          digitalWrite(ledRIGHT, HIGH);
        } else {
          if (!lpDEBUG) {
            driveCurveLeft(100, 50);
          }
          digitalWrite(ledLEFT, HIGH);
          digitalWrite(ledRIGHT, LOW);
        }
      
      stuckcounter = 1;
      notstuck = true;
    }

    if (stuckcounter < allowedstucktime) {
      digitalWrite(ledSTUCKBOMB, HIGH);
      delay(allowedstucktime / stuckcounter);
      digitalWrite(ledSTUCKBOMB, LOW);
    } else {
      digitalWrite(ledSTUCKBOMB, HIGH);
    }

    // Check if stuck according to counter
    if (stuckcounter > allowedstucktime) {
      notstuck = false;
    }
  }
}


// ___________________________________________________________________________
// _________________________DISTANCE MEASURE FUNCTIONS________________________

int divconstant = 35;
int obstacleconstant = 24;

uint16_t measureDistanceLEFT() {
  pinMode(U1, OUTPUT);
  digitalWrite(U1, LOW);
  digitalWrite(U1, HIGH);
  int i = micros() + 15;
  int j = micros();
  while (i > j) {j = micros();}
  digitalWrite(U1, LOW);
  pinMode(U1, INPUT);
  uint32_t imptime = micros();
  while (digitalRead(U1) == LOW) {
    if (micros() - imptime > obstacleconstant * 1000) {
      // No obstacle if no signal after *obstacleconstant ms
      return 50;
    }
  }
  imptime = micros();
  while (digitalRead(U1) == HIGH) {}
  imptime = micros() - imptime;
  uint16_t cm = imptime / divconstant;
  return cm;
}

uint16_t measureDistanceMID() {
  pinMode(U2, OUTPUT);
  digitalWrite(U2, LOW);
  digitalWrite(U2, HIGH);
  int i = micros() + 15;
  int j = micros();
  while (i > j) {j = micros();}
  digitalWrite(U2, LOW);
  pinMode(U2, INPUT);
  uint32_t imptime = micros();
  while (digitalRead(U2) == LOW) {
    if (micros() - imptime > obstacleconstant * 1000) {
      // No obstacle if no signal after *obstacleconstant ms
      return 50;
    }
  }
  imptime = micros();
  while (digitalRead(U2) == HIGH) {}
  imptime = micros() - imptime;
  uint16_t cm = imptime / divconstant;
  return cm;
}

uint16_t measureDistanceRIGHT() {
  pinMode(U3, OUTPUT);
  digitalWrite(U3, LOW);
  digitalWrite(U3, HIGH);
  int i = micros() + 15;
  int j = micros();
  while (i > j) {j = micros();}
  digitalWrite(U3, LOW);
  pinMode(U3, INPUT);
  uint32_t imptime = micros();
  while (digitalRead(U3) == LOW) {
    if (micros() - imptime > obstacleconstant * 1000) {
      // No obstacle if no signal after *obstacleconstant ms
      return 50; //
    }
  }
  imptime = micros();
  while (digitalRead(U3) == HIGH) {}
  imptime = micros() - imptime;
  uint16_t cm = imptime / divconstant;
  return cm;
}


// ___________________________________________________________________________
// __________________________SEPERATE DRIVE FUNCTIONS_________________________

void driveForward(int speed, int milliseconds) {
  analogWrite(MOTORA1, speed);
  digitalWrite(MOTORA2, LOW);
  analogWrite(MOTORB1, speed);
  digitalWrite(MOTORB2, LOW);
  delay(milliseconds);
}

void driveBackward(int speed, int milliseconds) {
  analogWrite(MOTORA2, speed);
  digitalWrite(MOTORA1, LOW);
  analogWrite(MOTORB2, speed);
  digitalWrite(MOTORB1, LOW);
  delay(milliseconds);
}

void driveCurveRight(int speed, int milliseconds) {
  analogWrite(MOTORA1, speed);
  digitalWrite(MOTORA2, LOW);
  analogWrite(MOTORB2, speed);
  digitalWrite(MOTORB1, LOW);
  delay(milliseconds);
}

void driveCurveLeft(int speed, int milliseconds) {
  analogWrite(MOTORA2, speed);
  digitalWrite(MOTORA1, LOW);
  analogWrite(MOTORB1, speed);
  digitalWrite(MOTORB2, LOW);
  delay(milliseconds);
}

// ___________________________________________________________________________
// ________________________________BLUETOOTHSETUP_____________________________
/*
DON'T MODIFY CODE BELOW THIS LINE! (commented out lcd)
*/

void programBluetooth() {
  //Serial.begin(9600);
  //Serial.begin(57600);

  Serial.begin(38400);
  delay(1000);


  boolean programmingSuccessful = true;
  programmingSuccessful &= sendCommand("AT+ORGL\r\n");
  delay (1000);
  programmingSuccessful &= sendCommand("AT+CLASS=001F00\r\n");

  programmingSuccessful &= sendCommand("AT+RESET\r\n");

  /*
  lcd.setCursor(0,0);
  lcd.print("Programming BT:");
  lcd.setCursor(0,1);
  */
  digitalWrite(ledMID, LOW);
  if (!programmingSuccessful) {
    // lcd.print("NOT successful!");
    // digitalWrite(13, LOW);
    digitalWrite(ledRIGHT, HIGH);
    return;
  }
  digitalWrite(ledLEFT, HIGH);
  /*
  digitalWrite(13, HIGH);
  lcd.print("successful!");
  */
}

// Sends a command to the bluetooth module
boolean sendCommand(String command) {
  Serial.print(command);

  delay(200);

  int state = 0;
  while (Serial.available() > 0) {
    char temp = Serial.read();
    if (temp == 'O' && state == 0) {
      state = 1;
    }
    if (temp == 'K' && state == 1) {
      state = 2;
    }
    if (temp == '\r' && state == 2) {
      state = 3;
    }
    if (temp == '\n' && state == 3) {
      state = 4;
    }
  }
  return (state == 4);
}

// ___________________________________________________________________________
// ________________________________COMMENT DUMP_______________________________

/*
#define R_S 2
#define E   3
#define DB4 4
#define DB5 5
#define DB6 7
#define DB7 8
#define NUM_CHAR 20
#define NUM_LINES 4

#include <LiquidCrystal.h>

LiquidCrystal lcd(R_S, E, DB4, DB5, DB6, DB7);
*/

/*
// For reference the values of measureDistance.
lcd.clear();
lcd.setCursor(0, 0);
lcd.print(dist1);
lcd.setCursor(0, 1);
lcd.print(dist2);
lcd.setCursor(0, 2);
lcd.print(dist3);
*/


// ___________________________________________________________________________
// ________________________________END OF FILE________________________________

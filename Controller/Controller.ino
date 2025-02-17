#include <Servo.h>
#include <SoftwareSerial.h>

// Servo myservo; // (currently commented out)
int knob, motion, notion = 0, i = 0, count = 10, value = 1, count2 = 0, count1 = 0, stime = 0, angle = 90, add = 1, lightcnt1 = 0, lightcnt2 = 0, mainlightcnt = 0;

#define RX 12   // CONFIRMED
#define TX 9    // CONFIRMED

SoftwareSerial Sentinal = SoftwareSerial(RX, TX);

void setup() {
  // myservo.attach(11);  // (currently commented out)

  pinMode(A3, INPUT);
  pinMode(A6, INPUT);
  pinMode(4, OUTPUT);   // main
  pinMode(2, OUTPUT);   // submain
  pinMode(6, OUTPUT);   // light2
  digitalWrite(6, HIGH); // Ensure light2 is OFF initially
  pinMode(7, OUTPUT);   // fan
  digitalWrite(7, HIGH);
  digitalWrite(2, HIGH);

  pinMode(10, OUTPUT);  // light1 (main light)
  digitalWrite(10, LOW);

  pinMode(13, OUTPUT);  // on-board LED for debugging
  digitalWrite(13, LOW);
  delay(6000);
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);

  Serial.begin(9600);
  Sentinal.begin(9600);
}

int ldrcnt = 0, systemMode = 1;
char receivedChar;

// Function to reset the system
void resetSystem() {
  Serial.println("System reset initiated...");
  delay(1000);
  asm volatile ("jmp 0");  // Software reset (AVR)
}

void loop() {
  receivedChar = Sentinal.read();

  // Handle commands from NodeMCU
  switch (receivedChar) {
    case 'N': systemMode = 0; break;
    case 'n': systemMode = 1; break;
    case 'L': digitalWrite(6, LOW); break;  // Turn light2 ON
    case 'l': digitalWrite(6, HIGH); break; // Turn light2 OFF
    case 'C': 
      mainlightcnt = 5000;  // Extend the light duration
      Serial.println("Motion continued, light duration extended");
      break;
    case 'R': 
      resetSystem();  // Reset the system
      break;
  }

  // Check if LDR detects enough light
  if (analogRead(A6) > 100) {
    ldrcnt++;
    if (ldrcnt >= 2) {
      ldrcnt = 0;
      systemMode = (systemMode + 1) % 3;
    }
  } else {
    ldrcnt = 0;
  }

  // Handle motion and lights
  if (systemMode != 2) {
    digitalWrite(4, HIGH);  // main light stays on

    if (analogRead(A3) < 100)
      lightcnt2 = 0;
    lightcnt2++;

    if (lightcnt2 > 15) {
      digitalWrite(13, LOW);  // Indicate movement
      mainlightcnt = 1000;   // Reset main light timer
    } else {
      digitalWrite(13, HIGH);  // No movement detected
    }

    if (mainlightcnt > 0) {
      mainlightcnt--;
      digitalWrite(10, systemMode);  // Control pin 10 (main light)

      // Notify motion detection
      if (systemMode == 1) {
        Sentinal.println('M');
        if (mainlightcnt < 300) digitalWrite(6, HIGH);
      }
    } else {
      digitalWrite(10, LOW);  // Turn off main light
      digitalWrite(6, HIGH);  // Turn off light2
      Serial.println("Main light and light2 turned off after timeout");
    }

  } else {
    digitalWrite(4, LOW);  // Turn off main light if systemMode = 2
    digitalWrite(10, LOW); // Turn off light1
    lightcnt1++;

    if (lightcnt1 >= 100 && lightcnt1 < 201) {
      digitalWrite(13, LOW);
    } else if (lightcnt1 > 203) {
      digitalWrite(13, HIGH);
      lightcnt1 = 0;
    }
  }

  delay(10);
}

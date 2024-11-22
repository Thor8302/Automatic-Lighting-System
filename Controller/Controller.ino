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
  digitalWrite(6, 1);   // turn off light2 by default
  pinMode(7, OUTPUT);   // fan
  digitalWrite(7, 1);
  digitalWrite(2, 1);

  pinMode(10, OUTPUT);  // light1 (main light)
  digitalWrite(10, 0);

  pinMode(13, OUTPUT);  // on-board LED for debugging
  digitalWrite(13, 0);
  delay(6000);
  digitalWrite(13, 1);
  delay(1000);
  digitalWrite(13, 0);

  Serial.begin(9600);
  Sentinal.begin(9600);
}

int ldrcnt = 0, incognito = 1;
char c;

// Function to reset the system
void resetSystem() {
 // Enable the watchdog timer with a very short timeout
  
  Serial.println("System reset successfully!");
}

void loop() {
  char a = 'a';
  a = Sentinal.read();

  // Handle commands from NodeMCU
  if (a == 'N')
    incognito = 0;
  else if (a == 'n')
    incognito = 1;
  else if (a == 'L')
    digitalWrite(6, 0);  // Turn light2 ON
  else if (a == 'l')
    digitalWrite(6, 1);  // Turn light2 OFF
  else if (a == 'C') {
    mainlightcnt = 5000;  // Extend the light duration
    digitalWrite(6, 0);   // Turn light2 ON during motion continuation
    Serial.println("Motion continued, light duration extended");
  }
  else if (a == 'R') {
    resetSystem(); // Reset the system when 'R' command is received
  }

  // Check if LDR detects enough light
  if (analogRead(A6) > 100) {
    ldrcnt += 1;
    if (ldrcnt >= 2) {
      ldrcnt = 0;
      if (incognito == 0)
        incognito = 1;
      else if (incognito == 1)
        incognito = 2;
      else
        incognito = 0;
    }
  } else {
    ldrcnt = 0;
  }

  // Handle motion and lights
  if (incognito != 2) {
    digitalWrite(4, 1);  // main light stays on

    if (analogRead(A3) < 100)
      lightcnt2 = 0;
    lightcnt2++;

    if (lightcnt2 > 15) {
      digitalWrite(13, 0);  // Indicate movement
       
      mainlightcnt = 1000;   // Reset main light timer
    } else {
      digitalWrite(13, 1);  // No movement detected
    }

    if (mainlightcnt > 0) {
      mainlightcnt--;
      digitalWrite(10, incognito);  // Control pin 10 (main light)

      // Check if motion is detected and send 'M' to NodeMCU
      if (incognito == 1) {
         Sentinal.println('M');  // Notify motion detected
        if(mainlightcnt < 300)
          digitalWrite(6, 1);
      }
    } else {
      // If mainlightcnt is over, turn off both pin 10 (main light) and pin 6 (light2)
      digitalWrite(10, 0);  // Turn off main light
      digitalWrite(6, 1);   // Turn off light2 when main light is off
      Serial.println("Main light and light2 turned off after timeout");
    }

  } else {
    digitalWrite(4, 0);  // Turn off main light if incognito = 2
    digitalWrite(10, 0); // Turn off light1 when incognito = 2
    lightcnt1 += 1;

    if (lightcnt1 >= 100 && lightcnt1 < 201) {
      digitalWrite(13, 0);
    } else if (lightcnt1 > 203) {
      digitalWrite(13, 1);
      lightcnt1 = 0;
    }
  }

  delay(10);
}

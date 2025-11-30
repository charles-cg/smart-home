#include <Servo.h>

Servo servo_1; // servo controller (multiple can exist)

int servo_pin = 11; // PWM pin for servo control
int pos = 0;    // servo starting position

void setup() {
  servo_1.attach(servo_pin); // start servo control
}

void loop() {
  for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo_1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15); // delay to allow the servo to reach the desired position
    if (pos==90){
      delay(5000); //wait 5 seconds once positioned at 90 degrees
    }
  }
  delay(5000); // wait 5 seconds after reaching 180 degrees
 
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo_1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);
    if (pos==90){
      delay(5000); // wait 5 seconds once positioned at 90 degrees
    }
  }
  delay(5000); // wait 5 seconds after arriving back at 0 degrees
}

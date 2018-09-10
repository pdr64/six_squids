#include <Servo.h>

Servo parallax1;
Servo parallax2;

int leftSen = A0;
int centSen = A1;
int rightSen = A2;

int thresh=150;
void setup() {
parallax1.attach(6);
parallax2.attach(5);
  
pinMode(leftSen, INPUT);
pinMode(centSen,INPUT);
pinMode(rightSen,INPUT);
Serial.begin(9600);
Serial.println("Left Sensor Centor Sensor Right Sensor");
}

void loop() {

// correct for veer right
while(analogRead(centSen)>thresh && analogRead(rightSen)>thresh && analogRead(leftSen)<thresh){
  parallax1.write(94);
  parallax2.write(75);     
}
//correct for veer left
while(analogRead(centSen)>thresh && analogRead(rightSen)<thresh && analogRead(leftSen)>thresh){
  parallax1.write(115);
  parallax2.write(94);
}
// go straight
while(analogRead(centSen)<thresh && analogRead(leftSen)>thresh && analogRead(rightSen)>thresh){
  parallax1.write(100);
  parallax2.write(90);
}

// T/fourway intersection
while(analogRead(centSen)>thresh && analogRead(leftSen)>thresh && analogRead(rightSen)>thresh){
  parallax1.write(100);
  parallax2.write(90);
}


Serial.println(analogRead(leftSen)+String("  ")+analogRead(centSen)+String("  ")+analogRead(rightSen));

}

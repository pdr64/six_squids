#include <Servo.h>

Servo parallax1;
Servo parallax2;

int leftSen = A0;
int centSen = A1;
int rightSen = A2;

int thresh=200;
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
if(!(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh)){
Serial.println("I am in loop!"); 
follow_line();

}
turn_left();

/*follow_line();
if(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh){
  turn_right();
}
follow_line();
if(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh){
  turn_right();
}
follow_line();
if(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh){
  turn_right();
}
follow_line();
if(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh){
  turn_right();
}
follow_line();
if(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh){
  turn_left();
}
follow_line();
if(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh){
  turn_left();
}
follow_line();
if(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh){
  turn_left();
}
*/
}

void turn_left(){
  parallax1.write(94);
  parallax2.write(75);
  delay(1000);  
  Serial.println("I am in turn left!"); 
}

void follow_line(){
  // correct for veer right
while(analogRead(centSen)>thresh && analogRead(rightSen)>thresh && analogRead(leftSen)<thresh){
  parallax1.write(94);
  parallax2.write(75);  
  Serial.println("I am in veer Right!");   
  }
//correct for veer left
while(analogRead(centSen)>thresh && analogRead(rightSen)<thresh && analogRead(leftSen)>thresh){
  parallax1.write(115);
  parallax2.write(94);
  Serial.println("I am in veer left!"); 
  }
// go straight
while(analogRead(centSen)<thresh && analogRead(leftSen)>thresh && analogRead(rightSen)>thresh){
  parallax1.write(105);
  parallax2.write(85);
  Serial.println("I am in straight!"); 
  }
}

void turn_right(){
  parallax1.write(115);
  parallax2.write(94);
  delay(1000);
  Serial.println("I am in turn Right!"); 
}

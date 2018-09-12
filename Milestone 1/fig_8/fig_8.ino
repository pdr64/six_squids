#include <Servo.h>

Servo parallax1;
Servo parallax2;

int leftSen = A0;
int centSen = A1;
int rightSen = A2;

int thresh= 150;
int state = 1;

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

Serial.println(analogRead(leftSen)+String("  ")+analogRead(centSen)+String("  ")+analogRead(rightSen));

follow_line();
} else {
 if(state==1 || state == 6 || state==7 || state==8){
    Serial.println(String("turning left ")+state );
    Serial.println(analogRead(leftSen)+String("  ")+analogRead(centSen)+String("  ")+analogRead(rightSen));
    turn_left();
    
    if(state==8){
      state=1;
      } else{
        state++;
      }
 } else if (state==2 || state ==3 || state==4 || state==5){
    Serial.println(String("turning right ")+state);
    Serial.println(analogRead(leftSen)+String("  ")+analogRead(centSen)+String("  ")+analogRead(rightSen));
    turn_right();
    state++;
 }
 
}

}

void turn_left(){
  parallax1.write(94);
  parallax2.write(75);
  delay(1300);  
  
}

void follow_line(){
  // correct for veer right
while(analogRead(centSen)>thresh && analogRead(rightSen)>thresh && analogRead(leftSen)<thresh){
  parallax1.write(94);
  parallax2.write(75);  
  //Serial.println("I am in veer Right!");   
  }
//correct for veer left
while(analogRead(centSen)>thresh && analogRead(rightSen)<thresh && analogRead(leftSen)>thresh){
  parallax1.write(115);
  parallax2.write(94);
  //Serial.println("I am in veer left!"); 
  }
// go straight
while(analogRead(centSen)<thresh && analogRead(leftSen)>thresh && analogRead(rightSen)>thresh){
  parallax1.write(100);
  parallax2.write(90);
  //Serial.println("I am in straight!"); 
  }
}

void turn_right(){
  parallax1.write(115);
  parallax2.write(94);
  delay(1300);
  //Serial.println("I am in turn Right!"); 
}

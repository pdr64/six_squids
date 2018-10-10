#define LOG_OUT 1// use the log output function
#define FFT_N 256// set to 256point fft
#include <FFT.h>
#include <Servo.h>

Servo parallax1;
Servo parallax2;

int leftSen = A0;
int centSen = A1;
int rightSen = A2;
int rightWall = A3;
int frontWall = A4;
int IRsense = A5;

int thresh= 150;
int wallThresh = 300;

void setup() {
parallax1.attach(6);
parallax2.attach(5);
  
pinMode(leftSen, INPUT);
pinMode(centSen,INPUT);
pinMode(rightSen,INPUT);
pinMode(rightWall, INPUT);
pinMode(frontWall, INPUT);
pinMode(IRsense, INPUT);

Serial.begin(9600);
Serial.println("Milestone 2 begin");
}

void loop() {
  while(1){
    int r = check_IR();
    if(!r){ 
      if(!(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh)){
        //Serial.println("clear to move");
        follow_line();
        if((analogRead(frontWall)>wallThresh))//||(analogRead(rightWall)<wallThresh)))
        {
          Serial.println(analogRead(frontWall)+" "+ analogRead(rightWall));
          turn_left(); 
        }
       }    
   }
    else{
      robot_stop();
      delay(1500);
    }
  }
}
int check_IR(){
  cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      fft_input[i] = analogRead(IRsense); // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    if(fft_log_out[76]>45)
      {
        digitalWrite(7, HIGH);
        Serial.println("Audio LED on");
        return 1;
      }
      else
      {
        digitalWrite(7, LOW);
        return 0;
      }
    Serial.println("start audio");
    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      ///Serial.println(fft_log_out[i]); // send out the data
    }
}

void turn_left(){
  parallax1.write(75);
  parallax2.write(75);
  delay(1500);  
  
}
void robot_stop(){
  parallax1.write(90);
  parallax2.write(90);
  delay(1500);
}

void follow_line(){
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
}

void turn_right(){
  parallax1.write(180);
  parallax2.write(94);
  delay(1300);
  //Serial.println("I am in turn Right!"); 
}

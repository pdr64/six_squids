#define LOG_OUT 1// use the log output func  tion
#define FFT_N 256// set to 256point fft
//#include <FFT.h>
#include <Servo.h>
#include <StackArray.h>

/////////////////////

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0x000000000CLL, 0x000000000DLL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back" };
role_e role = role_pong_back;
char dataArray[] = {2, 2, 0, 1, 1, 1}; 
byte dir_facing = 0; 
// 0 = north, 1 = east, 2 = south, 3 = west

char totalSquares[9][9]; //to see if we have visited or not 
StackArray <char> visitStack;

///////////////////

Servo parallax1;
Servo parallax2;

int rightSen = A2;
int centSen  = A3;
int leftSen  = A4;
int sensor   = A5; //used for mux

unsigned char ir         = 8;
unsigned char wall       = 12;
unsigned char seen_robot = 0;
unsigned char heard      = 0;
unsigned char roboStart  = 2; 

char pin_out_s0W = 4;
char pin_out_s1W = 7;
char pin_out_s2W = 3;


int thresh          = 500;
int frontwallThresh = 150;
int sidewallThresh  = 200;

boolean isReady = 0; 

void setup() {
pinMode(rightSen,    INPUT);
pinMode(centSen,     INPUT);
pinMode(leftSen,     INPUT);
pinMode(pin_out_s0W, OUTPUT);
pinMode(pin_out_s1W, OUTPUT);
pinMode(pin_out_s2W, OUTPUT);
pinMode(ir,          OUTPUT);
pinMode(wall,        OUTPUT);
pinMode(roboStart,   INPUT);

Serial.begin(9600);
set_select(0,1,1);

//wait for a start signal, either from the sound or from a button press
while(isReady == 0){
  //check_audio();
  if(digitalRead(roboStart) == LOW) isReady =1; 
}
parallax1.attach(6);
parallax2.attach(5);

//radio stuff
////////////////////
radio.begin();
radio.setRetries(15,15);
radio.setAutoAck(true);
radio.setChannel(0x50);
radio.setPALevel(RF24_PA_MIN);
radio.setDataRate(RF24_250KBPS);

role = role_ping_out;
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

radio.startListening();
radio.printDetails();
//////////////////////

radioWrite(dataArray);
}

void loop() {
  follow_line();
// check_IR();
}

//void check_audio(){
//  byte timeOn = TIMSK0;
//  byte freeOff = ADCSRA; 
//  byte admux = ADMUX; 
//  byte didro = DIDR0;
//  TIMSK0 = 0; // turn off timer0 for lower jitter
//  ADCSRA = 0xe5; // set the adc to free running mode
//  ADMUX = 0x45; // use adc1
//  DIDR0 = 0x01; // turn off the digital input for adc0
//  cli();  // UDRE interrupt slows this way down on arduino1.0
//  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
//    while(!(ADCSRA & 0x10)); // wait for adc to be ready
//    ADCSRA = 0xf5; // restart adc
//    byte m = ADCL; // fetch adc data
//    byte j = ADCH;
//    int k = (j << 8) | m; // form into an int
//    k -= 0x0200; // form into a signed int
//    k <<= 6; // form into a 16b signed int
//    fft_input[i] = k; // put real data into even bins
//    fft_input[i+1] = 0; // set odd bins to 0
//  }
//  fft_window(); // window the data for better frequency response
//  fft_reorder(); // reorder the data before doing the fft
//  fft_run(); // process the data in the fft
//  fft_mag_log(); // take the output of the fft
//  sei();
//  //Serial.println(fft_log_out[5]);
//  if(fft_log_out[5]>150) heard++;
//  else heard=0;
//  if(heard>8) isReady=1;
//  TIMSK0 = timeOn; // turn On timer0 for lower jitter
//  ADCSRA = freeOff; // set the adc free running mode off
//  ADMUX = admux; 
//  DIDR0 = didro; // turn on the digital input for adc0
//}
    
//void check_IR(){
//  parallax1.detach();
//  parallax2.detach();
//  byte timeOn = TIMSK0;
//  byte freeOff = ADCSRA; 
//  byte admux = ADMUX; 
//  byte didro = DIDR0;
//  TIMSK0 = 0; // turn off timer0 for lower jitter
//  ADCSRA = 0xe5; // set the adc to free running mode
//  ADMUX = 0x41; // use adc0
//  DIDR0 = 0x01; // turn off the digital input for adc0
//  cli();  // UDRE interrupt slows this way down on arduino1.0
//  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
//    while(!(ADCSRA & 0x10)); // wait for adc to be ready
//    ADCSRA = 0xf5; // restart adc
//    byte m = ADCL; // fetch adc data
//    byte j = ADCH;
//    int k = (j << 8) | m; // form into an int
//    k -= 0x0200; // form into a signed int
//    k <<= 6; // form into a 16b signed int
//    fft_input[i] = k; // put real data into even bins
//    fft_input[i+1] = 0; // set odd bins to 0
//  }
//  fft_window(); // window the data for better frequency response
//  fft_reorder(); // reorder the data before doing the fft
//  fft_run(); // process the data in the fft
//  fft_mag_log(); // take the output of the fft
//  sei();
//  //Serial.println("IR:");
//  //Serial.println(fft_log_out[42]);
//  if(fft_log_out[42]>100) seen_robot++;
//  else seen_robot=0;
//
//  digitalWrite(ir, LOW);
//  if(seen_robot>2)
//  {
//    digitalWrite(ir, HIGH);
//    robot_stop();
//    while(1);
//  }
//
//  TIMSK0 = timeOn; // turn On timer0 for lower jitter
//  ADCSRA = freeOff; // set the adc free running mode off
//  ADMUX = admux; 
//  DIDR0 = didro; // turn on the digital input for adc0
//  parallax1.attach(6);
//  parallax2.attach(5);
//}

//set mux value: this is for wall sensors as well as audio input  
void set_select(int x, int y, int z)
{
      digitalWrite(pin_out_s0W, z);
      digitalWrite(pin_out_s1W, y);
      digitalWrite(pin_out_s2W, x);
}

//turn left
void turn_left(){
  //adjust dir_facing 
   if (dir_facing == 0) dir_facing = 3;
   else dir_facing --; // Update dir_facing for left turning robot
  //go forward a bit to turn properly
  parallax1.write(95);
  parallax2.write(85);
  delay(600);
  //turn away from the line so we can detect it again
  parallax1.write(85);
  parallax2.write(85);
  delay(800);
  //wait until we are back on the line 
  while(analogRead(centSen)>thresh)
  {
    parallax1.write(85);
    parallax2.write(85);
  }
}

//turn around
void turn_around(){
  //adjust dir_facing

  if (dir_facing == 0) dir_facing = 2;
  else if (dir_facing == 2) dir_facing = 0;
  else if (dir_facing == 1) dir_facing = 3;
  else if (dir_facing == 3) dir_facing = 1;  
  
  parallax1.write(95);
  parallax2.write(85);
  delay(600);
  parallax1.write(0);
  parallax2.write(0);
  delay(800);
  while(analogRead(centSen)>thresh)
  {
    parallax1.write(85);
    parallax2.write(85);
  }
}

//turn right
void turn_right(){
  //adjust dir_facing 
  if (dir_facing == 3) dir_facing = 0;
  else dir_facing ++; // Update dir_facing for right turning robot
  
  parallax1.write(105);
  parallax2.write(75);
  delay(600);
  parallax1.write(95);
  parallax2.write(95);
  delay(800);
  while(analogRead(centSen)>thresh)
  {
    parallax1.write(95);
    parallax2.write(95);
  }
}

//stop
void robot_stop(){
  parallax1.write(90);
  parallax2.write(90);
}

bool frontw()
{
  set_select(0,0,1);
  int val = analogRead(sensor);
  if(val>frontwallThresh)return true;
  else return false;
}
bool leftw()
{
  set_select(0,0,0);
  int val = analogRead(sensor);
  if(val>sidewallThresh) return true;
  else return false;
}

bool rightw()
{
  set_select(0,1,0);
  int val = analogRead(sensor);
  if(val>sidewallThresh)  return true;
  else return false;
}

//follow line
void follow_line(){
  int left   = analogRead(leftSen);
  int right  = analogRead(rightSen);
  int center = analogRead(centSen);
  // go straight
  if(center<thresh && right>thresh && left>thresh){
    //Serial.println("straight");
    parallax1.write(92);
    parallax2.write(88); 
  }
  // correct for veer right
  else if(right<thresh && left>thresh){
    //Serial.println(" veer right");
    parallax1.write(96);
    parallax2.write(89); 
  }
  //correct for veer left
  else if(right>thresh && left<thresh){
    //Serial.println("veer left");
    parallax1.write(91);
    parallax2.write(86); 
  }
 
  //at intersection
  else if(center<thresh && right<thresh && left<thresh){
      if      (dir_facing == 0) dataArray [0] --; // If robot is facing north
      else if (dir_facing == 1) dataArray [1] ++; // If robot is facing east
      else if (dir_facing == 2) dataArray [0] ++; // If robot is facing south
      else if (dir_facing == 3) dataArray [1] --; // If robot is facing west
    // Setting north, east, south, and west to false
    for ( int i = 2; i < 6; i++){
      dataArray[i] = 0;
    }
      int left_space; 
      int right_space; 
      int front_space; 
      
      if      (dir_facing == 0) {
        dataArray [0] --; // If robot is facing north
        left_space = (dataArray[0]-1, dataArray[1]);
        right_space = (dataArray[0]+1, dataArray[1]); 
        front_space = (dataArray[0], dataArray[1]+1);
      }
      else if (dir_facing == 1) {
        dataArray [1] ++; // If robot is facing east
        left_space = (dataArray[0], dataArray[1]+1);
        right_space = (dataArray[0], dataArray[1]-1); 
        front_space = (dataArray[0]+1, dataArray[1]);
      }
      else if (dir_facing == 2) {
        dataArray [0] ++; // If robot is facing south
        left_space = (dataArray[0]-1, dataArray[1]);
        right_space = (dataArray[0]+1, dataArray[1]); 
        front_space = (dataArray[0], dataArray[1]-1);
      }
      else if (dir_facing == 3) {
        dataArray [1] --; // If robot is facing west
        left_space = (dataArray[0], dataArray[1]-1);
        right_space = (dataArray[0], dataArray[1]+1); 
        front_space = (dataArray[0]-1, dataArray[1]);
      }

///////////////////////////////////////////////////////////////////
///////////////// Check walls and add to stack ////////////////////
///////////////////////////////////////////////////////////////////
    //add left  step to stack
    if (!leftw())  {
       if (totalSquares[left_space] != 0) {
        //Serial.println("can go left");
        visitStack.push (left_space);
       }
      }  
    //update wall positions
    else {
        if      (dir_facing == 0) dataArray[5] =1; // If robot faces north, West=true
        else if (dir_facing == 1) dataArray[2] =1; // If robot faces east,  North=true
        else if (dir_facing == 2) dataArray[3] =1; // If robot faces south, East=true
        else if (dir_facing == 3) dataArray[4] =1; // If robot faces west,  South=true
    }
    if (!rightw()) {
      if (totalSquares[right_space] != 0) {
        //.println("can go right");
        visitStack.push (right_space);
      }
      }//add right step to stack

    else {
       if      (dir_facing == 0) dataArray[3] =1;
       else if (dir_facing == 1) dataArray[4] =1; // If robot faces east, South=true
       else if (dir_facing == 2) dataArray[5] =1; // If robot faces south, West=true
       else if (dir_facing == 3) dataArray[2] =1; // If robot faces west, North=true
    }

    //add front step to stack 
    if (!frontw()) {
      if (totalSquares[front_space] != 0) {
        //Serial.println("can go front");
        visitStack.push (front_space);
        }
      }
    else { 
      if      (dir_facing == 0) dataArray[2] =1; // North=true
      else if (dir_facing == 1) dataArray[3] =1; // East=true
      else if (dir_facing == 2) dataArray[4] =1; // South=true
      else if (dir_facing == 3) dataArray[5] =1; // West=true
    }

    //pick the last thing off the stack and go that way 
//////////////////////////////////////////////////////////////////////////////////
    char nextSquare = visitStack.pop();

    
    if(frontw()){ 

      digitalWrite(wall, HIGH);

      if(rightw() && !leftw()) turn_left(); //DO THIS LAST OR DIR IS WRONG
      
      else if (leftw() && !rightw())turn_right();
    
      else if(leftw() && rightw()) turn_around();
      else
      {
        turn_left();
      }
      digitalWrite(wall, LOW);
    } 
    else{  
      parallax1.write(100);
      parallax2.write(80);
    }
    for (size_t i = 0; i < 3; i++) {
      //check_IR();
    }
    parallax1.write(92);
    parallax2.write(88); 
    radioWrite(dataArray);
  }
}
int radioWrite(char dataArray[]){
  radio.stopListening(); // First, stop listening so we can talk.
    int number[6] = {dataArray[0],dataArray[1],dataArray[2], dataArray[3],dataArray[4], dataArray[5]};
  
    bool ok = radio.write( &number, 6 * sizeof(int) );
    // Now, continue listening
    radio.startListening();
    unsigned long started_waiting_at = millis();// Wait here until we get a response, or timeout (250ms)
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;
    // Describe the results
    if ( timeout ){}//Serial.println("Failed, response timed out.\n\r");
    else
    {
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );// Grab the response, compare, and send to debugging spew
    }
}

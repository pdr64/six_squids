#define LOG_OUT 1// use the log output func  tion
#define FFT_N 256// set to 256point fft
#include <FFT.h>
#include <Servo.h>

/////////////////////

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0x000000000CLL, 0x000000000DLL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back" };
role_e role = role_pong_back;
int dataArray[] = {2, 2, 0, 1, 1, 1}; 
int travel      = 0;
byte dir_facing = 0; 
// 0 = north, 1 = east, 2 = south, 3 = west

int prevSquare[] = {0, 0};
int totalSquares[9][3] = {
//x, y, isVisited
{0, 0 ,0},
{0, 1, 0},
{0, 2, 0},
{1, 0, 0},
{1, 1, 0},
{1, 2, 0},
{2, 0, 0},
{2, 1, 0},
{2, 2, 0}
};
///////////////////

Servo parallax1;
Servo parallax2;

int rightSen = A2;
int centSen  = A3;
int leftSen  = A4;
int sensor   = A5; //used for mux

int ir         = 8;
int wall       = 12;
int seen_robot = 0;
int heard      = 0;
int roboStart  = 2; 

int pin_out_s0W = 4;
int pin_out_s1W = 7;
int pin_out_s2W = 3;


int thresh          = 500;
int frontwallThresh = 150;
int sidewallThresh  = 200;

int isReady = 0; 

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
  check_audio();
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

void check_audio(){
  byte timeOn = TIMSK0;
  byte freeOff = ADCSRA; 
  byte admux = ADMUX; 
  byte didro = DIDR0;
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x45; // use adc1
  DIDR0 = 0x01; // turn off the digital input for adc0
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fft_input[i] = k; // put real data into even bins
    fft_input[i+1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  //Serial.println(fft_log_out[5]);
  if(fft_log_out[5]>150) heard++;
  else heard=0;
  if(heard>8) isReady=1;
  TIMSK0 = timeOn; // turn On timer0 for lower jitter
  ADCSRA = freeOff; // set the adc free running mode off
  ADMUX = admux; 
  DIDR0 = didro; // turn on the digital input for adc0
}
    
void check_IR(){
  parallax1.detach();
  parallax2.detach();
  byte timeOn = TIMSK0;
  byte freeOff = ADCSRA; 
  byte admux = ADMUX; 
  byte didro = DIDR0;
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x41; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fft_input[i] = k; // put real data into even bins
    fft_input[i+1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  //Serial.println("IR:");
  //Serial.println(fft_log_out[42]);
  if(fft_log_out[42]>100) seen_robot++;
  else seen_robot=0;

  digitalWrite(ir, LOW);
  if(seen_robot>2)
  {
    digitalWrite(ir, HIGH);
    robot_stop();
    while(1);
  }

  TIMSK0 = timeOn; // turn On timer0 for lower jitter
  ADCSRA = freeOff; // set the adc free running mode off
  ADMUX = admux; 
  DIDR0 = didro; // turn on the digital input for adc0
  parallax1.attach(6);
  parallax2.attach(5);
}

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
  

  //go forward a bit to turn properly
  parallax1.write(95);
  parallax2.write(85);
  delay(600);
  //turn away from the line so we can detect it again
  parallax1.write(85);
  parallax2.write(85);
  delay(300);
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
  
  
  parallax1.write(95);
  parallax2.write(85);
  delay(600);
  parallax1.write(95);
  parallax2.write(95);
  delay(100);
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

    for ( int i = 2; i < 6; i++){
      dataArray[i] = 0;
    }
    if(frontw()){ 
      if (dir_facing == 0) dataArray[0] --;
      else if (dir_facing == 1) dataArray [1] ++; 
      else if (dir_facing == 2) dataArray [0] ++; 
      else if (dir_facing == 3) dataArray [1] --; 
      digitalWrite(wall, HIGH);
      if(dir_facing == 0) dataArray[2] = 1; //north
      else if (dir_facing == 1) dataArray[3] =1; 
      else if (dir_facing == 2) dataArray[4] =1; 
      else if (dir_facing == 3) dataArray[5] =1; 
     
      if(rightw() && !leftw()){
       turn_left();
       if(dir_facing == 0) {
        dataArray[3] = 1; 
       }
       else if (dir_facing == 1) {
        dataArray[4] =1; 
       }
       else if (dir_facing == 2) {
        dataArray[5] =1;
       }
       else if (dir_facing == 3) {
        dataArray[2] =1;
       }
       if (dir_facing == 0) dir_facing = 3;
       else dir_facing --;
      }
      
      else if (leftw() && !rightw())
      {  
        turn_right();
        if(dir_facing == 0) {
          dataArray[5] = 1; 
        }
        else if (dir_facing == 1) {
          dataArray[2] =1; 
        }
        else if (dir_facing == 2) {
          dataArray[3] =1; 
        }
        else if (dir_facing == 3) {
          dataArray[4] =1; 
        }
        if (dir_facing == 3) dir_facing = 0;
        else dir_facing ++; 
        
      }
      else if(leftw() && rightw())
      {
        dataArray[3] = 1; //east = true
        dataArray[5] = 1; //west = true
        turn_around();

        if(dir_facing == 0){
          dataArray[3] = 1;
          dataArray[5] = 1;
        }
        else if (dir_facing == 1) {
          dataArray[4] =1; 
          dataArray[2] =1;
        }
        else if (dir_facing == 2) {
          dataArray[5] =1;
          dataArray[3] =1; 
        }
        else if (dir_facing == 3) {
          dataArray[2] =1;
          dataArray[4] =1;
        }
          if (dir_facing == 0) dir_facing = 2;
          else if (dir_facing == 2) dir_facing = 0;
          else if (dir_facing == 1) dir_facing = 3;
          else if (dir_facing == 3) dir_facing = 1;
      }
      else
      {
        turn_left();
      }
      digitalWrite(wall, LOW);
    } 
    else{ //go straight 
      parallax1.write(100);
      parallax2.write(80);
    }
    for (size_t i = 0; i < 3; i++) {
      check_IR();
    }
    parallax1.write(92);
    parallax2.write(88); 
    Serial.println(dataArray[0]); 
    Serial.println(dataArray[1]);
    Serial.println("write");
    radioWrite(dataArray);
  }
}
int radioWrite(int dataArray[]){
  radio.stopListening(); // First, stop listening so we can talk.
    //Serial.println(dataArray[0]  dataArray[1], dataArray[2], dataArray[3], dataArray[4], dataArray[5]);
    int number[6] = {dataArray[0],dataArray[1],dataArray[2], dataArray[3],dataArray[4], dataArray[5]};
  
    bool ok = radio.write( &number, 6 * sizeof(int) );
    if (ok){}
      //Serial.println("ok...");
    else{}
      //Serial.println("failed.\n\r");
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

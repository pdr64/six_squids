#define LOG_OUT 1// use the log output func  tion
#define FFT_N 256// set to 256point fft
#include <FFT.h>
#include <Servo.h>

/////////////////////
/*
//#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
//#include "prinf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0x000000000CLL, 0x000000000DLL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back" };
role_e role = role_pong_back;
int dataArray[] = {0, 0, 0, 0, 0, 0}; 
int size_array  = 6;
int travel      = 0;
*/
/////////////////////


Servo parallax1;
Servo parallax2;

int rightSen = A2;
int centSen = A3;
int leftSen = A4;

int ir = 8;
int wall = 12;
int seen_robot=0;

int pin_out_s0W = 4;
int pin_out_s1W = 7;
int pin_out_s2W = 3;
int sensor = A1;

int thresh= 500;
int wallThresh = 150;

int isReady = 0; 

void setup() {
parallax1.attach(6);
parallax2.attach(5);

pinMode(rightSen, INPUT);
pinMode(centSen, INPUT);
pinMode(leftSen, INPUT);
pinMode(pin_out_s0W, OUTPUT);
pinMode(pin_out_s1W, OUTPUT);
pinMode(pin_out_s2W, OUTPUT);
pinMode(ir, OUTPUT);
pinMode(wall, OUTPUT);
//pinMode(A0, INPUT);
Serial.begin(9600);
set_select(0,1,1);
while(isReady == 0){
  check_audio();
}
////////////////////
/*
printf_begin();
printf("\n\rRF24/examples/GettingStarted/\n\r");
printf("ROLE: %s\n\r",role_friendly_name[role]);
printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
radio.begin();
radio.setRetries(15,15);
radio.setAutoAck(true);
radio.setChannel(0x50);
radio.setPALevel(RF24_PA_MIN);
radio.setDataRate(RF24_250KBPS);
if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }
radio.startListening();
radio.printDetails();
*/
//////////////////////
}

void loop() {
  follow_line();
}

void check_audio(){
  parallax1.detach();
  parallax2.detach();
  byte timeOn = TIMSK0;
  byte freeOff = ADCSRA; 
  byte admux = ADMUX; 
  byte didro = DIDR0;
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x42; // use adc0
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
  Serial.println(fft_log_out[5]);
  if (fft_log_out[5] > 120) isReady = 1;
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
  ADMUX = 0x45; // use adc0
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
  Serial.println(fft_log_out[42]);
  if(fft_log_out[42]>100)
  {
    seen_robot++;
  }
  else
  {
    seen_robot=0;
  }
  digitalWrite(ir, LOW);
  if(seen_robot>2)
  {
    digitalWrite(ir, HIGH);
    robot_stop();
    while(1);
  }
//  else{
//    follow_line();
//  }
  TIMSK0 = timeOn; // turn On timer0 for lower jitter
  ADCSRA = freeOff; // set the adc free running mode off
  ADMUX = admux; 
  DIDR0 = didro; // turn on the digital input for adc0
  parallax1.attach(6);
  parallax2.attach(5);
}
  
void set_select(int x, int y, int z)
{
      digitalWrite(pin_out_s0W,z);
      digitalWrite(pin_out_s1W,y);
      digitalWrite(pin_out_s2W, x);
}

//turn left
void turn_left(){
  parallax1.write(100);
  parallax2.write(80);
  delay(200);
  parallax1.write(0);
  parallax2.write(0);
  delay(200);
  while(analogRead(centSen)>thresh)
  {
    parallax1.write(0);
    parallax2.write(0);
  }
}

//turn around
void turn_around(){
  parallax1.write(100);
  parallax2.write(80);
  delay(100);
  parallax1.write(0);
  parallax2.write(0);
  delay(1000);
  while(analogRead(centSen)>thresh)
  {
    parallax1.write(0);
    parallax2.write(0);
  }
}

//turn right
void turn_right(){
  parallax1.write(100);
  parallax2.write(80);
  delay(200);
  parallax1.write(180);
  parallax2.write(180);
  delay(200);
  while(analogRead(centSen)>thresh)
  {
    parallax1.write(180);
    parallax2.write(180);
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
  //Serial.println(" front:");
  //Serial.println(val);
  if(val>wallThresh)
  {
    return true;
  }
  else 
  {
    return false;
  }
  
  
}
bool leftw()
{
  set_select(0,0,0);
  int val = analogRead(sensor);
  //Serial.println(" left:");
  //Serial.println(val);
  if(val>wallThresh)
  {
    return true;
  }
  else 
  {
    return false;
  }
}
bool rightw()
{
  set_select(0,1,0);
  int val = analogRead(sensor);
  //Serial.println(" right:");
  //Serial.println(val);
  if(val>wallThresh)
  {
    return true;
  }
  else 
  {
    return false;
  }
}
//follow line
void follow_line(){
  int left = analogRead(leftSen);
  int right =analogRead(rightSen);
  int center =analogRead(centSen);
  // go straight
  if(center<thresh && right>thresh && left>thresh){
    //Serial.println("straight");
    parallax1.write(100);
    parallax2.write(80); 
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
    //int tmp = 0;
    //for ( int i = 0; i < size_array; i++ ) {
      
    //}
    //for ( int i = 2; i < 6; i++){
      //dataArray[i] = 0;
    //}
    if(frontw())
    { 
      //travel++;
      digitalWrite(wall, HIGH);
      //dataArray[2] = 1; //north = true
      if(rightw() && !leftw())
      {
        //dataArray[3] = 1; //east = true
        turn_left();
        Serial.println("turning left");
      }
      else if (leftw() && !rightw())
      {
        //dataArray[5] = 1; //west = true
        turn_right();
        Serial.println("turning right");
      }
      else if(leftw() && rightw())
      {
        //dataArray[3] = 1; //east = true
        //dataArray[5] = 1; //west = true
        turn_around();
        Serial.println("turning around");
      }
      else
      {
        turn_left();
        Serial.println("only front wall");
      }
      //radioWrite(dataArray);
      digitalWrite(wall, LOW);
    } 
    else{
      parallax1.write(100);
      parallax2.write(80);
    }
    for (size_t i = 0; i < 3; i++) {
      check_IR();
    }
  }
}
/*
int radioWrite(int dataArray()){
  radio.stopListening(); // First, stop listening so we can talk.

    printf("Now sending %d, %d, %d, %d, %d, %d...", dataArray[0], dataArray[1], dataArray[2], dataArray[3], dataArray[4], dataArray[5]);

    int number[6] = {dataArray[0],dataArray[1],dataArray[2], dataArray[3],dataArray[4], dataArray[5]};
  
    bool ok = radio.write( &number, 6 * sizeof(int) );

    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    }

    // Try again 1s later
    delay(1000);
}
*/

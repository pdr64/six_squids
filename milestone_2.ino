#define LOG_OUT 1// use the log output function
#define FFT_N 256// set to 256point fft
#include <FFT.h>
#include <Servo.h>


Servo parallax1;
Servo parallax2;

int leftSen = A5;
int centSen = A1;
int rightSen = A2;
int rightWall = A3;
int frontWall = A4;

int thresh= 150;
int wallthresh = 170;

void setup() {
parallax1.attach(6);
parallax2.attach(5);
  
pinMode(leftSen, INPUT);
pinMode(centSen,INPUT);
pinMode(rightSen,INPUT);
pinMode(rightWall, INPUT);
pinMode(frontWall, INPUT);
pinMode(2, OUTPUT);
pinMode(7, OUTPUT);

Serial.begin(9600);
Serial.println("Milestone 2 begin");
}

void loop() {
  follow_line();
  
  check_IR();
}
    
void check_IR(){
  int timeOn = TIMSK0; // turn off timer0 for lower jitter
  int freeOff = ADCSRA; // set the adc to free running mode
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
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
  if(fft_log_out[42]>120)
  {
    digitalWrite(2, HIGH);
    Serial.println("Visual LED on");
    robot_stop();
    delay(2500);
  }
  else
  {
    digitalWrite(2, LOW);
  }
  TIMSK0 = timeOn; // turn On timer0 for lower jitter
  ADCSRA = freeOff; // set the adc free running mode off
}

//turn left
void turn_left(){
  parallax1.write(92);
  parallax2.write(0);
  delay(400);
}

//stop
void robot_stop(){
  parallax1.write(95);
  parallax2.write(96);
}

//follow line
void follow_line(){
  // correct for veer right
  while((analogRead(centSen)>thresh && analogRead(rightSen)>thresh && analogRead(leftSen)<thresh)){
    parallax1.write(92);
    parallax2.write(92);   
  }
  //correct for veer left
  while((analogRead(centSen)>thresh && analogRead(rightSen)<thresh && analogRead(leftSen)>thresh)){
    parallax1.write(98);
    parallax2.write(98);
  }
  // go straight
  while(analogRead(centSen)<thresh && analogRead(leftSen)>thresh && analogRead(rightSen)>thresh){
    parallax1.write(180);
    parallax2.write(60); 
  }
  while(analogRead(centSen)<thresh && analogRead(leftSen)<thresh && analogRead(rightSen)<thresh)
  {
    uint16_t rWall = analogRead(rightWall);
    uint16_t fWall = analogRead(frontWall);
    if((fWall>=wallthresh && rWall>=wallthresh)||(rWall<wallthresh &&fWall>=wallthresh))
    {
      digitalWrite(7, HIGH);
      Serial.println("wall detected");
      turn_left();
      digitalWrite(7, LOW);
    }
    else{
      parallax1.write(180);
      parallax2.write(60);
    }
  }
}


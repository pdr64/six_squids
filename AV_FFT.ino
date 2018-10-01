/*
fft_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

void setup() {
  Serial.begin(115200); // use the serial portM
  //TIMSK0 = 0; // turn off timer0 for lower jitter
 // ADCSRA = 0xe5; // set the adc to free running mode
//  ADMUX = A0; // use adc0
 // DIDR0 = 0x01; // turn off the digital input for adc0
  pinMode(7, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  
  
}

void loop() {
  while(1) { // reduces jitter

    fft_audio ();
    fft_visual();    
  }
}

void fft_audio() {
    //ADMUX = A0;
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      /*while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int */
      fft_input[i] = analogRead(A0); // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    if(fft_log_out[20]>45)
      {
        digitalWrite(7, HIGH);
        Serial.println("Audio LED on");
      }
      else
      {
        digitalWrite(7, LOW);
      }
    Serial.println("start audio");
    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      ///Serial.println(fft_log_out[i]); // send out the data
    }
}

void fft_visual() {
    //ADMUX = A1;
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      /*while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int */
      fft_input[i] = analogRead(A1); // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    if(fft_log_out[76]>20) 
      {
        digitalWrite(4, HIGH);  
        Serial.println("Visual LED on");
      }
    else 
      {
        digitalWrite(4, LOW); 
        Serial.println(fft_log_out[76]); 
      }
    Serial.println("start visual");
    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      //Serial.println(fft_log_out[i]); // send out the data
      
    }
}


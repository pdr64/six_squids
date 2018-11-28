/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000000CLL, 0x000000000DLL };


// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;


void setup(void)
{

  Serial.println("reset");
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("* PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);



  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  role = 1;
}

void loop(void)
{  
  radioRead(); 
  
}
// vim:cin:ai:sts=2 sw=2 ft=cpp

int radioRead(){
      // if there is data ready

    int reset[6] = {5, 5, 5, 5, 5, 5};
      
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      int receive[6];
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &receive, 6 * sizeof(int) );

        // Spew it
        printf("Got payload %d, %d, %d, %d, %d, %d...",receive[0], receive[1], receive[2], receive[3], receive[4], receive[5]);

        String coords = receive[0] + "," + receive[1];
        int tmp = 0;
//        for ( size_t i = 0; i < 6; i++ ) {
//          if ( receive[i] == 5 ){
//            tmp += 1;
//          }
//          if ( tmp == 6 ){
//            coords = "reset";
//            Serial.println(coords);
//          }
//        }

        if ( receive[2] == 1 ){
          coords += ",north=true";
        }
        if ( receive[3] == 1 ){
          coords += ",east==true";
        }
        if ( receive[4] == 1 ){
          coords += ",south==true";
        }
        if ( receive[5] == 1 ){
          coords += ",west==true";
        }

        Serial.println(coords);
               
        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

       }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &receive, 6 * sizeof(int) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
}

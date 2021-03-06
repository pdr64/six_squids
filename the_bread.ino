#include <Servo.h>
#include <StackArray.h>

#define North 0 
#define East  1 
#define South 2 
#define West  3

/////////////////////

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9,10);
const uint64_t pipes[2] = { 0x000000000CLL, 0x000000000DLL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back" };
role_e role = role_pong_back;
/////////////////////////

int dataArray[] = {0, 0, 0, 0, 0, 0}; //TODO: don't harcode walls at the start
byte dir_facing = South; 

int totalSquares[9][9]; //to see if we have visited or not 
StackArray <int> visitStack; //where we're going next 
StackArray <int> history; //keeping track of where we've been so we can backtrack 

///////////////////

Servo parallax1;
Servo parallax2;

int rightSen = A1; //wall sensors
int centSen  = A2;
int leftSen  = A3;
int frontWall = A4; 
int rightWall = A0; 
int leftWall  = A5;

//these are mostly just LEDs
int  roboStart  = 2; 

//thresholds for movement
int thresh          = 300; //ground threshhold (for line following)
int frontwallThresh = 90;
int sidewallThresh  = 250;

void setup() {
  
  pinMode(rightSen,    INPUT);
  pinMode(centSen,     INPUT);
  pinMode(leftSen,     INPUT);
  pinMode(roboStart,   INPUT);
  Serial.begin(9600);
  
  
  //wait for a start signal, either from the sound or from a button press
  int isReady = 0; 
  while(isReady == 0){
    if(digitalRead(roboStart) == LOW){
      isReady =1;
    }
  }
  
  visitStack.push(0);
  visitStack.push(1);
  history.push(0);
  history.push(0);
  totalSquares[0][0] = 1;
  
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
//////////////////////

  dataArray[2] = 1;
  dataArray[5] = 1;

  radioWrite(dataArray);

  parallax1.attach(6);
  parallax2.attach(5);

  dataArray[0] = 0; 
  dataArray[1] = 1; 
  dataArray[2] = 0;
  
}

void loop() {
  follow_line();
}

//turn left
void turn_left(){
  Serial.println("turning left");
   if (dir_facing == 0) dir_facing = 3;
   else dir_facing --; // 
  //go forward a bit to turn properly
  parallax1.write(95);
  parallax2.write(85);
  delay(400);
  //turn away from the line so we can detect it again
  parallax1.write(85);
  parallax2.write(82);
  delay(200);
  //Serial.println("right sens: " + String(analogRead(rightSen)));
  //wait until we are back on the line 
  while(analogRead(rightSen)>thresh)
  {
    //Serial.println("right sens: " + String(analogRead(rightSen)));
    parallax1.write(85);
    parallax2.write(82);
  }

  //Serial.println("right sens: " + String(analogRead(rightSen)));
  
}

//turn around
void turn_around(){
  Serial.println("turning around");
  //adjust dir_facing

  if      (dir_facing == 0) dir_facing = 2;
  else if (dir_facing == 2) dir_facing = 0;
  else if (dir_facing == 1) dir_facing = 3;
  else if (dir_facing == 3) dir_facing = 1;  
  
  parallax1.write(95);
  parallax2.write(85);
  delay(300);
  Serial.println("getting away");
  parallax1.write(0);
  parallax2.write(0);
  delay(680);
  Serial.println("right sens: " +String(analogRead(leftSen)));
  while(analogRead(leftSen)>thresh)
  {
    //Serial.println("right sens: " +String(analogRead(leftSen)));
    parallax1.write(55);
    parallax2.write(55);
  }
  Serial.println("right sens: " + String(analogRead(leftSen)));
  Serial.println("done turning");
  //parallax1.write(87);
  //parallax2.write(84);
  //delay(200);
}

//turn right
void turn_right(){
  Serial.println("turning right");
  //adjust dir_facing 
  if (dir_facing == 3) dir_facing = 0;
  else dir_facing ++; // Update dir_facing for right turning robot
  
  parallax1.write(100);
  parallax2.write(80);
  delay(550);
  parallax1.write(85);
  parallax2.write(82);
  delay(200);
  while(analogRead(leftSen)>thresh)
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
  int val = analogRead(frontWall);
  if(val>frontwallThresh)return true;
  else return false;
}
bool leftw()
{
  int val = analogRead(leftWall);
  if(val>sidewallThresh) return true;
  else return false;
}

bool rightw()
{
  int val = analogRead(rightWall);
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
    parallax1.write(100);
    parallax2.write(80); 
    //Serial.println("going straight");
  }
  // correct for veer right
  else if(right<thresh && left>thresh){
    parallax1.write(100);
    parallax2.write(90); 
    //Serial.println("veering right");
  }
  //correct for veer left
  else if(right>thresh && left<thresh){
    parallax1.write(90);
    parallax2.write(80); 
    //Serial.println("veering left");
  }
 
  //at intersection
  else if(center<thresh && right<thresh && left<thresh){
    totalSquares[dataArray[0]][dataArray[1]] = 1; //this square has now been visited
//    for (int i = 0; i < 9; i++){
//      for (int j = 0; j < 9; j++){
//        Serial.print(totalSquares[j][i]);
//      }
//      Serial.println();
//    }
    
    // Setting north, east, south, and west to false (default)
    

    //finding values of boxes to front, left and right 
      int left_space[2]; 
      int right_space[2]; 
      int front_space[2]; 
      
      if      (dir_facing == North) {
        left_space[0]  = dataArray[0]-1;
        left_space[1]  = dataArray[1];
        right_space[0] = dataArray[0]+1;
        right_space[1] = dataArray[1]; 
        front_space[0] = dataArray[0];
        front_space[1] = dataArray[1]-1;
      }
      else if (dir_facing == East) {
        left_space[0]  = dataArray[0];
        left_space[1]  = dataArray[1]-1;
        right_space[0] = dataArray[0];
        right_space[1] = dataArray[1]+1; 
        front_space[0] = dataArray[0]+1;
        front_space[1] = dataArray[1];
      }
      else if (dir_facing == South) {
        left_space[0]  = dataArray[0]+1;
        left_space[1]  = dataArray[1];
        right_space[0] = dataArray[0]-1;
        right_space[1] = dataArray[1]; 
        front_space[0] = dataArray[0];
        front_space[1] = dataArray[1]+1;
      }
      else if (dir_facing == West) {
        left_space[0]  = dataArray[0];
        left_space[1]  = dataArray[1]+1;
        right_space[0] = dataArray[0];
        right_space[1] = dataArray[1]-1; 
        front_space[0] = dataArray[0]-1;
        front_space[1] = dataArray[1];
        
      }

///////////////////////////////////////////////////////////////////
///////////////// Check walls and add to stack ////////////////////
///////////////////////////////////////////////////////////////////
    //add left  step to stack
    if (!leftw())  {
       if (totalSquares[left_space[0]][left_space[1]] == 0) {
        //Serial.println("can go left");
        visitStack.push (left_space[1]);
        visitStack.push (left_space[0]);
       }
      }  
    //update wall positions
    else {
        Serial.println("left wall");
        if      (dir_facing == North) dataArray[5] =1; // If robot faces north, West=true
        else if (dir_facing == East)  dataArray[4] =1; // If robot faces east,  North=true
        else if (dir_facing == South) dataArray[3] =1; // If robot faces south, East=true
        else if (dir_facing == West)  dataArray[2] =1; // If robot faces west,  South=true
    }
    if (!rightw()) {
      if (totalSquares[right_space[0]] [right_space[1]] == 0) {
        visitStack.push (right_space[1]);
        visitStack.push (right_space[0]);
      }
      }//add right step to stack

    else {
      Serial.println("right wall");
       if      (dir_facing == North) dataArray[3] =1;
       else if (dir_facing == East)  dataArray[2] =1; // If robot faces east, South=true
       else if (dir_facing == South) dataArray[5] =1; // If robot faces south, West=true
       else if (dir_facing == West)  dataArray[4] =1; // If robot faces west, North=true
    }

    //add front step to stack 
    if (!frontw()) {
      if (totalSquares[front_space[0]] [front_space[1]] == 0) {
        visitStack.push (front_space[1]);
        visitStack.push (front_space[0]);

        }
      }
    else { 
      Serial.println("front wall");
      if      (dir_facing == North) dataArray[2] =1; // North=true
      else if (dir_facing == East)  dataArray[3] =1; // East=true
      else if (dir_facing == South) dataArray[4] =1; // South=true
      else if (dir_facing == West)  dataArray[5] =1; // West=true
    }


    //pick the last thing off the stack and go that way 
//////////////////////////////////////////////////////////////////////////////////
    int nextSquare[2] = {visitStack.pop(), visitStack.pop()}; //first choice is to move somewhere new

    int deltaX = dataArray[0] - nextSquare[0];
    int deltaY = dataArray[1] - nextSquare[1];

      
    if (((abs(deltaX) + abs(deltaY)) != 1) || (totalSquares[nextSquare[0]][nextSquare[1]] == 1)) {
      Serial.println("backtracking");
      visitStack.push(nextSquare[1]);
      visitStack.push(nextSquare[0]);
      nextSquare[0] = history.pop(); //pop off history stack 
      nextSquare[1] = history.pop();
      Serial.println("bang");
      
    }
    else {
      Serial.println("moving to next available square");
      history.push (dataArray[1]);
      history.push (dataArray[0]);
      //adding to the history so we can back track easily     
    }
     
     Serial.print("Current Square X: " + String(dataArray[0]));
    Serial.println("Current Square Y: " + String(dataArray[1]));
    Serial.print("Next Square X: " + String(nextSquare[0]));
    Serial.println("Next Square Y: " + String(nextSquare[1]));

      deltaX = dataArray[0] - nextSquare[0];
      deltaY = dataArray[1] - nextSquare[1];
    
    Serial.print("Delta X: " + String(deltaX)); 
    Serial.println("Delta Y: " + String(deltaY));

      if      (dir_facing == North) Serial.println("facing north"); // If robot is facing north
      else if (dir_facing == East)  Serial.println("facing east"); // If robot is facing east
      else if (dir_facing == South) Serial.println("facing south"); // If robot is facing south
      else if (dir_facing == West)  Serial.println("facing west"); // If robot is facing west
    


      int turnedAround = 0;
//      if (digitalRead(7)==HIGH){
//        Serial.println("robot seen");
//        parallax1.write(90);
//        parallax2.write(90);
//        delay(5000);
//        if (digitalRead(7) == HIGH){
//          turn_around();
//          turnedAround = 1;
//        }
//      }
      if(turnedAround ==0){
        if((dir_facing == North && deltaY == 1) || 
               (dir_facing == East  && deltaX == -1) || 
               (dir_facing == South && deltaY == -1) ||
               (dir_facing == West  && deltaX == 1)){
          //go straight
          Serial.println("straight");
          parallax1.write(92);
          parallax2.write(88); 
          delay(300);
        }
        else if ((dir_facing == North && deltaX == 1) || 
                 (dir_facing == East  && deltaY == 1) || 
                 (dir_facing == South && deltaX == -1) ||
                 (dir_facing == West  && deltaY == -1)) {
                  Serial.println("left");
                  turn_left();
                 }
  
        else if ((dir_facing == North && deltaX == -1) || 
                 (dir_facing == East  && deltaY == -1) || 
                 (dir_facing == South && deltaX == 1) ||
                 (dir_facing == West  && deltaY == 1)) {
                  Serial.println("right");
                  turn_right();
                  }
        else {
          Serial.println("around");
          turn_around();
        }
      }

      Serial.println("write");
      radioWrite(dataArray);
    
      if      (dir_facing == North) dataArray [1] --; // If robot is facing north
      else if (dir_facing == East)  dataArray [0] ++; // If robot is facing east
      else if (dir_facing == South) dataArray [1] ++; // If robot is facing south
      else if (dir_facing == West)  dataArray [0] --; // If robot is facing west

      Serial.print("left space:  " + String(left_space[0])  + ", " + String(left_space[1]));
      Serial.print("     right space: " + String(right_space[0]) + ", " + String(right_space[1]));
      Serial.println("   front space: " + String(front_space[0]) + ", " + String(front_space[1]));

    //parallax1.write(92);
    //parallax2.write(88);
    
    
    
    Serial.println();
    for ( int i = 2; i < 6; i++){
      dataArray[i] = 0;
    }
    follow_line();
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

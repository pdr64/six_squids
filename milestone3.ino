#define LOG_OUT 1// use the log output func  tion
#include <Servo.h>
#include <StackArray.h>

#define North 0 
#define East  1 
#define South 2 
#define West  3

/////////////////////

#include <SPI.h>
int dataArray[] = {2, 2, 0, 1, 1, 1}; //TODO: don't harcode walls at the start
byte dir_facing = North; 

int totalSquares[9][9]; //to see if we have visited or not 
StackArray <int> visitStack; //where we're going next 

///////////////////

Servo parallax1;
Servo parallax2;

int rightSen = A2; //wall sensors
int centSen  = A4;
int leftSen  = A3;
int sensor   = A5; //used for mux

//these are mostly just LEDs
char ir         = 8;
int  wall       = 12;
char seen_robot = 0;
char heard      = 0;
int  roboStart  = 2; 

//for mux control
char pin_out_s0W = 4;
char pin_out_s1W = 7;
char pin_out_s2W = 3;

//thresholds for movement
int thresh          = 500; //ground threshhold (for line following)
int frontwallThresh = 250;
int sidewallThresh  = 200;

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
  
  set_select(0,1,1); //default mux values
  
  //wait for a start signal, either from the sound or from a button press
  int isReady = 0; 
  while(isReady == 0){
    if(digitalRead(roboStart) == LOW){
      isReady =1;
    }
  }
  
  parallax1.attach(6);
  parallax2.attach(5);
}

void loop() {
  follow_line();
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
   if (dir_facing == 0) dir_facing = 3;
   else dir_facing --; // 
  //go forward a bit to turn properly
  parallax1.write(105);
  parallax2.write(75);
  delay(400);
  //turn away from the line so we can detect it again
  parallax1.write(85);
  parallax2.write(82);
  delay(800);
  //wait until we are back on the line 
  while(analogRead(rightSen)>thresh)
  {
    parallax1.write(85);
    parallax2.write(85);
  }
}

//turn around
void turn_around(){
  //adjust dir_facing

  if      (dir_facing == 0) dir_facing = 2;
  else if (dir_facing == 2) dir_facing = 0;
  else if (dir_facing == 1) dir_facing = 3;
  else if (dir_facing == 3) dir_facing = 1;  
  
  parallax1.write(105);
  parallax2.write(75);
  delay(300);
  parallax1.write(0);
  parallax2.write(0);
  delay(1200);
  while(analogRead(rightSen)>thresh)
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

    totalSquares[dataArray[0]][dataArray[1]] = 1; //this square has now been visited
//    for (int i = 0; i < 9; i++){
//      for (int j = 0; j < 9; j++){
//        Serial.print(totalSquares[j][i]);
//      }
//      Serial.println();
//    }
    
    // Setting north, east, south, and west to false (default)
    for ( int i = 2; i < 6; i++){
      dataArray[i] = 0;
    }

    //finding values of boxes to front, left and right 
      int left_space[2]; 
      int right_space[2]; 
      int front_space[2]; 
      
      if      (dir_facing == 0) {
        left_space[0]  = dataArray[0]-1;
        left_space[1]  = dataArray[1];
        right_space[0] = dataArray[0]+1;
        right_space[1] = dataArray[1]; 
        front_space[0] = dataArray[0];
        front_space[1] = dataArray[1]-1;
      }
      else if (dir_facing == 1) {
        left_space[0]  = dataArray[0];
        left_space[1]  = dataArray[1]+1;
        right_space[0] = dataArray[0];
        right_space[1] = dataArray[1]-1; 
        front_space[0] = dataArray[0]+1;
        front_space[1] = dataArray[1];
      }
      else if (dir_facing == 2) {
        left_space[0]  = dataArray[0]+1;
        left_space[1]  = dataArray[1];
        right_space[0] = dataArray[0]-1;
        right_space[1] = dataArray[1]; 
        front_space[0] = dataArray[0];
        front_space[1] = dataArray[1]+1;
      }
      else if (dir_facing == 3) {
        left_space[0]  = dataArray[0];
        left_space[1]  = dataArray[1]-1;
        right_space[0] = dataArray[0];
        right_space[1] = dataArray[1]+1; 
        front_space[0] = dataArray[0]-1;
        front_space[1] = dataArray[1];
        
      }

///////////////////////////////////////////////////////////////////
///////////////// Check walls and add to stack ////////////////////
///////////////////////////////////////////////////////////////////
    //add left  step to stack
    if (!leftw())  {
       if (totalSquares[left_space[0], left_space[1]] != 0) {
        //Serial.println("can go left");
        visitStack.push (left_space[1]);
        visitStack.push (left_space[0]);
       }
      }  
    //update wall positions
    else {
        if      (dir_facing == North) dataArray[5] =1; // If robot faces north, West=true
        else if (dir_facing == East)  dataArray[2] =1; // If robot faces east,  North=true
        else if (dir_facing == South) dataArray[3] =1; // If robot faces south, East=true
        else if (dir_facing == West)  dataArray[4] =1; // If robot faces west,  South=true
    }
    if (!rightw()) {
      if (totalSquares[right_space[0], right_space[1]] != 0) {
        visitStack.push (right_space[1]);
        visitStack.push (right_space[0]);
      }
      }//add right step to stack

    else {
       if      (dir_facing == North) dataArray[3] =1;
       else if (dir_facing == East)  dataArray[4] =1; // If robot faces east, South=true
       else if (dir_facing == South) dataArray[5] =1; // If robot faces south, West=true
       else if (dir_facing == West)  dataArray[2] =1; // If robot faces west, North=true
    }

    //add front step to stack 
    if (!frontw()) {
      if (totalSquares[front_space[0], front_space[1]] != 0) {
        //Serial.println("can go front");
        visitStack.push (front_space[1]);
        visitStack.push (front_space[0]);
        
//        Serial.println("dirFacing = " + String(dir_facing));
//        Serial.println("front space x: " + String(front_space[0]));
//        Serial.println("front space y: " + String(front_space[1]));
        }
      }
    else { 
      if      (dir_facing == North) dataArray[2] =1; // North=true
      else if (dir_facing == East)  dataArray[3] =1; // East=true
      else if (dir_facing == South) dataArray[4] =1; // South=true
      else if (dir_facing == West)  dataArray[5] =1; // West=true
    }

    //pick the last thing off the stack and go that way 
//////////////////////////////////////////////////////////////////////////////////
    int nextSquare[2] = {visitStack.pop(), visitStack.pop()};

//    Serial.println("Next Square X: " + String(nextSquare[0]));
//    Serial.println("Next Square Y: " + String(nextSquare[1]));

    
    int deltaX = dataArray[0] - nextSquare[0];
    int deltaY = dataArray[1] - nextSquare[1];
    
//    Serial.println("Delta X: " + String(deltaX)); 
//    Serial.println("Delta Y: " + String(deltaY));
    
    //simplest case: we can move to one right next to us 
    if (abs(deltaX) + abs(deltaY) == 1) {
          if((dir_facing == North && deltaY == 1) || 
             (dir_facing == East  && deltaX == 1) || 
             (dir_facing == South && deltaY == -1) ||
             (dir_facing == West  && deltaX == -1)){
        parallax1.write(92);
        parallax2.write(88); 
      }
      else if ((dir_facing == North && deltaX == 1) || 
               (dir_facing == East  && deltaY == -1) || 
               (dir_facing == South && deltaX == -1) ||
               (dir_facing == West  && deltaY == 1)) {
                turn_left();
               }

      else if ((dir_facing == North && deltaX == -1) || 
               (dir_facing == East  && deltaY == 1) || 
               (dir_facing == South && deltaX == 1) ||
               (dir_facing == West  && deltaY == -1)) {
                turn_right();
               }
      else turn_around();
    }
    else turn_around();
    
      if      (dir_facing == North) dataArray [1] --; // If robot is facing north
      else if (dir_facing == East)  dataArray [0] ++; // If robot is facing east
      else if (dir_facing == South) dataArray [1] ++; // If robot is facing south
      else if (dir_facing == West)  dataArray [0] --; // If robot is facing west
//      Serial.println("left space:  " + String(left_space[0])  + ", " + String(left_space[1]));
//      Serial.println("right space: " + String(right_space[0]) + ", " + String(right_space[1]));
//      Serial.println("front space: " + String(front_space[0]) + ", " + String(front_space[1]));

    parallax1.write(92);
    parallax2.write(88); 
    delay(600);
  }
}

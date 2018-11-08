#include <Wire.h>
#define OV7670_I2C_ADDRESS 0x21 //0x42 write 0x43 read
#define OV7670_I2C_W_ADDRESS 0x42
#define OV7670_I2C_R_ADDRESS 0x43 

int fpga_a = 7; 
int fpga_b = 8; 
int fpga_c = 9; 

///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);

  Serial.println("starting i2c"); 
  set_registers();
  delay(1000);
  
  read_key_registers();
  set_color_matrix();

  pinMode(fpga_a, INPUT); 
  pinMode(fpga_b, INPUT);
  pinMode(fpga_c, INPUT);
}

void loop(){
  checkTreasure();
 }

///////// Function Definition //////////////
void read_key_registers(){
  Serial.println("printing registers");
  Serial.println(read_register_value(0x12));
  Serial.println(read_register_value(0x0C));
  Serial.println(read_register_value(0x14));
  Serial.println(read_register_value(0x11));
  Serial.println(read_register_value(0x40));
  Serial.println(read_register_value(0x42));
  Serial.println(read_register_value(0x1E));
  Serial.println("done reading");
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1){
    Serial.println("in loop");
  }
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }


    //Q1. 594000 (bits) 
    //Q2. rgb 565/555
    //Q3. rgb 3:3:2 r<<2, g<<3, b<<3 
    //Q4. 176x144
void set_registers(){
    Serial.println("Writing registers");
    Serial.println (OV7670_write_register(0x12, 0x80)); //COM7: Reset registers, enable color bar, resolution and pixel format 
    delay(100);
    Serial.println(OV7670_write_register(0x12, 0x0E)); //COM7: Reset registers, enable color bar, resolution and pixel format 
    Serial.println(OV7670_write_register(0x0C, 0x08)); //COM3: Enable scaling
    Serial.println(OV7670_write_register(0x14, 0x0B)); //COM9: To make the image less noisy
    Serial.println(OV7670_write_register(0x11, 0xC0)); //CLKRC: Use external clock directly 
    Serial.println(OV7670_write_register(0x40, 0xD0)); //COM15: pixel format
    Serial.println(OV7670_write_register(0x42, 0x08)); //COM17: DSP color bar enable
    Serial.println(OV7670_write_register(0x1E, 0x30)); //MVFP: Vertically flip image enable
    
}

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}


void checkTreasure(){

  int a = digitalRead(fpga_a);
  int b = digitalRead(fpga_b);
  int c = digitalRead(fpga_c);

  if      ((a==0)&(b==0)&(c==0)) Serial.println("none detected"); 
  
  else if ((a==0)&(b==0)&(c==1)) Serial.println("red triangle");
  else if ((a==0)&(b==1)&(c==0)) Serial.println("red square");
  else if ((a==0)&(b==1)&(c==1)) Serial.println("red diamond");
  
  else if ((a==1)&(b==0)&(c==0)) Serial.println("blue triangle"); 
  else if ((a==1)&(b==0)&(c==1)) Serial.println("blue square");
  else if ((a==1)&(b==1)&(c==0)) Serial.println("blue diamond");

  else Serial.println("error: not a known state"); 
}


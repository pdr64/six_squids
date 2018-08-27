#include <Servo.h>;

Servo parallax1;
Servo parallax2;

void setup() {
  parallax1.attach(6);
  parallax2.attach(5);
  }

void loop() {
 parallax1.write(35);
 parallax2.write(94);
 delay(4000);
 parallax1.write(95);
 parallax2.write(165);
 delay(4000);
 
}

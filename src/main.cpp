#include <Arduino.h>
# define LED 2

// // put function declarations here:
// int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);

  Serial.begin(115200);  // Initialize serial communication
  pinMode(LED, OUTPUT);
  Serial.println("Starting LED Blink...");
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(LED, HIGH);
  Serial.println("LED ON");
  delay(2000);
  digitalWrite(LED, LOW);
  Serial.println("LED OFF");
  delay(2000);
}

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }
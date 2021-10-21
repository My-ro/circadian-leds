/* Analog Read to LED
 * ------------------ 
 *
 * turns on and off a light emitting diode(LED) connected to digital  
 * pin 13. The amount of time the LED will be on and off depends on
 * the value obtained by analogRead(). In the easiest case we connect
 * a potentiometer to analog pin 2.
 *
 * Created 1 December 2005
 * copyleft 2005 DojoDave <http://www.0j0.org>
 * http://arduino.berlios.de
 *
 */

int potPin = A5;    // select the input pin for the potentiometer
int ledPin = 13;   // select the pin for the LED
int val = 0;       // variable to store the value coming from the sensor
int bright = 0;

void setup() {
  pinMode(ledPin, OUTPUT);  // declare the ledPin as an OUTPUT
  Serial.begin(9600);
}

void loop() {
  val = analogRead(potPin);    // read the value from the sensor
  bright = ((float)val/1023*255);
  digitalWrite(ledPin, HIGH);  // turn the ledPin on
  delayMicroseconds((float)bright/255*10000);                  // stop the program for some time
  digitalWrite(ledPin, LOW);   // turn the ledPin off
  delayMicroseconds((1-(float)bright)/255*10000);                  // stop the program for some time
  Serial.print(bright);
  Serial.println(val);
}

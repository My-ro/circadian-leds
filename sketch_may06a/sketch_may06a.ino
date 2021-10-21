#define RED_LED 6
#define BLUE_LED 5
#define GREEN_LED 9
int PIN_LED = 13;

int brightness = 255;

int gBright = 0;
int rBright = 0;
int bBright = 0;

int fadeSpeed = 10;

void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  TurnOn();
  delay(5000);
  TurnOff();
  delay(5000);
}

void analogDigital(int PIN_LED, int brightness) {
  digitalWrite(PIN_LED, HIGH);  // turn the ledPin on
  delayMicroseconds((float)brightness/255*10000);                  // stop the program for some time
  digitalWrite(PIN_LED, LOW);   // turn the ledPin off
  delayMicroseconds(((1-(float)brightness)/255*10000)*0.5);                  // stop the program for some time
}

void TurnOn() { 
   for (int i = 0; i < 256; i++) {
       analogWrite(RED_LED, rBright);
       analogDigital(PIN_LED, rBright);
       rBright +=1;
       delay(fadeSpeed);
   }
   digitalWrite(PIN_LED, HIGH);
 
   for (int i = 0; i < 256; i++) {
       analogWrite(BLUE_LED, bBright);
       bBright += 1;
       delay(fadeSpeed);
   } 

   for (int i = 0; i < 256; i++) {
       analogWrite(GREEN_LED, gBright);
       gBright +=1;
       delay(fadeSpeed);
   }
   brightness = 255;
   digitalWrite(PIN_LED, HIGH);
}

void TurnOff() {
   for (int i = 0; i < 256; i++) {
       analogWrite(GREEN_LED, brightness);
       analogWrite(RED_LED, brightness);
       analogWrite(BLUE_LED, brightness);
       analogDigital(PIN_LED, brightness);
 
       brightness -= 1;
       delay(fadeSpeed);
   }
   rBright = 0;
   gBright = 0;
   bBright = 0;
   digitalWrite(PIN_LED, LOW);
}

void loop() {
  TurnOn();
  delay(30000);
  TurnOff();
  delay(30000);
}

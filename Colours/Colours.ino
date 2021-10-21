#define RED 6
#define GREEN 9
#define BLUE 5
#define TRIGGER A5
#define LED 13

/*
int daytime[3] = {255, 239, 142}; // 6500 K
int sunset[3] = {255, 100, 15}; // 3400 K
int nighttime[3] = {255, 53, 0}; // 1900 K
*/

int daytime[3] = {255, 194, 122}; // 6500 K
int sunset[3] = {245, 90, 15}; // 3400 K
int nighttime[3] = {255, 53, 0}; // 1900 K
int currentClr[3] = {255, 133, 0};

int bright = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  bright = (float)analogRead(TRIGGER);  // read the value from the potentiometer
  
  if (bright < 341) {
    currentClr[0] = nighttime[0];
    currentClr[1] = nighttime[1];
    currentClr[2] = nighttime[2];
  }
  else if (bright > 682) {
    currentClr[0] = daytime[0];
    currentClr[1] = daytime[1];
    currentClr[2] = daytime[2];
  }
  else {
    currentClr[0] = sunset[0];
    currentClr[1] = sunset[1];
    currentClr[2] = sunset[2];
  }
  
  Serial.println(bright);
  analogWrite(RED, currentClr[0]); // set all colours
  analogWrite(GREEN, currentClr[1]);
  analogWrite(BLUE, currentClr[2]);
}

#define RED 6
#define GREEN 9
#define BLUE 5
#define TRIGGER A5
#define LED 13

/*
  int daytime[3] = {255, 249, 252}; // 6500 K
  int sunset[3] = {255, 195, 131}; // 3400 K
  int nighttime[3] = {255, 133, 0}; // 1900 K

  int daytime[3] = {255, 239, 142}; // custom 1
  int sunset[3] = {255, 100, 15};
  int nighttime[3] = {255, 53, 0};
*/

int daytime[3] = {255, 194, 122}; // custom 2
int sunset[3] = {245, 60, 5};
int nighttime[3] = {255, 53, 0};

long wakeUpTime[2] = {06, 30}; // HHMM

long timenow[3] = {21, 29, 10}; // HHMMSS, current time
int date[3] = {18, 05, 2020}; // DDMMYYYY, uses month approximation to 30 days

float timeSpeed = 1; // debugging tool

int day = date[0] + 30 * date[1];
int year = date[2];
int stepDay = year % 4;
long offsetMillis = 1000 * timenow[2] + 60000 * timenow[1] + 3600000 * timenow[0];
bool offsetApplied = false;
unsigned long currentMillis = 0; // stores current time value
unsigned long long pastMillis = 0; // the value of the previous days millis(), so we can reset each day
float currentMinutes = 0;

unsigned long wakeTime = 60000 * wakeUpTime[1] + 3600000 * wakeUpTime[0];
unsigned long dawnTime = 0;
unsigned long duskTime = 0;
unsigned long nightTime = 0;
int currentTime = 0;
bool dawn = false;

int val;
float bright = 1;
bool brightLock = false;
bool brightUnlock = false;
float currentClr[3] = {0, 0, 0};
int reporter = 0;

int daylightSavingOn = 88; // daylight saving day numbers
int daylightSavingOff = 298;

void colourSwitch (int nextClr[3], float period, int fadeDelay = 10) {
  if (period > 3600) period = 3600;
  period /= timeSpeed;
  int divi = period * 100;
  float increment[3] = {
    ((float)nextClr[0] - currentClr[0]) / divi,
    ((float)nextClr[1] - currentClr[1]) / divi,
    ((float)nextClr[2] - currentClr[2]) / divi
  };
  float iBr = 0;
  if (brightLock) iBr = (1.00 - bright) / (float)divi;

  for (int i = 0; i < divi; i++) {
    currentClr[0] += increment[0];
    currentClr[1] += increment[1];
    currentClr[2] += increment[2];
    updateClr();
    if (brightLock) bright += iBr;
    if (i % (int)period == 0) {
      /*Serial.print(", ");
        Serial.print(i);
        Serial.print(", ");
        Serial.print(divi);
        Serial.print(increment[0]);
        Serial.print(", ");
        Serial.print(increment[1]);
        Serial.print(", ");
        Serial.print(increment[2]);
        Serial.print(", ");*/
      Serial.print(brightLock);
      Serial.print(", ");
      Serial.print(brightUnlock);
      Serial.print(", ");
      Serial.print(bright);
      Serial.print(", ");
      Serial.print(currentClr[0]);
      Serial.print(", ");
      Serial.print(currentClr[1]);
      Serial.print(", ");
      Serial.print(currentClr[2]);
      Serial.print(", ");
      Serial.print(int(float(i) / divi * 100));
      Serial.println("%");
    }
    delay(fadeDelay);
  }
}

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(TRIGGER, INPUT); //potentiometer
  pinMode(LED, OUTPUT);

  Serial.begin(9600);

  analogWrite(RED, 0);
  analogWrite(GREEN, 0);
  analogWrite(BLUE, 0);

  datekeeping();
  if ((day > daylightSavingOn) && (day < daylightSavingOff)) {
    if (wakeTime > 3600000) wakeTime -= 3600000;
    else wakeTime += 82800000;
    if (offsetMillis > 3600000) offsetMillis -= 3600000;
    else offsetMillis += 82800000;
  }
  daylightHours();
  if (offsetMillis < wakeTime) {
    currentTime = 0;
    colourSwitch(nighttime, 1 * timeSpeed, 10);
  }
  else if (offsetMillis < dawnTime) {
    currentTime = 1;
    colourSwitch(sunset, 1 * timeSpeed, 10);
  }
  else if (offsetMillis < duskTime) {
    currentTime = 2;
    colourSwitch(daytime, 1 * timeSpeed, 10);
  }
  else {
    currentTime = 3;
    colourSwitch(sunset, 1 * timeSpeed, 10);
  }
}

void loop() {
  timekeeping();
  updateClr();
  
  if (reporter == 0) {
    /*Serial.print(wakeTime);
      Serial.print(", ");
      Serial.print(dawnTime);
      Serial.print(", ");
      Serial.print(duskTime);
      Serial.print(", ");
      Serial.print(nightTime);
      Serial.print(", ");*/
    Serial.print(brightLock);
    Serial.print(", ");
    Serial.print(brightUnlock);
    Serial.print(", ");
    Serial.print(bright);
    Serial.print(", ");
    printTime(wakeTime);
    printTime(dawnTime);
    printTime(duskTime);
    printTime(nightTime);
    Serial.print(currentTime + 1);
    Serial.print(", ");
  }

  switch (currentTime) {
    case 0:
      if ((currentMillis > wakeTime) && (currentMillis < duskTime)) {
        if (dawn) {
          brightLock = true;
          colourSwitch(sunset, (dawnTime - wakeTime) / 1000);
        }
        currentTime++;
      }
      break;
    case 1:
      if ((currentMillis > dawnTime) && (currentMillis < duskTime)) {
        if (!dawn) brightLock = true;
        colourSwitch(daytime, (duskTime - dawnTime) / 1000);
        currentTime++;
      }
      break;
    case 2:
      if ((currentMillis > duskTime) && (currentMillis < nightTime)) {
        colourSwitch(sunset, (nightTime - duskTime) / 1000);
        currentTime++;
      }
      break;
    case 3:
      if (((currentMillis > nightTime) && (currentMillis < 86400000)) || ((currentMillis > 0) && (currentMillis < wakeTime))) {
        colourSwitch(nighttime, (wakeTime - nightTime) / 1000);
        currentTime = 0;
      }
      break;
  }
  
  digitalWrite(LED, HIGH);// turn the ledPin on
  delayMicroseconds(bright * 10000); // stop the program for some time
  digitalWrite(LED, LOW); // turn the ledPin off
  delayMicroseconds(((1 - bright) * 10000) * 0.5); // stop the program for some time
}

void timekeeping() {  //steps to next day
  if (currentMillis >= 86400000) {
    pastMillis += 86400000;
    if (!offsetApplied) {
      offsetApplied = true;
      pastMillis -= offsetMillis;
    }
    day += 1;
    datekeeping();
    daylightHours();
    if (day == daylightSavingOn) {
      wakeTime -= 3600000;
    }
    if (day == daylightSavingOff) {
      wakeTime += 3600000;
    }
  }
  currentMillis = millis() * timeSpeed - pastMillis; // this keeps our currentMillis the same each day
  //currentMillis *= timeSpeed;
  if (!offsetApplied) {
    currentMillis += offsetMillis;
  }
  currentMinutes = (float)currentMillis / 60000;
  reporter += timeSpeed;
  if (reporter > 99) reporter = 0;
  if (reporter == 0) {
    /*Serial.print(", ");
      Serial.print((long)pastMillis);
      Serial.print(currentMillis);
      Serial.print(", ");
      Serial.print((int)(currentMinutes / 60));
      Serial.print(":");
      if (((int)currentMinutes % 60) < 10) Serial.print("0");
      Serial.print((int)currentMinutes % 60);*/
    printTime(currentMillis);
    Serial.println();
  }
}

void datekeeping() {  //steps to next year
  if (((day >= 365) && (stepDay != 0)) || (day >= 366)) {
    day = 0;
    year += 1;
    stepDay += 1;
    if (stepDay == 4) {
      stepDay = 0;
    }
    daylightSavingOn = 88;
    daylightSavingOff = 298;
    if (stepDay == 0) {
      daylightSavingOn += 1;
      daylightSavingOff += 1;
    }
  }
}

void daylightHours() { // sunrise and sunset times based on 2020 Southampton sun graph
  if (wakeTime > 3600000) wakeTime -= 3600000;
  else wakeTime += 82800000;
  dawnTime = 3600000 * (6 + 2.15 * cos(((float)day + 7) * 2 * PI / 365.25));
  if (wakeTime > dawnTime) {
    dawn = false;
    dawnTime = wakeTime;
  } else {
    dawn = true;
  }
  duskTime = 3600000 * (18.2 - 1 - 2.2 * cos(((float)day + 13) * 2 * PI / 365.25));
  nightTime = 86400000 + wakeTime - 30600000;
  if (nightTime > 86400000) nightTime -= 86400000;
}

void updateClr() {
  val = analogRead(TRIGGER);  // read the value from the potentiometer
  //if (brightLock && val >= 1023) brightLock = false;
  if (brightLock && ((1021 < int(bright * 1023)) || (val < int(bright * 1023) + 2))) brightUnlock = true;
  if (brightUnlock && val >= int(bright * 1023)) {
    brightLock = false;
    brightUnlock = false;
  }
  if (!brightLock) bright = (float)val / 1023;
  analogWrite(RED, currentClr[0]*bright); // set all colours
  analogWrite(GREEN, currentClr[1]*bright);
  analogWrite(BLUE, currentClr[2]*bright);
}

void printTime (long wakeTime) {
  Serial.print((int)(wakeTime / 3600000));
  Serial.print(":");
  if (((wakeTime / 60000) % 60) < 10) Serial.print("0");
  Serial.print((wakeTime / 60000) % 60);
  Serial.print(", ");
}

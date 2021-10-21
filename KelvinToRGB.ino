//*************************PIN_INPUTS*************************//
#define RED 6
#define GREEN 9
#define BLUE 5
#define TRIGGER A5
#define LED 13

//*************************COLOUR_SET*************************//
/*
  int daytime[3] = {255, 239, 142}; // custom 1
  int sunset[3] = {255, 100, 15};
  int nighttime[3] = {255, 53, 0};

  int daytime[3] = {255, 194, 122}; // custom 2
  int sunset[3] = {245, 90, 15};
  int nighttime[3] = {255, 53, 0};
*/

int daytime[3] = {255, 194, 122}; // custom 2.1
int sunset[3] = {245, 60, 5};
int nighttime[3] = {255, 53, 0};

//*************************DESIRED_WAKING_TIMES*************************//
long wakeUpTime[2] = {6, 30}; // HHMM

long timenow[3] = {23, 19, 0}; // HHMMSS, current time
int dayOfTheWeek = 3; // 1 = Monday, 7 = Sunday
int date[3] = {13, 10, 2021}; // DDMMYYYY, uses month approximation to 30 days

float timeSpeed = 1; // debugging tool

//*************************TIME_COUNTING_SETUP*************************//
int day = date[0] + 30 * date[1];
int year = date[2];
int stepDay = year % 4;
long offsetMillis = 1000 * timenow[2] + 60000 * timenow[1] + 3600000 * timenow[0];  // compound multiplication doesn't work
bool offsetApplied = false; // first day needs offset cause millis() are too low and need an offset since last midnight
unsigned long currentMillis = 0; // stores current time value
unsigned long long pastMillis = 0; // the value of the previous days millis(), so we can reset each day
float currentMinutes = 0;

//*************************TRANSITION_TIMES*************************//
unsigned long wakeTime = 60000 * wakeUpTime[1] + 3600000 * wakeUpTime[0];
unsigned long dawnTime;
unsigned long duskTime;
unsigned long nightTime;
//unsigned long triggerTime;
int currentTime;
bool dawn = false;
bool dusk = false;

//*************************SMOOTHING_&_WAKING_UP*************************//
unsigned int val;
const int sample = 100; // smoothing
int pot[sample];
unsigned long total = 0;
int j;  // counting integer for circular buffer running total smoothing
float bright = 1;
bool brightLock = false;  // ignores potentiometer position and gradually increases brightness and colour before waking up
bool brightUnlock = false;  // allows unlocking potentiometer position by ramping up its value, but makes sure it doesn't unlock immediately after locking
bool weekend; // don't ignore off position during weekend
float currentClr[3] = {0, 0, 0};  // increments not accurate enough, set colour manually after gradient
int reporter = 0;

int daylightSavingOn = 88; // daylight savings day numbers
int daylightSavingOff = 298;

//*************************SWITCH_TO_NEXT_COLOUR*************************//
void colourSwitch (int nextClr[3], float period, int reports = 100, int fadeDelay = 33) {
  // fadeDelay was 10, too fast
  // then 100, too slow, took 1.8 hrs
  // now 55, was messing with morning time instead of wake time
  // so maybe 33
  Serial.println("Adjusting colour...");
  //unsigned long startMillis = currentMillis;
  if (period > 3600) {  // duration max is an hour
    period = 3600;
  }
  period /= timeSpeed;
  unsigned int divi = period * 100;
  float increment[3] = {
    ((float)nextClr[0] - currentClr[0]) / divi,
    ((float)nextClr[1] - currentClr[1]) / divi,
    ((float)nextClr[2] - currentClr[2]) / divi
  };
  float iBr = 0;
  if (brightLock) {
    iBr = (1.00 - bright) / (float)divi;
  }
  if (reports < 2) {
    reports = 2;
  }
  reports--;  // number of progress printoutouts

  for (int i = 0; i < divi; i++) {
    currentClr[0] += increment[0];
    currentClr[1] += increment[1];
    currentClr[2] += increment[2];
    updateClr();
    if (brightLock) {
      bright += iBr;
    }
    if (i % int(divi / reports) == 0) {
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
  currentClr[0] = nextClr[0];
  currentClr[1] = nextClr[1];
  currentClr[2] = nextClr[2];
  Serial.print(bright);
  Serial.print(", ");
  Serial.print(currentClr[0]);
  Serial.print(", ");
  Serial.print(currentClr[1]);
  Serial.print(", ");
  Serial.print(currentClr[2]);
  Serial.print(", ");
  Serial.println("100%");
}

//*************************SETUP*************************//
void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(TRIGGER, INPUT); //potentiometer
  pinMode(LED, OUTPUT);

  Serial.begin(9600);
  Serial.println();

  analogWrite(RED, 0);
  analogWrite(GREEN, 0);
  analogWrite(BLUE, 0);

  for (j = 0; j < sample; j++) {
    pot[j] = 0; // start off, fade in
  }

  datekeeping();

  if (stepDay == 0) {
    daylightSavingOn += 1;
    daylightSavingOff += 1;
    if (date[1] > 1) {
      day++;
    }
  }
  if ((day > daylightSavingOn) && (day < daylightSavingOff)) {
    if (wakeTime > 3600000) {
      wakeTime -= 3600000;
    }
    else {
      wakeTime += 82800000;
    }
    if (offsetMillis >= 3600000) {
      offsetMillis -= 3600000;
    }
    else {
      offsetMillis += 82800000;
    }
  }
  Serial.println("Daylight savings set.");

  daylightHours();
  if (dayOfTheWeek > 7) {
    dayOfTheWeek = 1;
    weekend = false;
  }
  if (dayOfTheWeek > 5) {
    weekend = true;
  }

  // set starting colour to the proper one
  if ((offsetMillis > nightTime) || (offsetMillis < wakeTime)) {
    currentTime = 0;
    colourSwitch(nighttime, 1 * timeSpeed, 6, 10);
    //correctTrigger(wakeTime);
  }
  else if (offsetMillis < dawnTime) {
    currentTime = 1;
    colourSwitch(sunset, 1 * timeSpeed, 6, 10);
    //correctTrigger(dawnTime);
  }
  else if (offsetMillis < duskTime) {
    currentTime = 2;
    colourSwitch(daytime, 1 * timeSpeed, 6, 10);
    //correctTrigger(duskTime);
  }
  else {
    currentTime = 3;
    colourSwitch(sunset, 1 * timeSpeed, 6, 10);
    //correctTrigger(nightTime);
  }

  Serial.println("Setup complete.");
}

//*************************PRINTER_FUNCTION*************************//
void printTime(unsigned long wakeTime, bool comma = true) {
  Serial.print((int)(wakeTime / 3600000));
  Serial.print(":");
  if (((wakeTime / 60000) % 60) < 10) {
    Serial.print("0");
  }
  Serial.print((wakeTime / 60000) % 60);
  if (comma) {
    Serial.print(", ");
  }
}

//*************************LOOP*************************//
void loop() {
  timekeeping();
  updateClr();

  if (reporter == 0) {  // print only once per 100 cycles
    /*Serial.print(wakeTime);
      Serial.print(", ");
      Serial.print(dawnTime);
      Serial.print(", ");
      Serial.print(duskTime);
      Serial.print(", ");
      Serial.print(nightTime);
      Serial.print(", ");*/
    Serial.print(bright);
    Serial.print(", ");
    /*Serial.print(dawn);
      Serial.print(", ");
      Serial.print(dusk);
      Serial.print(", ");*/
    printTime(wakeTime);
    printTime(dawnTime);
    printTime(duskTime);
    printTime(nightTime);
    Serial.print(currentTime + 1);
    Serial.print(", ");
    //printTime(triggerTime);
    printTime(currentMillis, false);
    Serial.println();
  }

  switch (currentTime) {
    case 0:
      if (((currentMillis + 3600000 > wakeTime) || !dawn) && (currentMillis < dawnTime)) {
        if (dawn) {
          if (!weekend) {
            brightLock = true;
            Serial.println("Brightness locked.");
          }
          colourSwitch(sunset, (wakeTime - currentMillis) / 1000);
          //correctTrigger(dawnTime);
        }
        currentTime++;
      }
      break;
    case 1:
      if ((currentMillis + 3600000 > dawnTime) && (currentMillis < duskTime)) {
        if (!dawn) {
          if (!weekend) {
            brightLock = true;
            Serial.println("Brightness locked.");
          }
        }
        colourSwitch(daytime, (dawnTime - currentMillis) / 1000);
        //correctTrigger(duskTime);
        currentTime++;
      }
      break;
    case 2:
      if (((currentMillis + 3600000 > duskTime) && (currentMillis < nightTime)) || !dusk) {
        if (dusk) {
          colourSwitch(sunset, (duskTime - currentMillis) / 1000);
          //correctTrigger(nightTime);
        }
        currentTime++;
      }
      break;
    case 3:
      if ((currentMillis + 3600000 > nightTime) && ((nightTime > wakeTime) || (currentMillis < wakeTime))) {
        colourSwitch(nighttime, (nightTime - currentMillis) / 1000);
        //correctTrigger(wakeTime);
        currentTime = 0;
      }
      break;
  }

  digitalWrite(LED, HIGH);// turn the ledPin on
  delayMicroseconds(bright * 10000); // stop the program for some time
  digitalWrite(LED, LOW); // turn the ledPin off
  delayMicroseconds(((1 - bright) * 10000) * 0.5); // stop the program for some time
}

//*************************DAY_COUNTER*************************//
void timekeeping() {  //steps to next day
  if (currentMillis >= 86400000) {
    pastMillis += 86400000;
    if (!offsetApplied) {
      offsetApplied = true;
      pastMillis -= offsetMillis;
    }
    day++;
    dayOfTheWeek++;
    if (dayOfTheWeek > 7) {
      dayOfTheWeek = 1;
      weekend = false;
    }
    if (dayOfTheWeek > 5) weekend = true;
    datekeeping();
    daylightHours();
    if (day == daylightSavingOn) {
      wakeTime += 3600000;
    }
    if (day == daylightSavingOff) {
      wakeTime -= 3600000;
    }
  }
  currentMillis = millis() * timeSpeed - pastMillis; // this keeps our currentMillis the same each day
  //currentMillis *= timeSpeed;
  if (!offsetApplied) {
    currentMillis += offsetMillis;
  }
  currentMinutes = (float)currentMillis / 60000;
  reporter += timeSpeed;
  if (reporter > 99) {
    reporter = 0;
  }
  if (reporter == 0) {
    /*Serial.print(", ");
      Serial.print((long)pastMillis);
      Serial.print(currentMillis);
      Serial.print(", ");
      Serial.print((int)(currentMinutes / 60));
      Serial.print(":");
      if (((int)currentMinutes % 60) < 10) Serial.print("0");
      Serial.print((int)currentMinutes % 60);*/
  }
}

//*************************YEAR_COUNTER*************************//
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
  Serial.println("Date set.");
}

//*************************CALCULATE_DAWN/DUSK_TIMES*************************//
void daylightHours() { // sunrise and sunset times based on 2020 Southampton sun graph
  dawnTime = 3600000 * (6 + 2.15 * cos(((float)day + 7) * 2 * PI / 365.25));
  if (wakeTime >= dawnTime) {
    dawn = false;
    dawnTime = wakeTime;
  }
  else {
    dawn = true;
  }
  duskTime = 3600000 * (18.2 - 2.2 * cos(((float)day + 13) * 2 * PI / 365.25));
  nightTime = 86400000 + wakeTime - 30600000;
  if (nightTime > 86400000) {
    nightTime -= 86400000;
  }
  if (duskTime >= nightTime) {
    dusk = false;
    duskTime = nightTime;
  }
  else {
    dusk = true;
  }
  Serial.println("Transition times set.");
}

//*************************WRITE_COLOURS_TO_OUTPUT*************************//
void updateClr() {
  if (j >= sample) {
    j = 0;
  }
  pot[j] = analogRead(TRIGGER);  // read the value from the potentiometer
  //total += pot[j];
  total = 0;
  for (int i = 0; i < sample; i++) {
    total += pot[i];
  }
  val = total / sample;
  /*Serial.print(val);
    Serial.print(", ");
    Serial.println(total);*/
  j++;
  //total -= pot[j];
  //if (brightLock && val >= 1023) brightLock = false;
  if ((brightLock && ((1020 < int(bright * 1023)) || (val + 3 < int(bright * 1023)))) && !brightUnlock) {
    brightUnlock = true;
    Serial.println("Brightness unlock ready.");
  }
  if (brightUnlock && val >= int(bright * 1023)) {
    brightLock = false;
    brightUnlock = false;
    Serial.println("Brightness unlocked.");
    analogWrite(RED, 0);  // indicate that brightness has been unlocked
    analogWrite(GREEN, 0);
    analogWrite(BLUE, 0);
    delay(25);
  }
  if (!brightLock) {
    bright = (float)val / 1023;
  }
  analogWrite(RED, currentClr[0]*bright); // set all colours
  analogWrite(GREEN, currentClr[1]*bright);
  analogWrite(BLUE, currentClr[2]*bright);
}

//*************************SET_NEXT_TRIGGER*************************//
void correctTrigger(unsigned long nextTime) { // used to allow fading an hour before transition time without messing with times
  if (nextTime < 3600000) {
    //triggerTime = 82800000 + nextTime;
  }
  else {
    //triggerTime = nextTime - 3600000;
  }
}

// This #include statement was automatically added by the Particle IDE.
#include <elapsedMillis.h>

// This #include statement was automatically added by the Particle IDE.
#include <LiquidCrystal.h>

LiquidCrystal lcd(D0, D1, D2, D3, D4, D5);

elapsedMillis cycleTimer;
elapsedMillis titleTimer;
elapsedMillis spankTimer;
elapsedMillis spankDelay;

//Controls speed of scrolling text(ms)
int textCycleLimit = 500;
//How often to check title(ms)
int titleTimerLimit = 1500;
//Time in-between spanks(Avoid track-skipping)(ms)
int doubleSpankLimit = 35;
//Controls time from last spank to track-change(ms)
int spankTimerLimit = 700;

bool firstSpank = false;

int numSpanks = 0;
int sensorValue = 0;
int counter = 0;

String current;

//We have to initialize cpy to make first iteration of loop() behave as expected
String cpy = "tempstring";

void setup() {
  // Subscribe to the integration response event
  lcd.begin(16, 1);
  Particle.subscribe("hook-response/SpankifyNext", NULL, MY_DEVICES);
  Particle.subscribe("hook-response/SpankifyPrevious", NULL, MY_DEVICES);
  Particle.subscribe("hook-response/SpankifyFetch", fetchHandler, MY_DEVICES);
  pinMode(A0, INPUT);
}

void loop() {
  String data = String(10);
  sensorValue = analogRead(A0);
    
    //First check for second-spank
  if (sensorValue > 1650 && firstSpank && spankDelay > doubleSpankLimit
     ) {
    numSpanks++;
    spankDelay = 0;
  }
  //Second check for first-spank
  else if (sensorValue > 1650 && !firstSpank) {
    numSpanks++;
    //We start our spankTimer and spankDelay from 0 here to avoid double-skipping tracks
    spankTimer = 0;
    spankDelay = 0;
    firstSpank = true;
  }
    //We only fetch the title once every 1,5 seconds(titleTimerLimit)
  if (titleTimer > titleTimerLimit ) {
    Particle.publish("SpankifyFetch", PRIVATE);
    titleTimer = 0;
  }
  
  //Scroll text over lcd. We wont scroll if the length of string is < 16
  if (cycleTimer > textCycleLimit && current.length() > 16) {
      //Wrap content when offscreen
    if (counter >= current.length()) {
      lcd.print(current);
      counter = 0;
      cycleTimer = 0;
    } else {
      lcd.scrollDisplayLeft();
      cycleTimer = 0;
      counter++;
    }
  }


    //Switch case entered if spankTimer duration has passed since last spank
  if (spankTimer > spankTimerLimit && numSpanks > 0) {
    switch (numSpanks) {
      case 1:
        Particle.publish("SpankifyNext", NULL, PRIVATE);
        break;
      case 2:
        Particle.publish("SpankifyPrevious", NULL, PRIVATE);
        break;
    }
    //Reset counters and bool after every action (Next/Previous)
    firstSpank = false;
    numSpanks = 0;
    spankTimer = 0;
  }





}

//Update LCD if previous Title(prev) is different from current Title(current)
void fetchHandler(const char *event, const char *data) {
  current = String(data);
  if (current != cpy) {
    lcd.clear();
    lcd.print(data);
    cpy = current;
  }
}

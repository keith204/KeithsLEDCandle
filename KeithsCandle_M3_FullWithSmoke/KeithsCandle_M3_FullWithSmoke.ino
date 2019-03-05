// Using technique from here https://learn.adafruit.com/multi-tasking-the-arduino-part-3/using-neopatterns 
//    see link above for a good tutorial on this multi-task-ish way of doing things.

// Flicker/Smoke Realistic Candle
// Keith Kelly 2019-03-04 www.KeithsTestGarage.com
// Milestone 3 of 3



#ifdef __AVR__
#include <avr/power.h>
#endif

// NeoPatterns will require the Adafruit NeoPixel library.  Be sure to install that.  
#include "NeoPatterns.h"


#define NEO_PIN 5    // First RGBW Strip Pin
#define NEO2_PIN 9   // Second RGBW Strip Pin
#define NEO_COUNT 3  // First RGBW Strip Count
#define NEO2_COUNT 3 // Second RGBW Strip Count
#define MIC_PIN A1   // Microphone Pin
#define FLAME_PIN A3 // IR Sensor Pin
#define SMOKE_PIN 3  // Smoke Pin
#define SMOKE_TIME_MILLIS 1200 // How long should smoke wire be heated?

#define FLAME_DURATION 2000 // millis- duration of flame before lighting candle
#define FLAME_THRESHOLD 800 // 0...1023 - analog reading of FLAME value to detect
#define MIC_THRESHOLD 950 // 0...1023 - analog reading of MIC value to detect

void Strip1Complete(); 
void Strip2Complete();

NeoPatterns Strip1(NEO_COUNT, NEO_PIN, NEO_RGBW + NEO_KHZ800, &Strip1Complete);
NeoPatterns Strip2(NEO_COUNT, NEO2_PIN, NEO_RGBW + NEO_KHZ800, &Strip2Complete);

uint32_t baseColor; // What is the ON resting color?
uint32_t offColor;  // What is the OFF resting color?


// Vars to keep track of things
bool smoking = false;
bool flaming = false;

bool lit = false;
int flame_val = 0;
int mic_val = 0;
byte rnd = 0;

// Vars for tracking pseudo-async times
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long smokeStartedMillis = 0;    // time smoking started
unsigned long flameStartedMillis = 0;        // will store last time LED was updated


void setup(){

    //Serial.begin(9600); // uncomment this to figure sensor readings.

    // initialize pins
    pinMode(FLAME_PIN, INPUT);
    pinMode(MIC_PIN, INPUT);
    pinMode(SMOKE_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT); 

    // Make sure the following pins are OFF
    digitalWrite(SMOKE_PIN, LOW);  
    digitalWrite(LED_BUILTIN, LOW); 


    // Initialize NeoPixel Strip 1
    Strip1.begin();
    Strip1.show();
    baseColor = Strip1.Color(2, 110, 0, 100);// GRBW
    Strip1.Color1 = baseColor;

    // Initialize NeoPixel Strip 2
    Strip2.begin();
    Strip2.show();
    baseColor = Strip2.Color(2, 110, 0, 100);// GRBW
    Strip2.Color1 = baseColor;
    
    blowOutCandle(false);  // Make sure candle is off (but don't smoke)

}


void loop(){
    smokeCheck(); // Check if we should stop smoking.  
    checkSensors(); // Check for fire and strong winds
    Strip1.Update();
    Strip2.Update();
}

// Checks for fire and strong winds every 50 milliseconds
void checkSensors(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 50) { // only check every 50ms
    flame_val = analogRead( FLAME_PIN );
    mic_val = analogRead( MIC_PIN );
    previousMillis = currentMillis;
    
    //  Uncomment below to check sensor readings.  
    //    Serial.print(mic_val);
    //    Serial.print(" : ");
    //    Serial.println(flame_val);
  }



  if (lit && mic_val > MIC_THRESHOLD){ // MIC REGISTERED SOUND.  BLOW OUT.
    blowOutCandle(true); // Turn off light (and start SMOKING!)
    lit = false;
  }

  if (!lit){ // if candle isn't lit
    if (flame_val > FLAME_THRESHOLD)  { // if flame is detected
        
        // record the starting time (if flame not yet detected)
        if (!flaming){  // if not flaming
          flaming = true; 
          flameStartedMillis = millis();  // record time that flaming begins
        }
        else if (checkFlame()){ // has flame been flaming long enough?          
          lightCandle(); // ok finally light it up. 
          lit = true; 
        }
    }
    else{ // no flame detected
      flaming = false; 
    }
    
  }  
}


// Check if flame has been detected long enough.
bool checkFlame(){    
  unsigned long currentMillis = millis(); // get current time
  if (currentMillis - flameStartedMillis >= FLAME_DURATION) {
    return true;  
  }
  else
    return false;
}


//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// The below configuration routines are the same except for the strip
// being used.  I kept them separate so the strips can be configured
// with different ranges if desired.

void Strip1Complete(){
    Strip1.Reverse();

    if (Strip1.Direction == REVERSE){ // go back down
        Strip1.Interval = random(5,22); // choose random speed in range
    }
    else{  // pattern ended.  Stop, then trigger re-flicker
        
        /*  This needs an explanation:  
            
              rnd = random(random(45,55),random(80,255));

            You'd think that we could just do random(45,255), right?

            Selecting the low and high randoms first helps make sure that
            the ending random number is *more likely* to fall within a limited range.  

            When observing the flickering of a real candle, the flame generally bounces
            around in a certain range.  But, every once in a while, it spikes high or low.

            This random(random, random) solves that problem!  #proudnerddadmoment
        */
        rnd = random(random(45,55),random(80,255)); 

        Strip1.Pixel = random(0,Strip1.numPixels()); // pick a random Pixel
        Strip1.Interval = 1; 
        Strip1.Color2 = Strip1.Color(2,rnd,0,rnd-10); //GRBW random red and random white-10
    }
}

void Strip2Complete(){
    Strip2.Reverse();

    if (Strip2.Direction == REVERSE){ // go back down        
        Strip2.Interval = random(5,22); // choose random speed in range
    }
    else{  // pattern ended.  Stop, then trigger re-flicker
        rnd = random(random(45,55),random(80,255)); // see explanation in Strip1Complete().
        Strip2.Pixel = random(0,Strip2.numPixels()); // pick a random Pixel
        Strip2.Interval = 1;
        Strip2.Color2 = Strip2.Color(2,rnd,0,rnd-10); //GRBW random red and random white-10
    }
}

// (pseudo)ASYNC light the candle by starting the animations
void lightCandle(){
    Strip1.Flicker(Strip1.Color1, Strip1.Color2, 20, 5);
    Strip2.Flicker(Strip2.Color1, Strip2.Color2, 20, 5);
}

void blowOutCandle(bool shouldSmoke){
  if(shouldSmoke)
    smokeStart();
    
  cWipe(offColor, 10); // synchronously turn the candle off.

  // We don't want no stinking patterns.
  Strip1.ActivePattern = NONE; 
  Strip2.ActivePattern = NONE;
}


// SMOKING METHODS
void smokeStart(){
  digitalWrite(SMOKE_PIN, HIGH); // HEAT SMOKE WIRE
  smoking = true;
  smokeStartedMillis = millis(); // record time smoking started
}

// Stop smoking if the time has come.
void smokeCheck(){
  if (!smoking)
    return; // if not smoking, get out of here.
    
  unsigned long currentMillis = millis();
  if (currentMillis - smokeStartedMillis >= SMOKE_TIME_MILLIS) {
    smokeStop();    // smoke break is over
  }
}

// Ok seriously now stop it
void smokeStop(){
  digitalWrite(SMOKE_PIN, LOW); // TURN OFF SMOKE WIRE
  smoking = false;
}



// synchronous color wipe
// To avoid smoke becoming fire, be sure to only 
//   use synchronous methods like this SPARINGLY and when you
//   are sure it won't interfere with the smoking.  
//
// In other words, here's something that would be very bad:
//
//  smokeStart();
//  cWipe(offColor, 1000);   // CPU would get caught up in
//                              here for a while and would 
//                              not stop the smoking process
//                              
//  At this point, I'm only using it directly after smokeStop();

void cWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < Strip1.numPixels(); i++) {
    Strip1.setPixelColor(i, c);
    Strip1.show();
    delay(wait);
  }
  for (uint16_t i = 0; i < Strip2.numPixels(); i++) {
    Strip2.setPixelColor(i, c);
    Strip2.show();
    delay(wait);
  }
}


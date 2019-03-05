
// FIRST TEST OF MIC AND FLAME SENSOR
// Keith Kelly 2019-03-04 www.KeithsTestGarage.com
// Milestone 1 of 3


#define MIC_PIN A1   // Microphone Pin
#define FLAME_PIN A3 // IR Sensor Pin

#define FLAME_DURATION 2000 // millis- duration of flame before lighting candle
#define FLAME_THRESHOLD 800 // 0...1023 - analog reading of FLAME value to detect
#define MIC_THRESHOLD 950 // 0...1023 - analog reading of MIC value to detect



// Vars to keep track of things
bool smoking = false;
bool flaming = false;

bool lit = false;
int flame_val = 0;
int mic_val = 0;

// Vars for tracking pseudo-async times
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long flameStartedMillis = 0;    // will store last time LED was updated

void setup(){

    Serial.begin(9600); // uncomment this to figure sensor readings.

    // initialize pins
    pinMode(FLAME_PIN, INPUT);
    pinMode(MIC_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT); 

    digitalWrite(LED_BUILTIN, LOW); // Turn OFF built-in LED

    blowOutCandle();  // Make sure candle is off (but don't smoke)

}


void loop(){
    checkSensors(); // Check for fire and strong winds
}

// Checks for fire and strong winds every 50 milliseconds
void checkSensors(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 50) { // only check every 50ms
    flame_val = analogRead( FLAME_PIN );
    mic_val = analogRead( MIC_PIN );
    previousMillis = currentMillis;
    
    //  Uncomment below to check sensor readings.  
    Serial.print(mic_val);
    Serial.print(" : ");
    Serial.println(flame_val);
  }


  if (lit && mic_val > MIC_THRESHOLD){ // MIC REGISTERED SOUND.  BLOW OUT.
    blowOutCandle(); // Turn off light (and start SMOKING!)
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



// (pseudo)ASYNC light the candle by starting the animations
void lightCandle(){
    digitalWrite(LED_BUILTIN, HIGH);
}

void blowOutCandle(){    
  digitalWrite(LED_BUILTIN, LOW);
}




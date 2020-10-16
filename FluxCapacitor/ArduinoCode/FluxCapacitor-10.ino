/*
 * Flux Capacitor code
 *
 * for
 * Arduino Pro Mini (ATMega168, 5V, 16MHz) 
 * (should also work for Uno or Nano)
 *
 *  10/16/2020  v10
 *              copy of v09 cleaned up and added comments for GitHub
 *
 *
 * How the Arduino pins are connected:
 *
 * Pin(s)	connected to: 
 * TX/RX	RX/TX of DFPlayer Mini (via 1k resistors)
 * 2		"busy" signal from DFPlayer Mini
 * 3,5,6,9	transistors to Y-bar LEDs (from outer to inner) via R=2k2
 * 10		transistor to the 4x2 LEDs in the sides of the enclosure
 * 11		transistor to the center LED
 * A2,A3	push buttons to set volume and mode of operation
 *
 *
 * uses sound files on microSD card in folders 01,02,03,07,08,09
 * 1:  8 clips   constant noise  (4s - 39s) 
 * 2: 14 clips   longer sparking noise (aggressive)   (4-33s)   
 * 3: 18 clips   short noise (1-3s)
 * 7:  4 clips   BTTF  3x"bling" + "88mph, serious..."
 * 8:  4 clips   announce 4 modes
 * 9:  4 clips   announce 4 volume levels
 *
 *
 * The first push button is used to set the volume in four levels
 * (one is "silent")
 *
 * Easter egg: if both push buttons are pressed at startup, it plays
 * the sound clip "... 88 miles per hour, you see some serious shit"
 *
 */



// ===========================================================================
// ===========================================================================
// === Initialization
// ===========================================================================
// ===========================================================================

// -----------------------------------------------------------------
// --- global variables
byte currentMode = 3;    // four modes of operation: 0, 1, 2, 3 
                         // start in mode 3: most activity

byte startupVolume = 2;  // DFPlayer volume-level at startup (in range:0-3)
//byte startupVolume = 0; // set to "0" if you want it to start in silent mode

// -----------------------------------------------------------------
// --- LED bars in Y-piece
const byte pinLedBar[4] = {3,5,6,9};         // from outer to inner
const byte pinLedCenter = 10;                // bright center LED in enclosure
const byte pinLedBox = 11;                   // all 4x2 LEDs in the enclosure

// -----------------------------------------------------------------
// --- two push buttons
const byte maxButton = 2;                    // number of buttons
const byte pinPushButton[2] = {A2,A3};

// -----------------------------------------------------------------
// --- DFPlayer - connected to hardware Serial - TX/RX
const byte pinDfpBusy = 2;
const byte dfpVolume[4] = {0x12, 0x15, 0x19, 0x23}; // four volume levels


// ===========================================================================
// ===========================================================================
// === Setup
// ===========================================================================
// ===========================================================================

void setup() {
  Serial.begin(9600);   // communiction with DFPlayer

  // --- all LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(pinLedBar[i], OUTPUT);
  }
  pinMode(pinLedCenter, OUTPUT);
  pinMode(pinLedBox, OUTPUT);
  
  // --- initialize push button pins as inputs with pull-ups
  for (byte i=0; i<maxButton; i++) {
    pinMode(pinPushButton[i], INPUT_PULLUP);
  } 
  // - check if button is pressed at startup
  byte buttonPressed = 0; 
  if (digitalRead(pinPushButton[0]) == LOW || digitalRead(pinPushButton[1]) == LOW) buttonPressed = 1;
 
  // --- initialize random number generator with unconnected analog input
  unsigned int startSeed = analogRead(A0); 
  delay(70);
  startSeed += analogRead(A1);
  delay(50);
  startSeed += analogRead(A0);
  randomSeed(startSeed);
    
  // --- init DFPlayer
  pinMode(pinDfpBusy, INPUT);         // init Busy pin from DFZPlayer (lo: file is playing / hi: no file playing)
  dfpExecute(0x3F, 0x00, 0x00);       // Send request for initialization parameters
  //while (Serial.available()<10)     // Wait until initialization parameters are received (10 bytes) - does not work - weird!
  delay(80);                          // have >20ms delays between commands   
  dfpExecute(0x06,0x00,dfpVolume[startupVolume]);   // set volume
  delay(80);                          // have >20ms delays between commands

  delay(400);
  
  // - DFPlayer play startup sound
  //
  // - check: if one button is pressed, play "at 88 mph this baby ... serious..."
  byte track = 1 + random(3);   // randomly select one of the tracks 001-003 in folder 07 (BTTF sounds)         
  if (buttonPressed == 1) {
    dfpPlay(0x07,4,0);    // 007-04 "88 mph -> serious"
    delay(8000);          // ... and wait for sound clip to finish
  } else {
    dfpPlay(0x07,track,0);
  }
  
  delay(100);
  initialTest();       // play at startup: Y-LEDs & box LEDs

}


// ===========================================================================
// ===========================================================================
// === Loop
// ===========================================================================
// ===========================================================================

void loop() {

  // --- read push buttons
  readButtons();

  // --- main Logic
  mainLogic();
   
  // --- continue with the LEDs patterns in Y-LED bars
  //     and the enclosure LEDs and center LED
  setLedBars(0,0);
  setBoxLed(0);
  
}


// ===========================================================================
// ===========================================================================
// === Subroutines
// ===========================================================================
// ===========================================================================


// --- some LED test pattern that is displayed at startup
void initialTest() {
  const unsigned int t=230;
  const unsigned int t2=1;
  for (byte i=0; i<4; i++) {
    analogWrite(pinLedBar[i],20);
    delay(t);
    analogWrite(pinLedBar[i],255);
    delay(t);
    analogWrite(pinLedBar[i],0);
  }
  delay(t);
  
  for (byte i=0; i<255; i++) {
    analogWrite(pinLedCenter,i);
    delay(t2);
  }
  delay(t);
  
  for (byte i=0; i<255; i++) {
    analogWrite(pinLedBox,i);
    analogWrite(pinLedCenter,255-i);
    delay(t2);
  }
  delay(t);
  for (byte i=0; i<255; i++) {
    analogWrite(pinLedBox,255-i);
    analogWrite(pinLedCenter,i);
    delay(t2);
  }
  
  delay(t);
  analogWrite(pinLedCenter,0);
  analogWrite(pinLedBox,0);
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// --- main Logic
// ----------------------------------------------------------------------------
void mainLogic() {
  static byte currentState = 0;   //   0:waiting   1:busy   2:break
  static unsigned long previousTime = 0;
  static unsigned long duration = 1000;
  static unsigned long timeLastHigh = 30000;
  static byte activityHigh = 0;       // indicates if current activity is "high" - not used
  const unsigned long timeBeforeHigh[4]  = {3600000000, 360000, 150000, 66000};  // 100h, 6min, 2 1/2min, 66sec
  const unsigned long lowDurationBase[4] = {25000, 22000, 14000, 11000};
  const unsigned long lowDurationRand[4] = {55000, 50000, 19000, 19000};
  const unsigned long highDurationBase[4] = { 6000,  6000, 6000, 5000};
  const unsigned long highDurationRand[4] = {11000, 11000, 7000, 4000};
  
    
  if (currentState == 0) {    // ============= ready to start next activity ==============
    activityHigh = 0;
   
    previousTime= millis();
    currentState = 1;
    if ((millis()-timeLastHigh) < timeBeforeHigh[currentMode]) {   // --- start new low-activity
      duration = lowDurationBase[currentMode] + random(lowDurationRand[currentMode]);
      startActivity(0,currentMode);
    } else {                                                       // --- start new high activity
      timeLastHigh = millis();
      duration = 6000 + random(11000);
      activityHigh = 1;
      startActivity(1,currentMode);
    }
     
  } else if (currentState == 1) {    // ============= activity in progress ======================
    if ((millis()-previousTime) > duration && digitalRead(pinDfpBusy) == HIGH) {  // end activity?
      previousTime = millis();
      duration = (4-currentMode)*random(1500) + 2000;
      currentState = 2;
      // send stop to LEDs
      setLedBars(99,0);
      setBoxLed(99);
      activityHigh = 0;
    }
    
  } else if (currentState == 2) {   // ============ a break before the next activity ============
     if ((millis()-previousTime) > duration) {
      //previousTime = millis();
      currentState = 0;
      activityHigh = 0;
    }
  }
  
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// --- start activity   loHi:  0=lo  1=hi
// ----------------------------------------------------------------------------
void startActivity(byte loHi, byte mode) {
  byte track = 0;
  byte chanceSoundLow[4] = {9,8,7,3};     // the chance for playing sound in low activity mode
   
  if (loHi == 0) {        // --- low activity
    byte rd = random(10);
    if (rd < 3) {
      setLedBars(2,10);        // x3
    } else if (rd < 6) {
      setLedBars(2,5);         // x3
    } else if (rd < 8) {
      setLedBars(2,2);         // x2
    } else if (rd < 9) {
      setLedBars(4,10);        // x1
    } else {
      setLedBars(4,5);         // x1
    }
    if (random(chanceSoundLow[mode]) < 1) {  // 1:n chance for sound
      track = 1 + random(26);      // 8 + 18 tracks in folders 01&03
      if (track < 19) {
        dfpPlay(0x03,track,0);     // short track in folder 03 
      } else {
        dfpPlay(0x01,track-18,0);  // long track in folder 01
      }
    }
    
  } else {                // --- high activity
    byte i = 2 + random(4);
    setBoxLed(i);    // 2-5
     byte rd = random(5);
    if (rd == 0) {
      setLedBars(4,1);  // time: 1,2,5,10 
    } else {
      setLedBars(2,1);  // time: 1,2,5,10
    }
    // test
    //setBoxLed(2);    // 2 fast flicker - perfect
    //setBoxLed(3);    // 3
    //setBoxLed(4);    // 4 fast/slow/break perfect
    track = 1 + random(14);
    dfpPlay(0x02,track,0);        // long sparking track in folder 02
  }
  
}
  

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// --- code for Y-shaped LED bars
//    0: continue
//    1: stop immediately
//   99: stop soon - after finalizing period
//
//  currently using: setLedBars(2,3-11); 
//  - make finer steps
//
// ----------------------------------------------------------------------------

void setLedBars(byte stateLedBars, byte timeStep) {
  static byte stateCurrent = 0; 
   
  static unsigned long previousTime = 0;
  const byte stepSize2 = 25;              // brightness steps in mode=2
  const byte stepSize3 =  9;              // brightness steps in mode=3
  static unsigned int timeInterval = 2;   // time for each step - set in call
  //static unsigned int extraTime = 0;      // extra time at the end

  static byte level = 0;
  static byte steps = 0;
  static char currentLed = 0;
  static byte stopSoon = 0;
  static char forthBack = 1;


  // --- update the state
  if (stateLedBars != 0 && stateCurrent != stateLedBars) {
    if (stateLedBars == 99 && stateCurrent != 0) {   // --- catch & store "stop soon" signal
      stopSoon = 1;

    } else {
      stateCurrent = stateLedBars;
      timeInterval = timeStep;
      stopSoon = 0;
      // reset values
      level = 0;
      currentLed = 0;
      steps = 0;
      forthBack = 1;
      //extraTime = 0;
    }
  }
  
  // --- continue according to the current state  
  if (stateCurrent == 0) {                                                // --- do nothing
    
  } else if (stateCurrent == 1) {                                         // --- turn off - all 4 pins to 0
    stateCurrent = 0;
    for (int i = 0; i < 4; i++) { 
      analogWrite(pinLedBar[i],0);
    }
    // reset values
    level = 0;
    currentLed = 0;
    steps = 0;
    forthBack = 1;
    //extraTime = 0;
        
  } else if (stateCurrent == 2) {                     // --- regular operation - super fast
    if ((millis()-previousTime) > timeInterval) { 
      previousTime = millis();

      if (level < (255-stepSize2)) {
        level = level + stepSize2;
      } else {
        level = 0;
        currentLed += 1;
        if (stopSoon == 1 && currentLed > 4 && currentLed < 10) {
          stopSoon = 0;      // reset flag
          stateCurrent = 1;  // stop in next call
        }
        if (currentLed > 9) currentLed = 0;
      }
      if (currentLed<4) analogWrite(pinLedBar[currentLed],level);
      if (currentLed>0 && currentLed<5) analogWrite(pinLedBar[currentLed-1],250-level);
    }
    
  }  else if (stateCurrent == 3) {                     // --- regular operation - less fast - smaller steps
    if ((millis()-previousTime) > timeInterval) { 
      previousTime = millis();

      if (level < 250) {
        level = level + stepSize3;
      } else {
        level = 0;
        currentLed += 1;
        if (stopSoon == 1 && currentLed > 4 && currentLed < 10) {
          stopSoon = 0;      // reset flag
          stateCurrent = 1;  // stop in next call
        }
        if (currentLed > 9) currentLed = 0;
      }
      if (currentLed<4) analogWrite(pinLedBar[currentLed],level);
      if (currentLed>0 && currentLed<5) analogWrite(pinLedBar[currentLed-1],250-level);
    }
  } else if (stateCurrent == 4) {                     // --- forth and back - rare regular
    if ((millis()-previousTime) > timeInterval) { 
      previousTime = millis();

      if (level < (255-stepSize2)) {
        level = level + stepSize2;
      } else {
        level = 0;
        currentLed += forthBack;
        if (stopSoon == 1 && currentLed > 4 && currentLed < 10) {
          stopSoon = 0;      // reset flag
          stateCurrent = 1;  // stop in next call
        }
      }
      if (currentLed<4 && currentLed>-1) analogWrite(pinLedBar[currentLed],level);
      if (forthBack==1 && currentLed>0 && currentLed<4) analogWrite(pinLedBar[currentLed-1],250-level);
      if (forthBack==-1 && currentLed>-1 && currentLed<3) analogWrite(pinLedBar[currentLed+1],250-level);
      if (currentLed > 5) forthBack = -1;     //currentLed = 0;
      if (currentLed < -2) forthBack = 1;
    }
    
  } 


}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// --- LEDs in box: center & frame
//
//     stateBoxLed: 2-4
//     2: flicker
//     3: longer fade
//     4: longer fade
//     5: alternate center vs. box LEDs
// ----------------------------------------------------------------------------
void setBoxLed(byte stateBoxLed) {
  static byte stateCurrent = 0;
  static unsigned long previousTime = 0;
  static unsigned long startTime = 0;
  static unsigned int timeInterval2 = 1;   // time for each step in mode 2
  static unsigned int timeInterval3 = 10;  // time for each step in mode 3
  static unsigned int timeInterval4 = 8;   // time for each step in mode 4
  static unsigned int timeInterval5 = 1;   // time for each step in mode 5
  static unsigned int extraTime = 0;
  const byte stepSize2 = 36;               // brightness steps for mode 2
  const byte stepSize3 = 2;                // brightness steps for mode 3
  const byte stepSize4 = 2;                // brightness steps for mode 4
  const byte stepSize5 = 2;                // brightness steps for mode 5
  
  static byte level = 0;
  static byte updown = 1;       // =1:up    =2:down   =0:break
  static byte stopSoon = 0;
  static byte maxLevel = 130;   
     
  // catch: if called with 99 but no action is in progress
  
  // --- update the state
  if (stateBoxLed != 0 && stateCurrent != stateBoxLed) {
    if (stateBoxLed == 99 && stateCurrent != 0) {   // --- catch & store "stop soon" signal
      stopSoon = 1;
    } else {
      stateCurrent = stateBoxLed;
      startTime = millis();
      stopSoon = 0;
      // - reset values
      level = 0;
      updown = 1;
      extraTime = 0;
      maxLevel = 130;
    }
  }

  // --- continue according to the current state  
  if (stateCurrent == 0) {                                                // --- 0: do nothing
    
  } else if (stateCurrent == 1) {                                         // --- 1: turn off 
    stateCurrent = 0;
    analogWrite(pinLedBox,0);
    analogWrite(pinLedCenter,0);
    // - reset values
    level = 0;
    updown = 1;
    extraTime = 0;
    
  } else if (stateCurrent == 2) {                                         // --- 2: fast flicker
    if ((millis()-previousTime) > (timeInterval2+extraTime)) { 
      previousTime = millis();  
      analogWrite(pinLedBox,level);
      analogWrite(pinLedCenter,level);
      
      if (updown == 1) {            // ramping up
        if (level < (225-stepSize2)) {
          level = level + stepSize2;
        } else {
          updown = 2;
          extraTime = timeInterval2;          // extra time during ramp down
        }
      } else if (updown == 2) {                      // ramping down
        if (level > stepSize2) {
          level = level - stepSize2;
        } else {
          level = 0;
          updown = 3;
          extraTime = 0;
        }
      } else {              // break
        if (stopSoon == 1) {
          stopSoon = 0;      // reset flag
          stateCurrent = 1;  // stop in next call
        }
        updown++;
        if (updown > 60) updown = 1;
      }
    }
  } else if (stateCurrent == 3) {                                         // --- 3: slow fade 
    if ((millis()-previousTime) > (timeInterval3+extraTime)) { 
      previousTime = millis();  
      analogWrite(pinLedBox,level);
      analogWrite(pinLedCenter,level);
      
      if (updown == 1) {            // ramping up
        if (level < (95-stepSize3)) {
          level = level + stepSize3;
        } else {
          updown = 2;
          extraTime = 0;
        }
      } else if (updown == 2) {                      // ramping down
        if (level > stepSize3) {
          level = level - stepSize3;
        } else {
          level = 0;
          updown = 3;
          extraTime = 0;
        }
      } else {              // break
        if (stopSoon == 1) {
          stopSoon = 0;      // reset flag
          stateCurrent = 1;  // stop in next call
        }
        updown++;
        extraTime = 15;
        if (updown > 12) updown = 1;
      }
    }
  } else if (stateCurrent == 4) {                                         // --- 4: fast rise/slow fade
    if ((millis()-previousTime) > (timeInterval4+extraTime)) { 
      previousTime = millis();  
      analogWrite(pinLedBox,level);
      analogWrite(pinLedCenter,level);
      
      if (updown == 1) {            // ramping up
        if (level < (125-stepSize4)) {
          level = level + stepSize4*2;
        } else {
          updown = 2;
          extraTime = timeInterval4*1;
        }
      } else if (updown == 2) {                      // ramping down
        if (level > stepSize4) {
          level = level - stepSize4;
        } else {
          level = 0;
          updown = 3;
          extraTime = 0;
        }
      } else {              // 99: break
        if (stopSoon == 1) {
          stopSoon = 0;      // reset flag
          stateCurrent = 1;  // stop in next call
        }
        updown++;
        if (updown > 50) updown = 1;
      }
    }
  } else if (stateCurrent == 5) {                                     // ---5: center/sides alternate - no fade
    if ((millis()-previousTime) > (timeInterval5+extraTime)) { 
      previousTime = millis();
      analogWrite(pinLedBox,(maxLevel-level));
      analogWrite(pinLedCenter,level);
      if (updown == 1) {          // Box on, center off
        level = maxLevel;
        extraTime = 200;
        updown = 2;
      } else if (updown == 2) {   // Box off, center on
        level = 0;
        extraTime = 100;
        maxLevel = 10;          
        updown = 3;
      } else if (updown == 3) {   // Box on, center off -dim
        level = maxLevel;
        extraTime = 200;          
        updown = 4;
        if (stopSoon == 1) {
          stopSoon = 0;      // reset flag
          stateCurrent = 1;  // stop in next call
        }
      } else if (updown == 4) {   // Box off, center on -dim
        level = 0;
        extraTime = 100;
        maxLevel = 130;          
        updown = 1;
      }
    }
  }

}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// --- read buttons
// ---------------------------------------------------------------------------------
void readButtons()
{
  static unsigned long previousTime = 0;  
  const byte timeInterval = 3;   // every 3ms -> pick a short time interval, you don't 
                                 //              need to check the button at 10kHz
  byte a[maxButton] = {};        // array to store the latest readings for the buttons

  static byte volumeLevel = startupVolume;   // start with volume level -> defined in global variable

  // - check all buttons
  if ((millis()-previousTime) > timeInterval) {
    previousTime = millis();
    for (byte i=0; i<maxButton; i++) {
      a[i] = checkButtons(i);  
    }
 
    if (a[0] > 0) {             // change volume
      volumeLevel  = volumeLevel + 1;
      if (volumeLevel > 3) volumeLevel = 0; 

      dfpExecute(0x06,0x00,dfpVolume[volumeLevel]); // set volume
      delay(30); 
      dfpPlay(0x09,volumeLevel+1,1);   // play number of vol level (forced)
      delay(1900);
    }
    
    if (a[1] > 0) {         // change mode
      setLedBars(1,0);  
      setBoxLed(1);
      currentMode = currentMode +1;
      if (currentMode > 3) currentMode = 0;
      delay(30); 
      dfpPlay(0x08,currentMode+1,1);   // play number of mode (forced)        << initially: set to 3
      delay(1900);      
    }
    
  }
  
}


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// --- check one button for short or long push (code from blog)
// ---------------------------------------------------------------------------------
//     returns: 0-none  1-short  2-long
//
byte checkButtons(byte buttonNo) {
  const unsigned long timeDebounce = 100; // time to debounce 
  const unsigned long timeLong = 400;    // minimum time for Long press 
  const unsigned long timeBreak = 200;   // time interval after button release, 
                                         //  before ready for next press 
  static byte state[maxButton] = {};     // this initializes all elements to zero
  static unsigned long previousTime[maxButton] = {};  // this initializes all elements to zero
  byte r = 0;

  r = 0;      // 0:not  1:short  2:long

  if (state[buttonNo] == 0) {             // --- no button has been pressed - check if 
    if (digitalRead(pinPushButton[buttonNo]) == LOW) {
      previousTime[buttonNo] = millis();
      state[buttonNo] = 1;
    }
  } else if (state[buttonNo] == 1) {  // --- button was pressed - check for how long
    if ( (millis()-previousTime[buttonNo]) > timeDebounce) {
      if ( (millis()-previousTime[buttonNo]) < timeLong) {
        if ( digitalRead(pinPushButton[buttonNo]) == HIGH) { // released -> short press
          previousTime[buttonNo] = millis();
          state[buttonNo] = 3;
          r = 1;
        }
      } else {                        // it was a long press
        state[buttonNo] = 2;
        r = 2;
      }
    }
  } else if (state[buttonNo] == 2) {  // --- wait for long button press to end
    if (digitalRead(pinPushButton[buttonNo]) == HIGH) {
      previousTime[buttonNo] = millis();
      state[buttonNo] = 3;
    }
  } else if (state[buttonNo] == 3) {  // --- wait a little while after previous button press
    if ( (millis()-previousTime[buttonNo]) > timeBreak) {
      state[buttonNo] = 0;
    }
  }
  return r;
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// --- DFPlayer: check if not in "mute" mode - then play clip
//     input:  folder number(1-99), track number(1-255), force-play (0/1)
//     mute/unmute:   0=unmuted  1=muted
//        toggle mute flag:   folder=0, track=0 
//        set mute flag:      folder=0, track=1
//        erase mute flag:    folder=0, track=2
//
void dfpPlay(byte folder, byte track, byte force)
{ 
  static byte mute = 0;

  if (folder == 0) {
    if (track == 0) {
      mute = 1 - mute;     // toggle between 0 and 1
    } 
    else if (track == 1) {
      mute = 0;            // unmute  (=0)
    }
    else if (track == 2) {
      mute = 1;            // mute  (=1)
    }
    if (mute == 1) {
      dfpStop();                        // play no clip - really want silence
    } else if (mute == 0) {
      dfpExecute(0x0F,2, 1+random(4));  // play one of four beep sounds in folder 02
      delay(22);
    }
  } else if (mute == 0) {              // if not in mute mode: play
    if (force == 1) dfpStop();
    dfpExecute(0x0F,folder,track);
    delay(22);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// --- DFPlayer: check if a sound clip is playing - then stop it
void dfpStop()
{
  if (digitalRead(pinDfpBusy) == LOW) {   // if playing:
    dfpExecute(0x16,0,0);                 //     stop
    delay(22);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// --- DFPlayer: excecute the DFPlayer command with two parameters - on Serial
void dfpExecute(byte CMD, byte Par1, byte Par2)
{ 
  # define Start_Byte     0x7E
  # define Version_Byte   0xFF
  # define Command_Length 0x06
  # define Acknowledge    0x00 
  # define End_Byte       0xEF
  // Calculate the checksum (2 bytes)
  uint16_t checksum =  -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  // Build the command line
  uint8_t Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
               Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
  // Send the command line to DFPlayer
  for (byte i=0; i<10; i++) Serial.write( Command_line[i]);
}


/*
  Micro Time Circuits: Flux Capacitor & SID & Time Circuits & Analog Gauges
  Arduino Pro Mini 328 16MHz 5V

  2019/03/03  geometry of flux cap & SID
  2019/03/05  adopting SID code - works
  2019/03/06  finalize Flux Capacitor & SID code
              also: test code for Time Circuits
              -> v01
  2019/03/06  start code for Time Circuits 
              options: alternate TC and Flux+SID - or: two stand-alone circuits
  2019/03/07  finalize TC - start speedometer
  2019/03/15  finalize speedometer, add Analog Gauges
              TC: am/pm labels, SID: dark LEDs-darker circles(lenses)
  2019/03/18  final version - used in brick stage 
  
*/



#include <TFT_ST7735.h> // Hardware-specific library
#include <SPI.h>

//TFT_ST7735 tft = TFT_ST7735();       // rot0  Invoke custom library
TFT_ST7735 tft = TFT_ST7735(104,160);       // rot0  Invoke custom library
//TFT_ST7735 tft = TFT_ST7735(160,80); 

// global variables
byte activity = 0;
byte peak[10];
byte lastPeak[10];

// define the colors
uint16_t colSpeedoCase = tft.color565(220,170,65);
uint16_t colAgCase = tft.color565(33,39,43);
uint16_t colAgBright = tft.color565(205,150,105);         // warm white
uint16_t colAgOff = tft.color565(53,0,0);     
uint16_t colAgEmpty = tft.color565(200,0,0);     

uint16_t colSidCase = tft.color565(11,10,4);
uint16_t colSidLenses = tft.color565(4,4,1);
uint16_t colSidLenses2 = tft.color565(16,15,9);
uint16_t colSidLine = tft.color565(44,40,16);

uint16_t colGasBottle = tft.color565(109,0,6);
uint16_t colGasBottle2 = tft.color565(80,0,4);
uint16_t colGasBottle3 = tft.color565(50,0,3); 
uint16_t colGasBottle4 = tft.color565(121,0,7);
  
uint16_t colTcCase = tft.color565(8,8,8);
uint16_t colTcOpenings = tft.color565(0,0,0);
uint16_t colTcLabelsRed = tft.color565(115,0,0);
uint16_t colTcLabelsBlack = tft.color565(0,0,0);
uint16_t colTcLabelsFont = tft.color565(180,160,140);
uint16_t colTcDigits[3] = {tft.color565(200,0,0),
                         tft.color565(0,150,0),
                         tft.color565(160,160,0)};
uint16_t colTcLeds[3] = {tft.color565(150,150,0),
                         tft.color565(0,150,0),
                         tft.color565(150,150,0)};

uint16_t colFluxBase = tft.color565(25, 21, 7);

const byte noDates = 14;
unsigned int myDates[noDates][3] = {      // all month-day, year, hour-min
  {1026,1985,121},       // 8 relevant dates from the movie
  {1026,1985,122},     
  {1105,1955,600},
  {1112,1955,1838},
  {704,1776,812},
  {1225,0,1603},
  {1021,2015,1928},
  {101,1885,0},
  {609,1961,1201},       // Michael J Fox 
  {1022,1938,1125},      // Christopher Lloyd 
  {415,1959,1811},       // Tom Wilson
  {705,1966,1143},       // Claudia Wells 
  {208,1953,803},        // Mary Steenburgen
  {921,1929,1527}        // Elsa Raven
};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);
  delay(300);
  tft.init();

  tft.invertDisplay(1);
  //tft.setRotation(1);
  //Serial.println(tft.getRotation());
  
  tft.fillScreen(TFT_BLACK); 
  delay(700);
   
  unsigned int startSeed = analogRead(A1); 
  delay(130);
  startSeed += analogRead(A2);
  delay(150);
  startSeed += analogRead(A3);
  randomSeed(startSeed);
 
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void loop() { 
  static unsigned long previousTime = 0;
  static unsigned int timeSpeedoAnalog = 0;
  static unsigned int timeFluxSid = 0;
  static unsigned int timeTimeCircuits = 0;
  static byte interval;

    
  // --- Flux Capacitor & SID ---------------------------
  delay(500);
  tft.fillScreen(TFT_BLACK);
  delay(700);
  fluxCapEnclosure(); 
  sidEnclosure();
  for (byte i=0; i<10; i++) {
   peak[i] = 0;
   lastPeak[i] = 0;
  }
  delay(1200);
  timeFluxSid = 44000 + random(24000);
  previousTime = millis();
  while ((millis() - previousTime) < timeFluxSid ) {
    nextSid();
    updateSid();
    operateFlux();
  }
  delay(600);
   
  // --- Time Circuits ------------------------------
  tft.fillScreen(TFT_BLACK); 
  delay(1400);
  tcEnclosure();
  delay(1100);
  timeTimeCircuits = 38000 + random(21000);
  previousTime = millis();
  while ((millis() - previousTime) < timeTimeCircuits) {
    tcOperate();
    tcBlink();
  }
  delay(800);
 
  // --- Speedometer ---------------------------
  tft.fillScreen(TFT_BLACK);
  delay(700);
  speedoEnclosure();
  agEnclosure(); 
  agNeedles(0,0,0);
  delay(250);
  tcNumber(0,4,0);

  agBlink();  
  timeSpeedoAnalog = 48000 + random(27000);
  previousTime = millis();
  while ((millis() - previousTime) < timeSpeedoAnalog ) {
    updateSpeedoAnalog();
  }
  agBlink();
  
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void updateSpeedoAnalog() {
  static unsigned long previousTime = 0;
  static unsigned long previousTimeS= 0;
  static int pos[3] = {4,4,6};
  const byte maxPos[3] = {4,4,6};
  static int move[3] = {-1,-1,-1};
  static int speed = 0;
  static int speedDir = 1;
  //static byte keep = 0;
  static unsigned long keepStart = 0;
  
  if ((millis() - previousTime) > 10 ) {
    previousTime = millis();

    // - needles
    for (byte i=0; i<3; i++) {
      if (random(200) > 196) pos[i] += move[i];
      if (pos[i] < 0) {
        pos[i] = 0;
        move[i] = - move[i]; 
      } else if (pos[i] > maxPos[i]) {
        pos[i] = maxPos[i];
        move[i] = - move[i]; 
      }
    }
    agNeedles(pos[0],pos[1],pos[2]);
   
    // - speed
    if ((millis() - keepStart) > 2900 ) {
      keepStart = 0;
      if (random(100) > 90) {
        speed += speedDir;
        if (speed > 88) {
          speed = 88;
          speedDir  = -speedDir;
          keepStart = millis();
        } else if (speed < 0) {
          speed = 0;
          speedDir  = - speedDir;
          keepStart = millis();
        }
        tcNumber(speed,4,0);
      }
    }
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void agEnclosure() { 
  tft.fillRect(60, 5 , 40, 132, colAgCase);
  tft.fillCircle(79,115,15,colTcLabelsBlack);
  tft.fillCircle(79,115,10,colAgBright);
  tft.fillCircle(79,80,15,colTcLabelsBlack);
  tft.fillCircle(79,80,10,colAgBright);
  tft.fillRoundRect(63, 10, 34, 46, 2,colTcLabelsBlack);
  tft.fillRoundRect(68, 15, 18, 36, 2,colAgBright); 
  tft.fillRect(68, 27, 6, 12,colAgOff); 

  // labels
  tft.fillRect(80, 108, 5, 15, colTcLabelsBlack); 
  tft.fillRect(80, 73, 3, 15, colTcLabelsBlack); 
  tft.fillRect(83, 75, 4, 11, colTcLabelsBlack); 
  for(byte j = 0 ; j < 7; j++) {
    tft.fillRect(90, 19+4.78*j , 1, 1, colTcLabelsFont); 
  }
}

// --------------------------------------------------------------
// --------------------------------------------------------------
void agBlink() {
  for (byte i=0; i<3; i++) {
    agEmpty(1);
    delay(900);
    agEmpty(0);
    delay(600); 
  }
}
 
// --------------------------------------------------------------
// ------------------------------------------------------------
void agEmpty(byte state) {
  if (state == 1) 
    tft.fillRect(68, 28, 5, 10,colAgEmpty);
  else
    tft.fillRect(68, 27, 6, 12,colAgOff);
}
    
// --------------------------------------------------------------
// ------------------------------------------------------------
void agNeedles(byte pos0, byte pos1, byte pos2) {
  static byte prevPosit[3] = {0,0,0};
  byte pos[3];
  pos[0] = pos0;
  pos[1] = pos1;
  pos[2] = pos2;
  
  for (byte i=0; i<3; i++) {
    agDrawNeedle(i,prevPosit[i],0); 
    agDrawNeedle(i,pos[i],1); 
    prevPosit[i] = pos[i];
  }
}

// ------------------------------------------------------------
void agDrawNeedle(byte n, byte pos, byte col) { 
  uint16_t color;
  if (col == 1) color = colTcLabelsBlack; 
  else color = colAgBright;
      
  if (n==0) {
    // needle - left
    tft.drawLine(79,111+2*pos, 73+abs(pos-2),107+4*pos, color);
  } else if (n == 1) {
    tft.drawLine(79, 76+2*pos, 73+abs(pos-2),72+4*pos, color);
  } else {
    tft.drawLine(85, 27+2*pos, 75+abs(pos-3),21+4*pos, color);
  }

}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void speedoEnclosure() { 
  tft.fillRect(25, 78 , 24, 59, colSpeedoCase);
  tft.fillRect(27, 88 , 20, 35, colTcLabelsBlack); 
  tft.fillRect(28, 124, 5, 12, colTcLabelsBlack);
  tft.fillRect(34, 126, 5, 8, colTcLabelsBlack);
  tft.fillRect(40, 126, 5, 8, tft.color565(150,0,0)); 
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void tcEnclosure() { 
  byte w;
  byte x;
  byte y;

  // - case plus labels  
  for(byte i = 0 ; i < 3; i++) {
    w = 160;
    x = 24+i*27;
    y = 0;
    tft.fillRect(x, y , 26, w, colTcCase);
    
    tft.fillRect(x+2,   6, 2, 14, colTcLabelsRed); 
    tft.fillRect(x+2,  31, 2, 14, colTcLabelsRed); 
    tft.fillRect(x+2,  66, 2, 18, colTcLabelsRed); 
    tft.fillRect(x+2, 108, 2, 10, colTcLabelsRed); 
    tft.fillRect(x+2, 130, 2, 20, colTcLabelsRed); 
    
    tft.fillRect(x+21, 55, 4, 50, colTcLabelsBlack); 
    for(byte j = 0 ; j < 9; j++) {
      if (j!=3) tft.fillRect(x+22, 61+4.78*j , 2, 1, colTcLabelsFont); 
    } 
  }
  
  // - display openings
  for(byte i = 0 ; i < 3; i++) {
    x = 24+i*27+6;
    y= 125;
    tft.fillRect(x, y , 14, 30, colTcOpenings);
    y= 101;
    tft.fillRect(x, y , 14, 20, colTcOpenings);
    y= 56;
    tft.fillRect(x, y , 14, 40, colTcOpenings);
    y= 29;
    tft.fillRect(x, y , 14, 20, colTcOpenings);
    y= 4;
    tft.fillRect(x, y , 14, 20, colTcOpenings); 
    
    // - test: am/pm
    tft.fillCircle(x+5,52,1,colTcLeds[i]);
    // - labels: am/pm
    tft.fillRect(x+2, 50, 2, 5, colTcLabelsRed); 
    tft.fillRect(x+7, 50, 2, 5, colTcLabelsRed); 
  }
}

// -----------------------------------------------------------------------------
// operate TC
void tcOperate() {
  static unsigned long previousTime = 0;
  static byte oldDate[6];
  static byte newDate[6] = {10,26,19,85,1,21};
  
  if ((millis() - previousTime) > 11000) {
    previousTime = millis();

    // set current time to movie time
    tcMonth(10,1);
    tcNumber(26,4,1); 
    tcNumber(19,3,1);
    tcNumber(85,2,1);
    tcNumber(1,1,1);
    tcNumber(21,0,1);

    // copy new Date to old Date
    for (byte i=0; i<6; i++) {
      oldDate[i] = newDate[i]; 
    }
 
    // get new Date
    byte r = 1+random(noDates-1);
    newDate[0] = myDates[r][0]/100;
    newDate[1] = myDates[r][0]%100;
    newDate[2] = myDates[r][1]/100;
    newDate[3] = myDates[r][1]%100; 
    newDate[4] = myDates[r][2]/100;
    newDate[5] = myDates[r][2]%100;

    if (newDate[4] > 12) newDate[4] = newDate[4] - 12;
    else if (newDate[4] == 0) newDate[4] = 12; 

    // erase bottom
    tcMonth(13,2);
    for (byte i=0; i<5; i++) { 
      tcNumber(100,4-i,2); 
    }
    delay(450);
    
    // erase top & fill bottom
    tcMonth(13,0);
    for (byte i=0; i<5; i++) { 
      tcNumber(100,4-i,0); 
    }
    delay(100);
    tcMonth(oldDate[0],2);
    for (byte i=0; i<5; i++) {
      tcNumber(oldDate[i+1],4-i,2);  
    }
    
    // fill top
    delay(500);
    tcMonth(newDate[0],0);
    for (byte i=0; i<5; i++) { 
      tcNumber(newDate[i+1],4-i,0); 
    }
    
  }
}

// -----------------------------------------------------------------------------
// blink colons in TC
void tcBlink() {
  static unsigned long previousTime = 0;
  static byte state = 0;
   
  if ((millis() - previousTime) > 500) {
    previousTime = millis();
    for (byte row=0; row<3; row++) {
      byte x = 24+row*27+6;
      if (state == 0) {
        tft.fillCircle(x+5,26,1,colTcOpenings );
        tft.fillCircle(x+9,26,1,colTcOpenings );
      } else {
        tft.fillCircle(x+5,26,1,colTcLeds[row]);
        tft.fillCircle(x+9,26,1,colTcLeds[row]);     
      }
    }
    state = 1 - state;
  }
}

// -----------------------------------------------------------------------------
// draw a 2-dig number in a given color in a given row   
//    column: 0:min 1:hour 2:year-low 3:year-high 4:day
void tcNumber(byte number, byte col, byte row) {
  byte yoffset[5] = {5,30,57,77,102};      // offset in y
  byte digitLo = number%10;
  byte digitHi = number/10;

  // clear display
  tft.fillRect(24+row*27+7, yoffset[col] , 12, 18, tft.color565(0,0,0)); 
  // write
  if (digitHi < 10) {
    tcDigit(yoffset[col],   digitLo, row);
    tcDigit(yoffset[col]+10, digitHi, row);
  }
}

// -----------------------------------------------------------------------------
// draw a 3-dig month in a given row     
//    column: 0:min 1:hour 2:year-low 3:year-high 4:day
void tcMonth(byte month, byte row) {
  byte yoffset = 126;      // offset in y
 
  // clear display
  tft.fillRect(24+row*27+7, yoffset , 12, 29, tft.color565(0,0,0)); 
  // write
  if (month == 1) {
    tcDigit(yoffset+20, 10, row); 
    tcDigit(yoffset+10, 11, row);
    tcDigit(yoffset,    12, row); 
  } else if (month == 2) {
    tcDigit(yoffset+20, 13, row); 
    tcDigit(yoffset+10, 14, row);
    tcDigit(yoffset,    15, row); 
  } else if (month == 3) {
    tcDigit(yoffset+20, 16, row); 
    tcDigit(yoffset+10, 11, row);
    tcDigit(yoffset,    17, row); 
  } else if (month == 4) {
    tcDigit(yoffset+20, 11, row); 
    tcDigit(yoffset+10, 18, row);
    tcDigit(yoffset,    17, row); 
  } else if (month == 5) {
    tcDigit(yoffset+20, 16, row); 
    tcDigit(yoffset+10, 11, row);
    tcDigit(yoffset,    19, row); 
  } else if (month == 6) {
    tcDigit(yoffset+20, 10, row); 
    tcDigit(yoffset+10, 20, row);
    tcDigit(yoffset,    12, row); 
  } else if (month == 7) {
    tcDigit(yoffset+20, 10, row); 
    tcDigit(yoffset+10, 20, row);
    tcDigit(yoffset,    21, row); 
  } else if (month == 8) {
    tcDigit(yoffset+20, 11, row); 
    tcDigit(yoffset+10, 20, row);
    tcDigit(yoffset,    22, row); 
  } else if (month == 9) {
    tcDigit(yoffset+20, 5, row); 
    tcDigit(yoffset+10, 14, row);
    tcDigit(yoffset,    18, row); 
  } else if (month == 10) {
    tcDigit(yoffset+20, 0, row); 
    tcDigit(yoffset+10, 23, row);
    tcDigit(yoffset,    24, row); 
  } else if (month == 11) {
    tcDigit(yoffset+20, 12, row); 
    tcDigit(yoffset+10, 0, row);
    tcDigit(yoffset,    25, row); 
  } else if (month == 12) {
    tcDigit(yoffset+20, 0, row); 
    tcDigit(yoffset+10, 14, row);
    tcDigit(yoffset,    23, row); 
  } 
}

// -----------------------------------------------------------------------------
// draw a single digit in a given row   a-g -> 0-6
void tcDigit(byte y, byte digit, byte row) {
  if (digit == 0) {
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 3, row);
    tcSegment(y, 4, row);
    tcSegment(y, 5, row);
  } else if (digit == 1) {
    tcSegment(y, 1, row);
    tcSegment(y, 2, row);
  } else if (digit == 2) {
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 3, row);
    tcSegment(y, 4, row);
    tcSegment(y, 6, row);
  } else if (digit == 3) {
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 3, row);
    tcSegment(y, 6, row);
  } else if (digit == 4) {
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 5, row);
    tcSegment(y, 6, row);
  } else if (digit == 5) {
    tcSegment(y, 0, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 3, row);
    tcSegment(y, 5, row);
    tcSegment(y, 6, row);
  } else if (digit == 6) {
    tcSegment(y, 2, row); 
    tcSegment(y, 3, row);
    tcSegment(y, 4, row);
    tcSegment(y, 5, row);
    tcSegment(y, 6, row);
  } else if (digit == 7) {
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
  } else if (digit == 8) {
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 3, row);
    tcSegment(y, 4, row);
    tcSegment(y, 5, row);
    tcSegment(y, 6, row);
  } else if (digit == 9) {
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 5, row);
    tcSegment(y, 6, row);
  
  } else if (digit == 10) {          // J
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 3, row); 
  } else if (digit == 11) {          // A
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 6, row);
  } else if (digit == 12) {          // N
    tcSegment(y, 1, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 7, row); 
    tcSegment(y, 9, row);
  } else if (digit == 13) {          // F
    tcSegment(y, 0, row); 
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 6, row); 
  } else if (digit == 14) {          // E
    tcSegment(y, 0, row); 
    tcSegment(y, 3, row);  
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 6, row); 
  } else if (digit == 15) {          // B
    tcSegment(y, 0, row);
    tcSegment(y, 2, row); 
    tcSegment(y, 3, row);  
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 6, row); 
    tcSegment(y, 8, row); 
  } else if (digit == 16) {          // M
    tcSegment(y, 1, row);
    tcSegment(y, 2, row);   
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 7, row);  
    tcSegment(y, 8, row); 
  } else if (digit == 17) {          // R
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);   
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 6, row);  
    tcSegment(y, 9, row); 
  } else if (digit == 18) {          // P
    tcSegment(y, 0, row);
    tcSegment(y, 1, row);   
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 6, row);  
  } else if (digit == 19) {          // Y
    tcSegment(y, 7, row);
    tcSegment(y, 8, row); 
    tcSegment(y, 12, row);
  } else if (digit == 20) {          // U 
    tcSegment(y, 1, row); 
    tcSegment(y, 2, row);
    tcSegment(y, 3, row);     
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
  } else if (digit == 21) {          // L
    tcSegment(y, 3, row);     
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
  } else if (digit == 22) {          // G
    tcSegment(y, 0, row);
    tcSegment(y, 2, row);
    tcSegment(y, 3, row);     
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row); 
    tcSegment(y, 6, row);  
  } else if (digit == 23) {          // C
    tcSegment(y, 0, row);
    tcSegment(y, 3, row);     
    tcSegment(y, 4, row); 
    tcSegment(y, 5, row);   
  } else if (digit == 24) {          // T
    tcSegment(y, 0, row);
    tcSegment(y, 11, row); 
    tcSegment(y, 12, row);
  } else if (digit == 25) {          // V
    tcSegment(y, 4, row);
    tcSegment(y, 5, row); 
    tcSegment(y, 8, row);
    tcSegment(y, 10, row);     
  }   
}

// -----------------------------------------------------------------------------
// draw a single segment in a given row   a-g -> 0-6
// 
void tcSegment(byte y, byte segment, byte row) {
  uint16_t color = colTcDigits[row];
  byte hlength = 6;
  byte vlength = 5;
  byte width = 2; 
  
  byte x = 24+row*27+7;    // baseline

  // horizontal segments
  if (segment == 0) {                       // a  top horiz
    tft.fillRect(x, y+1 , width, hlength, color);  
  } else if (segment == 1) {                // b  right top
    tft.fillRect(x+1, y , vlength, width, color); 
  } else if (segment == 2) {                // c  right bot 
    tft.fillRect(x+6, y , vlength, width, color); 
  } else if (segment == 3) {                // d  bottom horiz
    tft.fillRect(x+10, y+1 , width, hlength, color);  
  } else if (segment == 4) {                // e  left bottom
    tft.fillRect(x+6, y+6 , vlength, width, color); 
  } else if (segment == 5) {                // f  left top
    tft.fillRect(x+1, y+6 , vlength, width, color); 
  } else if (segment == 6) {                // g  middle horiz
    tft.fillRect(x+5, y+1 , width, hlength, color);  
  
  // - add segments     
  } else if (segment == 7) {                //  top left diag
    tft.drawLine(x+1,y+6, x+5, y+4, color); 
    tft.drawLine(x+1,y+7, x+5, y+5, color);
  } else if (segment == 8) {                //  top right diag
    tft.drawLine(x+1,  y, x+5, y+2, color); 
    tft.drawLine(x+1,y+1, x+5, y+3, color);
  } else if (segment == 9) {                //  bot right diag
    tft.drawLine(x+6,y+3, x+10, y+1, color); 
    tft.drawLine(x+6,y+4, x+10, y+2, color);
  } else if (segment == 10) {               //  bot left diag
    tft.drawLine(x+6,y+3, x+10, y+6, color); 
    tft.drawLine(x+6,y+4, x+10, y+7, color);
  } else if (segment == 11) {               //  upper T 
    tft.fillRect(x+1, y+3 , vlength, width, color); 
  } else if (segment == 12) {               // lower T
    tft.fillRect(x+6, y+3 , vlength, width, color);
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void fluxCapEnclosure() {
  byte x = 24;
  byte y = 91;  
  byte w = 69;
  byte h = 80;
  byte th = 10;
  
  // enclosure
  //tft.fillRoundRect(x, y, h, w, 8, tft.color565(8,8,8));
  tft.fillRoundRect(x, y, h, w, 8, tft.color565(15,15,30));
  tft.fillRoundRect(x+th, y+th, h-2*th, w-2*th, 6, TFT_BLACK);
 
  // red labels
  tft.fillRect(x+3,y+w*0.5-21, 2, 43, tft.color565(140,0,0));
  tft.fillRect(x+6,y+w*0.5-13, 2, 27, tft.color565(140,0,0)); 
  //tft.fillRect(x+h*0.62,y+w*0.5-17, 2, 34, tft.color565(140,0,0)); 
  tft.fillRect(x+49,y+17, 2, 34, tft.color565(140,0,0)); //  x=24 y = 91

  const byte centx = 63;
  const byte centy = 125;
  
  //bases at i=6
  tft.fillCircle(centx+6*4,centy,5,colFluxBase);
  tft.fillCircle(centx-6*3,centy+6*3,5,colFluxBase);        //x=45  y=143
  tft.fillCircle(centx-6*3,centy-6*3,5,colFluxBase);        //x=45  y=107

  // red plugs
  byte plugLength = 9;
  byte plugWidth = 5;
  tft.fillRoundRect(85, 119 , plugWidth, plugLength, 2, tft.color565(140,0,0));
  tft.fillRoundRect(43, 141 , plugLength, plugWidth, 2, tft.color565(140,0,0)); 
  tft.fillRoundRect(43, 105 , plugLength, plugWidth, 2, tft.color565(140,0,0));
  tft.fillRect(85, 119 , plugWidth, plugLength-3, tft.color565(140,0,0)); 
  tft.fillRect(43+3, 141 , plugLength-3, plugWidth, tft.color565(140,0,0)); 
  tft.fillRect(43+3, 105 , plugLength-3, plugWidth, tft.color565(140,0,0)); 

  // yellow cables
  tft.fillRoundRect(86, 105 , 3, 14, 1, TFT_YELLOW); 
  tft.fillRoundRect(52, 142 , 14, 3, 1, TFT_YELLOW);
  tft.fillRoundRect(52, 106 , 14, 3, 1, TFT_YELLOW);
  tft.fillRect(86, 115 , 3, 4, TFT_YELLOW); 
  tft.fillRect(52, 142 , 4, 3, TFT_YELLOW);
  tft.fillRect(52, 106 , 4, 3, TFT_YELLOW);
}

// -----------------------------------------------------------------
// --- operateFlux      operate the Flux Capacitor
// -----------------------------------------------------------------
void operateFlux() {
  static unsigned long previousTime1 = 0;
  static unsigned long previousTime2 = 0;
  static unsigned interval1 = 5000;
  static unsigned interval2 = 50;
  static byte step = 0;
  
  if ((millis() - previousTime1) > interval1) {
    previousTime1 = millis();
    interval1 = 10000 + random(13000);
    interval2 = 20 + random(45);
    activity =  (65-interval2)/2;   // range: 0-22  used in SID approx speed/4
  }
    
  if ((millis() - previousTime2) > interval2) {
    previousTime2 = millis();
    step++;
    if (step > 10) step = 0;
    if (0 < step && step <5) setFluxLed(step,1);
    if (1 < step && step <6) setFluxLed(step-1,2);
    if (2 < step && step <7) setFluxLed(step-2,3);
    if (3 < step && step <8) setFluxLed(step-3,4);
    //if (step > 1 && step < 5) tft.fillRect(73,108, 2, 34, tft.color565(140,200,0));
    if (step < 6) tft.fillRect(73,125, 2, 1, tft.color565(140,0,0)); 
  }
  
}

// -------------------------------------------------------------
void setFluxLed(byte ledNo, byte state) {
  uint16_t color;
  const byte centx = 63;
  const byte centy = 125;
  byte i = 0;

  if (state == 1) color = tft.color565(255,190,130);         // warm white
  else if (state == 2) color = tft.color565(120,90,60);
  else if (state == 3) color = tft.color565(55,40,25);
  else if (state == 4) color = ST7735_BLACK;
  
  // set LED 
  i = 5-ledNo;
  tft.fillCircle(centx+i*4,centy,1,color);
  tft.fillCircle(centx-i*3,centy+i*3,1,color);
  tft.fillCircle(centx-i*3,centy-i*3,1,color); 
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void sidEnclosure() {
  byte x = 27;
  byte y = 15;   
  byte w1 = 38;
  byte w2 = 50;
  byte h = 71;
  
  tft.fillRect(x, y-(w2-w1)/2 , h, w2, colSidCase);
  tft.drawRect(x, y-(w2-w1)/2 , h, w2, colSidLine);
  tft.fillRect(x, y , h, w1, colSidCase);
  tft.drawRect(x, y , h, w1, colSidLine);
    
  // mounting holes
  tft.fillCircle(35, 12, 1, colSidLine);
  tft.fillCircle(88, 12, 1, colSidLine);
  tft.fillCircle(35, 55, 1, colSidLine);
  tft.fillCircle(88, 55, 1, colSidLine);

  // cable
  tft.fillRect(x+h+3, y-(w2-w1)/2+4, 3, w2+24, tft.color565(10,10,10));
  for (byte n = 0 ; n < 18; n++) { 
    tft.fillRect(x+h+2, y-(w2-w1)/2+6+4*n, 4, 2, tft.color565(30,30,30));
  }
  
  // cable holders
  tft.fillRect(x+h-4, y+2,    10, 5, tft.color565(128,0,128));
  tft.fillRect(x+h-4, y+w1-7, 10, 5, tft.color565(128,0,128));

  tft.drawRect(x+h-4, y+2,    10, 5, tft.color565(67,47,67));
  tft.drawRect(x+h-4, y+w1-7, 10, 5, tft.color565(67,47,67));
  
  tft.fillRect(x+h-2, y+4,    8, 1, tft.color565(18,18,18));
  tft.fillRect(x+h-2, y+w1-5, 8, 1, tft.color565(18,18,18));

  // LED lenses
  for (byte row=1; row<21; row++) {
    for (byte col=0; col<10; col++) {
      // turn off LEDs -> draw lenses
      plotSid(col, row, 0);
    }
  }  

  // - gas bottle
  tft.fillRect(32, 74, 7, 2, tft.color565(205,127,50));   // bronze pipe
  tft.fillRect(40, 74, 1, 2, tft.color565(60,50,50));
  tft.fillRect(41, 73, 5, 4, colGasBottle);
 
  tft.fillRect(46, 72, 2, 6, colGasBottle);
  tft.fillRect(46, 72, 2, 1, colGasBottle2);
  tft.fillRect(48, 71, 2, 8, colGasBottle);
  tft.fillRect(48, 71, 2, 2, colGasBottle2);
  
  tft.fillRect(50, 70, 2, 10, colGasBottle);
  tft.fillRect(50, 70, 2, 3, colGasBottle2);
  tft.fillRect(50, 70, 2, 1, colGasBottle3);
  tft.fillRect(50, 79, 2, 1, colGasBottle3);
 
  tft.fillRect(52, 69, 40, 12, colGasBottle);
  tft.fillRect(52, 69, 40, 4, colGasBottle2);
  tft.fillRect(52, 69, 40, 2, colGasBottle3);
  tft.fillRect(52, 79, 40, 2, colGasBottle3);
  tft.fillRect(52, 75, 40, 2, colGasBottle4);

  tft.fillRect(92, 70, 1, 10, colGasBottle); 
  tft.fillRect(92, 70, 1, 3, colGasBottle2);
  tft.fillRect(92, 70, 1, 1, colGasBottle3);
  tft.fillRect(92, 79, 1, 1, colGasBottle3); 

  // holder
  tft.fillRect(59, 68, 5, 14, tft.color565(3,3,3)); 
  tft.fillRect(61, 68, 1, 14, tft.color565(70,70,70)); 
  tft.fillRect(82, 68, 5, 14, tft.color565(3,3,3));
  tft.fillRect(84, 68, 1, 14, tft.color565(80,80,80)); 
}

// -----------------------------------------------------------------
// --- updateSid        update the SID display
//     peak[i]=0      i=0(no LED) -> i from 1-20
// -----------------------------------------------------------------
void updateSid() {
  for (byte col = 0 ; col < 10; col++) { 
    if (peak[col] != lastPeak[col]) {
      if (peak[col] > lastPeak[col]) {
      
        for (byte i = lastPeak[col]; i < peak[col]; i++) {
          // draw additional LEDs
          plotSid(col, i+1, 1);  
          if ((i+1) > 20) {
            Serial.print(i);  
            Serial.print("  ");
            Serial.println(peak[col]);
          }
        }
      } else if (peak[col] < lastPeak[col]) { 
        for (byte i = peak[col]; i <lastPeak[col]; i++) { 
         // turn off LEDs
         plotSid(col, i+1, 0);
        }
      }
      lastPeak[col] = peak[col];
    }
  }
}

// -----------------------------------------------------------------
// --- nextSid        compute next SID configuration - random walk
//                    the amount of action depends on the global 
//                    variable "activity" which is in the range 0-22
//                    (for a real SID, this would correspond to 1/4*speed)
// -----------------------------------------------------------------
void nextSid() {
  // state: 0: down  1:stop-down  2:stop-up  3:up
  //
  static unsigned long previousTime = 0;
  static unsigned interval = 3;
  static byte state[10];
  static byte col = 0;
  byte nbrL = 0;
  byte nbrR = 0;
   
  if ((millis() - previousTime) > interval) {
    previousTime = millis();

    // - find neighbors
    if (col > 0) {
      nbrL = col - 1;
    } else {
      nbrL = col + 1;
    }
    if (col < 9) {
      nbrR = col + 1;
    } else {
      nbrL = col - 1;
    }

    // - change state - when reached bottom/top - or when stopped
    if (state[col] == 0 && peak[col] ==  1) {               // when reached bottom - stop
      state[col] = 1;
    } else if (state[col] == 3 && peak[col] == 20) {        // when reached top - stop
      state[col] = 2;
    } else if (state[col] == 1 && random(100) < 20) {       // when stopped down - go up
      state[col] = 3;
    } else if (state[col] == 2 && random(100) < 20) {       // when stopped up - go down
     state[col] = 0;
     
    } else if (state[col] == 0 && peak[col] < peak[nbrR] && state[nbrR] == 3 && random(100) < 18) { // follow nbrR up
      state[col] = 1;
    } else if (state[col] == 0 && peak[col] < peak[nbrL] && state[nbrL] == 3 && random(100) < 18) { // follow nbrL up
      state[col] = 1;
    } else if (state[col] == 3 && peak[col] > peak[nbrR] && state[nbrR] == 0 && random(100) < 20) { // follow nbrR down
      state[col] = 1;
    } else if (state[col] == 3 && peak[col] > peak[nbrL] && state[nbrL] == 0 && random(100) < 20) { // follow nbrL down
      state[col] = 1;
      
    //} else if (state[col] == 3 && random(100) < 8) {       // small chance to change up direction
    } else if (state[col] == 3 && random(100) < (10-activity*0.2)) {       // small chance to change up direction
      state[col] = 2;
    //} else if (state[col] == 0 && random(100) < 4) {        // small chance to change down direction
    } else if (state[col] == 0 && random(100) < (2+activity*0.2)) {        // small chance to change down direction
      state[col] = 1;
    }

    //if (state[col] == 3 && peak[col] < 20 && random(100) < 30) {        // go up w/ prob  def:30
    if (state[col] == 3 && peak[col] < 20 && random(100) < (20+activity)) {        // go up w/ prob
      peak[col]++;
    //} else if (state[col] == 0 && peak[col] > 0 && random(100) < 35) {  // go down w/ prob.  def:35
    } else if (state[col] == 0 && peak[col] > 0 && random(100) < (45-activity)) {  // go down w/ prob.
      peak[col]--;
    }
          
    col++;
    if (col > 9) col = 0;  
       
  } 
  
}

 
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// --- plot or remove(val=0) circle in SID panel
void plotSid(byte x, byte y, byte val) {
  uint16_t color;
  if (val == 0) {
    color = colSidLenses;
  } else if (y < 14) { 
    color = tft.color565(0,170,0);  // darker green
  } else if (y < 20) {
    color = ST7735_YELLOW;
  } else {
    color = ST7735_RED;
  }
  tft.fillCircle(xConvert(y), yConvert(x), 1, color);
  if (val == 0) {
    tft.drawPixel(xConvert(y),yConvert(x), colSidLenses2);
  }
}


// -----------------------------------------------------------------------------
// --- convert SID coordinates to display coordinates
byte xConvert(byte x) {
   return 93-x*3;
}
byte yConvert(byte y) {
   return 47-y*3;
}

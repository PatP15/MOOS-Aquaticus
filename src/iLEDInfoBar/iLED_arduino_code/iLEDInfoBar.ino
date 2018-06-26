/**************************************************************/
/*    NAME: Caileigh Fitzgerald                               */
/*    ORGN: MIT                                               */
/*    FILE: iLEDInfoBar_arduino.cpp                           */
/*    DATE: May 5th 2018                                      */
/*                                                            */
/*  This needs to be compiled on the arduino if any changes   */
/*  are made. Otherwise the arduino doesn't need much from    */
/*  the programmer other than finding the correct Serial      */
/*  for comms. IF there is a change made use these commands   */
/*  to update the code on the Arduino.                        */
/*                                                            */
/*   $ sudo apt-get install gcc-avr avr-libc avrdude // linux */
/*   $  */
/************************************************************/
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "iLEDInfoBar_enums.h"
#define PIN 2
bool benchtest = false;
bool on = true;

// hex values for RGB colors
const uint32_t green  = 0x00FF00;
const uint32_t red    = 0xFF0000;
const uint32_t yellow = 0xE6FF00;
const uint32_t purple = 0x3500FF;
const uint32_t blue   = 0x0000FF;
const uint32_t blank  = 0x000000;

const int     OFFSET = 0; // shifting icons along the strip
const int     OFF_BLINK = 10;
const int     DEBUG = 8; // pin 4
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const int     interval = 90; // in milliseconds
const int     NUM_PIXELS = 5;
int           blinkingIcons[NUM_PIXELS]; // keeps track of icons that are blinking
unsigned int  t = 0;
unsigned int  mod = 0; // for determening on blink or off blink

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// * Forward declare for readability *
void changeState(int, int);
uint32_t getPixelColor(int);
//void blink(int);
int ctoi(char);
void allOn();
void allOff();
void setupColors();
void blink();
bool updateBlinkingList(int, int);
void add(int);
void debug();
void printElapsedTime(int); // format stop()
void start() {t=micros();}
int stop() {t=micros()-t; return(t);}
///////////////////////////////////
///           SETUP             ///
void setup()
{
  Serial.begin(9600);
  while (!Serial){;}
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  pinMode(DEBUG, OUTPUT);
  digitalWrite(DEBUG, HIGH);
  //debug();

  for (int i=0; i<NUM_PIXELS; i++) blinkingIcons[i]='!';
}

///////////////////////////////////
///           LOOP              ///
void loop()
{
  if (benchtest) start();
  if (Serial.available() > 0) 
  {
    digitalWrite(DEBUG, HIGH);
    
    char buffer[5];
    Serial.readBytes(buffer, 2);  // read 2 bytes from Serial port
    int type  = ctoi(buffer[0]);

    if (type == ALL_ON)  { allOn(); return; }   // for debugging
    if (type == ALL_OFF) { allOff(); return; }  // for debugging

    int state = ctoi(buffer[1]);
    strip.setBrightness(100);
    changeState(type, state);
  } else
  {
    digitalWrite(DEBUG, LOW);
  }

  //debug();
  blink();
  strip.show();
  
  if (benchtest) printElapsedTime(stop());
}

///////////////////////////////////
///     UTILITY FUNCTIONS       ///
void changeState(int type, int state)
{
  if (benchtest) start();
  if (state == m_OFF)
  {
    strip.setPixelColor(type+OFFSET,blank);
    updateBlinkingList(type, state); // remove from list if it's on it
  }
  else if (state == m_ACTIVE)
  {
    strip.setPixelColor(type+OFFSET,getPixelColor(type));
  }
  else if (state == m_BLINKING)
  {
    strip.setPixelColor(type+OFFSET,getPixelColor(type));
    updateBlinkingList(type, state); // this will call changeState()
  }
  else if (state == OFF_BLINK)
  {
    strip.setPixelColor(type+OFFSET,blank);
  }
  if (benchtest) printElapsedTime(stop());
}

uint32_t getPixelColor(int type)
{
  if (benchtest) start();
  switch (type) // set Icon colors here
  {
    case m_HAVE_FLAG:     if (benchtest) printElapsedTime(stop()); return (green);
    case m_FLAG_ZONE:     if (benchtest) printElapsedTime(stop()); return (green);
    case m_OUT_OF_BOUNDS:  return (red);
    case m_TAGGED:        if (benchtest) printElapsedTime(stop()); return (red);
    case m_IN_TAG_RANGE:  if (benchtest) printElapsedTime(stop()); return (yellow);
    case m_ROBOT_HEALTH:  if (benchtest) printElapsedTime(stop()); return (red);
    default:              return (blank);
  }
}

void debug()
{
  digitalWrite(DEBUG, HIGH);
  delay(100);
  digitalWrite(DEBUG, LOW);
  delay(300);
}

void blink()
{
  //debug();
  if (benchtest) start();
  currentMillis = millis();  // for blinking lights
  if (currentMillis - previousMillis >= interval) {
    // save the last time LED blinked
    previousMillis = currentMillis;
    for (int i=0;i<NUM_PIXELS;i++)
    {
      if (blinkingIcons[i]=='!') return;
      changeState(blinkingIcons[i], (on ? m_ACTIVE : OFF_BLINK));
    }
    
    debug();
    (on ? on=false : on=true); // swap on and off for blink
  }
  if (benchtest) printElapsedTime(stop());
}

void add(int type)
{ 
  for (int i=0;i<NUM_PIXELS;i++) 
  {
    if (blinkingIcons[i] == type)
    { // it's already in the list
      return;
    }
    else if (blinkingIcons[i]=='!')
    { // it isn't in the list
      blinkingIcons[i]=type;
      return;
    }
  } // end for loop
}

bool updateBlinkingList(int type, int state)
{
  if (benchtest) start();
  for (int i=0;i<NUM_PIXELS;i++)
    if (blinkingIcons[i] == type){  // if type in list
      if (benchtest) printElapsedTime(stop());
      if (state == m_OFF) blinkingIcons[i]='!'; // && new state off, remove from list
      return(true); // either is or was on list
    }
  // if exit loop then type not in list
  if (state != m_OFF) // if not in list && new state is not OFF, then add to list
    add(type);
  if (benchtest) printElapsedTime(stop());
  return(false);
}

void allOff()
{
  if (benchtest) start();
  for (unsigned int i=0; i<NUM_PIXELS; i++)
    strip.setPixelColor(i,blank);
  strip.show();
  if (benchtest) printElapsedTime(stop());
}

void allOn()
{
  if (benchtest) start();
  for (unsigned int i=0; i<NUM_PIXELS; i++)
    strip.setPixelColor(i,green);
  strip.show();
  if (benchtest) printElapsedTime(stop());
}

int ctoi(char ch) { return (ch - 48); }
void printElapsedTime(int t) { Serial.print(t); Serial.println("mu"); }



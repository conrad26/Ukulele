/*
 * Display_Input.cpp
 *
 * Created: 5/25/2014 5:53:44 PM
 *  Author: Tom
 */

#include "Arduino.h"
#include <avr/io.h>

//#line 1 "display_input8.ino"

#include <ByteBuffer.h>
//#include <ooPinChangeInt.h>
#include "Goldelox_Serial_4DLib.h"
#include "Goldelox_const4D.h"
#include <Adafruit_NeoPixel.h>
//#include <MsTimer2.h>
//#include <AdaEncoder.h>
#include <Rotary.h>

#define DEBUG
#ifdef DEBUG
void debug();
#endif

//#define LOG_MESSAGES
/*
#ifdef LOG_MESSAGES
#define HWLOGGING Serial
#else
#define HWLOGGING if (1) {} else Serial
#endif
*/

void setup();
void loop();
void setupMainScreen();
void mycallback(int ErrCode, unsigned char Errorbyte);
void flash();
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
void theaterChase(uint32_t c, uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
void displayChord(char* chord, uint8_t wait);
uint32_t Wheel(byte WheelPos);
ByteBuffer printBuffer(200);

//setup the rotary encoder
Rotary r = Rotary(2, 3);
unsigned char result;
int8_t clicks = 0;
char id = 0;
int encoderPos = 0;
int setButton = 8;

int ledSupplyEnablePin = 6;
int ledSupplyEnable = 0;

//setup the LED array
#define PIN 7 //this is the LED data pin
Adafruit_NeoPixel strip = Adafruit_NeoPixel(48, PIN, NEO_GRB + NEO_KHZ800);
int brightness = 120;  //default value, keep it divisible by 10, please

//setup the OLED display
#include <mySoftwareSerial.h>
mySoftwareSerial DisplaySerial(10, 11) ;
Goldelox_Serial_4DLib Display(&DisplaySerial);
int resetPin = 9;

//supply variables
int supplyOnDelay = 100;
int supplyOffDelay = 10;

//variables to support the menu structure
int inputDelay = 150;
int current_line = 0;
long intensity[3] = {0, 0, 0};
int menuPage = 0;
char* menu[] = {"Theater\n", "TheaterBow\n", "Message\n", "Rainbow\n", "Brightness\n", "Chord Library\n", "Off\n"};
int menuSize = 6;
int menuCounter = 0;
int lastReportedPos = 0;
int menuItem = 0;
char bufIntToChar[12];


//variables to support the sliders
int ySlider[3] = {8, 38, 68};
int sliderHeight = 15;
word color[] = {0xF800, 0x0400, 0x001F};
int start = 0;
long scale[3];
int scaling[3];
int colorIndex;
int sliderMode = 0;

//variables to support the metronome
int period;
int tempo = 60;

//variables for the chord library
//notes
char* note[] = {" A\n", "Bb\n", " B\n", " C\n", "Db\n", " D\n", "Eb\n", " E\n", " F\n", "Gb\n", " G\n", "Ab\n"};
//chord qualities
char* quality[] = {" maj\n", " min\n", "   7\n", "min7\n"," dim\n", " aug\n", "   6\n", "maj7\n", "   9\n"};
//chords
char* majorChords[] = {"2100\n", "3211\n", "4322\n", "0003\n", "1114\n", "2220\n", "3331\n", "4442\n", "2010\n", "3121\n", "0232\n", "5343\n"};
char* minorChords[] = {"2000\n", "3111\n", "4222\n", "0333\n", "1103\n", "2210\n", "3321\n", "0432\n", "1013\n", "2120\n", "0231\n", "1342\n"};
char* seventhChords[] = {"0100\n", "1211\n", "2322\n", "0001\n", "1112\n", "2223\n", "3334\n", "1202\n", "2310\n", "3424\n", "0212\n", "1323\n"};
char* minorSeventhChords[] ={"0433\n","1111\n","2222\n","3333\n","4444\n","2213\n","3324\n","0202\n","1313\n","2424\n","0211\n","0322\n"};
char* diminishedChords[] =  {"2323\n","0101\n","1212\n","2323\n","0101\n","1212\n","2323\n","0101\n","1212\n","2323\n","0101\n","1212\n"};
char* augmentedChords[]  =  {"2114\n","3221\n","4332\n","1003\n","2110\n","3221\n","2114\n","1003\n","2110\n","4322\n","4332\n","1003\n"};
char* sixthChords[] = {"2424\n","0211\n","1322\n","0000\n","1111\n","2222\n","3333\n","1020\n","2213\n","3324\n","0202\n","1313\n"};
//variables for the chord library
int noteIndex=0;
int qualityIndex=0;
int noteOrQuality=0;


//setting up to do character displays
void messageLoop();
//this should be a complete set of a 4 row x 5 column characters, in ascii order.  That is, the first is ascii 30 ("0") and so on through all the capital letters (ascii 5A or "Z") 
// index 0='0'; 1='1' ; 2='2' ; 3='3' ; 4='4' ; 5='5' ; 6='6' ; 7='7' ; 8='8' ; 9='9' ; 10=':' ; 11=';' ; 12='<' ; 13='=' ; 14='>'; 15='?'; 16='@'; 17='A' ; 
long asciiFont[43] = {0x069996,0x06E66F,0x06924F,0x0F171F,0x099F11,0x0F8E1E,0x0F8F9F,0x0F1111,0x0F9F9F,0x0F9F11,0x066066,0x066064,0x036863,0x00f0f0,0x0c616c,0x0f9202,0x0e9e87,0x069f99,0x0e9e9e,0x0f888f,0x0e999e,0x0f8f8f,0x0f8e88,0x068b96,0x099f99,0x0f666f,0x011196,0x09aca9,0x08888f,0x09ff99,0x09dfb9,0x069996,0x0f9f88,0x0f99bf,0x0f9fa9,0x0f8f1f,0x0f6666,0x09999F,0x099955,0x099ff9,0x099699,0x099f1E,0x0f36cf};
//so we want to create a 12 row wide shift register  each letter will be shifted in from the left (bottom) in order.  e.g. 1,2,3 would be 0x0F66E6,0x0F4296,0x0F171F,0.  As it stands there will at best be two characters on the screen at any one time.
//need to handle spaces in the message too.
String message="GO ORIOLES ";
int RedLED = 0x7F;  //red, green, blue 
int GreenLED = 0x30;
int BlueLED = 0x00;

//set up 9 x 2 byte deep buffer (each character is three bytes; display can output 12 nibbles, so can really only handle 6 bytes).  Add one byte to allow look ahead.
byte messageBuffer[] = {0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00};
String longString;
int bytePointer = 0;
int messagePointer = 0;
int outputPointer = 6; // start at the sixth byte and increment down by 6, looping as needed


//___________________________________________________________

void setup() {

  //OLED display reset
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, 0);
  delay(100);
  digitalWrite(resetPin, 1);

  //5V LED supply
  pinMode(ledSupplyEnablePin, OUTPUT);
  digitalWrite(ledSupplyEnablePin, 0);

  //output for metronome
  pinMode(12, OUTPUT);

  //select button on rotary encoder; enable the pull-up on pin
  pinMode(setButton, INPUT);
  digitalWrite(setButton, HIGH);

  //setup the main OLED display menu
  Display.TimeLimit4D   = 5000 ;
  delay (5000);  //time to allow display to initialize
  DisplaySerial.begin(9600) ;
  delay(500);  //time to allow serial port to initialize; not sure if this is needed
  Display.gfx_ScreenMode(LANDSCAPE);;
  Display.SSTimeout(0);
  Display.gfx_Cls();
  setupMainScreen();
  
  //hightlight the first item in the menu
  Display.txt_Inverse(1);
  Display.putstr(menu[0]) ;
  Display.txt_Inverse(0);
  Display.txt_MoveCursor(0, 0) ;

  //initialize the LED string
  strip.begin();
  strip.show();
}

void loop() {
  //menuPage = 2;

  result = r.process();  //the loop around this call must be tight or it cannot run through the state machine properly.  As such, it's better to only branch when there's a result.
  // delay(10);
  if (result) {
    if (result == DIR_CW) clicks += 1; //using an integer variable for later calculations
    if (result == DIR_CCW) clicks -= 1;
  }

  //menuPage 0 is the main menu; menuPage 1 is the chord library
  //clicks is used to determine if the encoder has been turned, NOT counting number of clicks, once the encoder position is modified, reset clicks to zero
  //highlight the current line; unhighlight the one above and below
  if (menuPage == 0) {
    if (start == 0) setupMainScreen();
    start = 1;
    //this section handles the main menu, it's designed to highlight the selected line and roll over if the end is reached either way.
    if (clicks > 0) {
      if (encoderPos == menuSize) {
        encoderPos = 0;  //roll around to the beginning line
        Display.txt_MoveCursor(menuSize, 0);
        Display.putstr(menu[menuSize]) ;
        Display.txt_MoveCursor(0, 0);
        Display.txt_Inverse(1);
        Display.putstr(menu[0]) ;
        clicks = 0;	//reset the clicks
      }
      else {
        encoderPos += 1; //we're going clockwise
        clicks = 0; //reset the clicks
        Display.txt_MoveCursor(encoderPos - 1, 0);
        Display.putstr(menu[encoderPos - 1]) ;
        Display.txt_MoveCursor(encoderPos, 0);
        Display.txt_Inverse(1);
        Display.putstr(menu[encoderPos]) ;
      }
    }
    if (clicks < 0) {
      if (encoderPos == 0) {
        encoderPos = menuSize;	//roll around to the beginning line
        clicks = 0;
        Display.txt_MoveCursor(0, 0);
        Display.putstr(menu[0]) ;
        Display.txt_MoveCursor(menuSize, 0);
        Display.txt_Inverse(1);
        Display.putstr(menu[menuSize]) ;
      }
      else {
        encoderPos -= 1;  //we're going counter-clockwise
        clicks = 0;
        Display.txt_MoveCursor(encoderPos + 1, 0);
        Display.putstr(menu[encoderPos + 1]) ;
        Display.txt_MoveCursor(encoderPos, 0);
        Display.txt_Inverse(1);
        Display.putstr(menu[encoderPos]) ;
      }
    }
    //handle the select click
    if (digitalRead(setButton) == LOW) {
      //Serial.println("Button Press - main");
      switch (encoderPos) {  //do an action based on the selected line
        case 0:
          theaterChase(50, 100);
          break;

        case 1:
          theaterChaseRainbow(50);
          break;

        case 2:
          delay(200);  //debounce the keypress
          while (digitalRead(setButton)) {  //keep looping until you get a button press
            messageLoop();
            delay(200); //this sets the speed of the loop
          }
          colorWipe(strip.Color(0, 0, 0), 0);
          menuPage = 0;
          break;

        case 3:
          rainbowCycle(50);
          break;

        case 4:
        //this page handles the overall brightness of the LEDs
          menuPage = 2;
          Display.gfx_Cls();
          Display.txt_Height(2);
          Display.txt_Width(2);
          Display.txt_MoveCursor(1, 0);
          Display.putstr("BRIGHTNESS\n");
          Display.txt_Height(1);
          Display.txt_Width(1);
          Display.txt_MoveCursor(12,2);
          Display.putstr("Click to accept");
          Display.txt_Height(3);
          Display.txt_Width(3);
          Display.txt_MoveCursor(2, 2);
          //Display.putstr("       \n");
          //Display.txt_MoveCursor(2,2);
          itoa(brightness, bufIntToChar, 10);
          Display.putstr(bufIntToChar);
          Display.txt_Height(1);
          Display.txt_Width(1);
          break;

        case 5:
        //this is the chord library
          menuPage = 1;
          Display.gfx_Cls();
          break;

        case 6:
        //turn off the LEDs
          colorWipe(strip.Color(0, 0, 0), 0);
          break;
      }
      delay(500);
      start = 0;
    }
  }

  //go to the second page; in this case it's the sliders for individual light control
  //sliders have two modes; in the first (sliderMode=0), you're selecting which slider to change; in the second (sliderMode=1) you're changing the slider value
  if (menuPage == 1) {
    if (start==0){
      start=1;
      Display.txt_Height(2);
      Display.txt_Width(2);
      Display.txt_MoveCursor(1,3);
      Display.putstr("Chord\n");
      //Display.txt_Height(3);
      //Display.txt_Width(3);
      Display.txt_MoveCursor(3,0);
      Display.txt_Inverse(1);
      Display.putstr(note[noteIndex]);
      Display.txt_MoveCursor(3,4);
      Display.putstr(quality[qualityIndex]);
      colorWipe(0,10);
      }
    if (clicks != 0) {  //this is actually very important so that the loop around the encoder can stay tight until it has a result
      //handle the display
      if (!noteOrQuality) { //0 = select note; 1= select quality
        if (clicks < 0 && noteIndex == 0) {
          noteIndex = 11;  //rollover
          clicks=0;
          Display.txt_MoveCursor(3, 0);
          Display.txt_Inverse(1);
          Display.putstr(note[noteIndex]);
        }
        else if (clicks > 0 && noteIndex == 11) {
          noteIndex = 0; //rollover
          clicks=0;
          Display.txt_MoveCursor(3, 0);
          Display.txt_Inverse(1);
          Display.putstr(note[noteIndex]);
        }
        else {
          noteIndex = noteIndex + clicks; //handle the display
          clicks=0;
          Display.txt_Inverse(1);
          Display.txt_MoveCursor(3, 0);
          Display.putstr(note[noteIndex]);
        }
      }
      else if (noteOrQuality) {
        if (clicks < 0 && qualityIndex == 0) {
          qualityIndex = 6;  //rollover; change max and min as new chords are added
          clicks=0;
          Display.txt_MoveCursor(3,4);
          Display.txt_Inverse(1);
          Display.putstr(quality[qualityIndex]);
        }
        else if (clicks > 0 && qualityIndex == 6) {
          qualityIndex = 0; //rollover
          clicks=0;
          Display.txt_MoveCursor(3,4);
          Display.txt_Inverse(1);
          Display.putstr(quality[qualityIndex]);
        }
        else {
          qualityIndex = qualityIndex + clicks;
          clicks=0;
          Display.txt_MoveCursor(3,4);
          Display.txt_Inverse(1);
          Display.putstr(quality[qualityIndex]);
        }
      }
    }
    if (digitalRead(setButton) == LOW) {
      if (!noteOrQuality) {
        Display.txt_MoveCursor(3, 0);
        Display.putstr(note[noteIndex]);
        Display.txt_MoveCursor(3,4);
        Display.txt_Inverse(1);
        Display.putstr(quality[qualityIndex]);  
        noteOrQuality=1;
        delay(500);
      }
      else if (noteOrQuality && qualityIndex ==0) {
        displayChord(majorChords[noteIndex], 10);
        start=0;
        noteOrQuality=0;
        menuPage=0;
        delay(500);
      }
      else if (noteOrQuality && qualityIndex ==1) {
        displayChord(minorChords[noteIndex], 10);
        start=0;
        noteOrQuality=0;
        menuPage=0;
        delay(500);
      }
      else if (noteOrQuality && qualityIndex ==2) {
        displayChord(seventhChords[noteIndex], 10);
        start=0;
        noteOrQuality=0;
        menuPage=0;
        delay(500);
      }
       else if (noteOrQuality && qualityIndex ==3) {
        displayChord(minorSeventhChords[noteIndex], 10);
        start=0;
        noteOrQuality=0;
        menuPage=0;
        delay(500);       
      } 
      else if (noteOrQuality && qualityIndex ==4) {
        displayChord(diminishedChords[noteIndex], 10);
        start=0;
        noteOrQuality=0;
        menuPage=0;
        delay(500);
      } 
      else if (noteOrQuality && qualityIndex ==5) {
        displayChord(augmentedChords[noteIndex], 10);
        start=0;
        noteOrQuality=0;
        menuPage=0;
        delay(500);
      } 
      else if (noteOrQuality && qualityIndex ==6) {
        displayChord(sixthChords[noteIndex], 10);
        start=0;
        noteOrQuality=0;
        menuPage=0;
        delay(500);
      }
    }
  }

  //menuPage 2 handles changing the global intensity of all LEDs
  //setup the display
  if (menuPage == 2) {
    //limit the value of brightness
    if (clicks != 0) {
      if (clicks < 0 && brightness <= 0) brightness = 0;
      else if (clicks > 0 && brightness >= 250) brightness = 250;
      else brightness = brightness + clicks * 10;
      Serial.print("brightness = ");
      Serial.println(brightness);
      Serial.print("clicks = ");
      Serial.println(clicks);
      Display.txt_Height(2);
      Display.txt_Width(2);
      Display.txt_MoveCursor(1, 0);
      Display.putstr("BRIGHTNESS\n");
      Display.txt_Height(1);
      Display.txt_Width(1);
      Display.txt_MoveCursor(12,2);
      Display.putstr("Click to accept");
      Display.txt_Height(3);
      Display.txt_Width(3);
      Display.txt_MoveCursor(2, 2);
      itoa(brightness, bufIntToChar, 10);
      if (brightness < 100) Display.putstr(" ");
      if (brightness < 10) Display.putstr(" ");
      Display.putstr(bufIntToChar);
      Display.txt_Height(1);
      Display.txt_Width(1);
      clicks = 0;
    }
    if (digitalRead(setButton) == LOW) {
      //accept value and exit out of menuPage 2
      strip.setBrightness(brightness);
      menuPage = 0;
    }
  }
}

void setupMainScreen() {
  Display.gfx_Cls();
  Display.txt_MoveCursor(0, 0);
  Display.txt_Height(2);
  Display.txt_Width(1);
  Display.txt_FGcolour(14);
  //Display.txt_Inverse(1);
  for (int j = 0; j < 7; j++) {
    Serial.println(encoderPos);
    if (j == encoderPos) {
      Display.txt_Inverse(1);
      Display.putstr(menu[j]);
    }
    else {
      Display.putstr(menu[j]) ;
    }
  }
  Display.txt_MoveCursor(encoderPos, 0) ;
  current_line = encoderPos;  //what is this for?
}

void debug() {
  Serial.print("menuPage=");
  Serial.println(menuPage);
  Serial.print("menuItem=");
  Serial.println(menuItem);
  Serial.print("start=");
  Serial.println(start);
}

void flash() {
  tone(12, 200, 10);
}

void messageLoop() {
  //so we're going to have a loop to load the buffer and a separate loop to shift it out
  //we'll need to write out the entire LED string on each update....wait. Oh!!! the string IS a shift register.  Can I just shift in one bit at a time and not worry about it at all??? (save this idea for later)

  //the first shift LEDs are 47,35,23,11, the second 46,34,22,10 and so on.
  //so for the number zero 0x0F999F, the F is shifted in first, 9 second and so on.
  //the algorithm is then (0x0F999F & 0xFF0000) >> 16, then (0x0F999F & 0x0FF00) >> 8, then (0x0F999F & 0x0000FF)

  //load the buffer if the output is at a character boundary
  int index;
  if (outputPointer == 17 || outputPointer == 5 || outputPointer == 11 ) {
    char displayChar = message.charAt(messagePointer);  //grab the first character of the message
    long codedChar = asciiFont[displayChar - 48];  //displayChar should be the character in ASCII, subtracting 48 should give the index in the asciiFont array.
    if (displayChar == 32) codedChar = 0x000000;
    // going to waste some memory space to make this easier to conceptualize.  Each nibble will get it's own memory location; this doubles the memory requirment, but it's not much to start with for the buffer 
    //the bytePointer is there to help conceptualize, but could be based on the outputPointer; just need to handle loop around
    messageBuffer[bytePointer+5]=byte((codedChar & 0xF00000) >> 20); //mask all but the last nibble and shift it over by 20 (and so on)
    messageBuffer[bytePointer+4]=byte((codedChar & 0x0F0000) >> 16); //this should put one nibble per memory location
    messageBuffer[bytePointer+3]=byte((codedChar & 0x00F000) >> 12); //all six represent on character
    messageBuffer[bytePointer+2]=byte((codedChar & 0x000F00) >> 8);
    messageBuffer[bytePointer+1]=byte((codedChar & 0x0000F0) >> 4);
    messageBuffer[bytePointer]  =byte((codedChar & 0x00000F));
    //
    if(bytePointer ==0) { //handle the loop around on the bytePointer
      bytePointer = 12;
    }
    else {
      bytePointer -= 6; //we are filling from the bottom up  ; NOTE: need to look at reversing this to see if it makes it easier
    }
    if (messagePointer == message.length()-1) {  //handle the loop around on the message
      messagePointer = 0;
    }
    else {
      messagePointer +=1; //move to the next character
    }
  }

  // loop to continually shift out the buffer
  // want to write out the entire strip on each pass through the loop, only the starting location changes
  
  for (int row=12;row > 0;row--) {
    index = outputPointer + (12-row);
    if (index > 17) index = outputPointer+(12-row)-18;  //loop if greater than 17
    for (int column=4; column > 0; column--) {     
      strip.setPixelColor(uint16_t(12*(column-1)+(row-1)),uint8_t(RedLED*(bitRead(messageBuffer[index], column-1))),uint8_t(GreenLED*(bitRead(messageBuffer[index], column-1))),uint8_t(BlueLED*(bitRead(messageBuffer[index], column-1))));  //at each location light up the LED if the bit is a one  
    }
  }
  //outputPointer points to the current lowest byte in the display string
  if (outputPointer == 0) outputPointer=17;
  else outputPointer -= 1; 
  strip.show();
}


void displayChord(char* chord, uint8_t wait) {
  //parse the chord into led positions; remember that first LED is index 0 in the string
  //ex. A = "2100", so light no pixels on the A string, nor on the E string, light pixel 0 on the C string and pixel 1 on the G string
  //important to note that 0 is off, not the first LED
  //so want to light up LEDs 24 (position = chord[3]-1,chord[2]+11,chord[1]+23,chord[0]+35
  //using above example, light up LEDs (a "0" substring is "off") 24 and 37
  for (int i = 0; i < 4; i++) {
    if (int(chord[i] - '0')) {
      int ledNumber = int(chord[i] - '0') + (3 - i) * 12 - 1; //see above discussion, the (3-i) is to reverse the index
      strip.setPixelColor(ledNumber, 0, 125, 125);
    }
  }
  strip.show();
  delay(wait);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;
  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void theaterChase(uint32_t c, uint8_t wait) {
  delay(200);  //debounce the keypress
  int j=0;
  do {
      for (int q = 0; q < 3; q++) {
        for (int i = 0; i < strip.numPixels(); i = i + 3) {
          strip.setPixelColor(i + q, c);
        }
        strip.show();

        delay(wait);

        for (int i = 0; i < strip.numPixels(); i = i + 3) {
          strip.setPixelColor(i + q, 0);
        }
      }
      j++;
    } while (digitalRead(setButton));  //keep looping until you get a button press
  }

void theaterChaseRainbow(uint8_t wait) {
  int j=0;
  do {
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255));
    m  }
      strip.show();
      delay(wait);
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);
      }
    }
    j++; 
  } while (digitalRead(setButton));  //keep looping until you get a button press
}

uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}




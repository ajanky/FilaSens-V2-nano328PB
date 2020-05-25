/*
 FilaSense V0.9
 (c) 2020 Andreas Jankowsky / 3BOTS 3D Engineering GmbH
 
 FUSE Values for ATMEGA328PB;
 FUSES: L=0xFF; H=0xDA; E=0xFC;

*/

#include "Wire.h"
#include "U8g2lib.h"
#include "MagAlpha.h"

const int RED  = 4;     // RED LED
const int BLUE = 5;     // BLUE LED
const int ZERO = 1;     // Index signal
const int OUT  = 0;     // Output signal
const int XDIR = 6;     // external DIR signal
const int ENCB = 7;     // alt encoder input
const int XSTP = 17;    // external STP signal

float lastmm  = 1;
int blink;
double angle = 0;
int count = 0;
float mm = 0;
float speed;
unsigned long time, lasttime;

#define SPI_SCLK_FREQUENCY  40000000      //SPI SCLK Clock frequency in Hz
#define SPI_CS              10            //SPI CS pin

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL, SDA);
MagAlpha magAlpha;

void setup() {

 pinMode(RED, OUTPUT);
 pinMode(BLUE, OUTPUT);
 pinMode(OUT, OUTPUT);
 pinMode(ZERO, INPUT);
 pinMode(SPI_CS, OUTPUT);
 pinMode(2, INPUT_PULLUP);  // IRQs
 pinMode(3, INPUT_PULLUP);  // IRQs
 pinMode(XDIR, INPUT_PULLUP);  // external DIR signal
 pinMode(ENCB, INPUT_PULLUP);  // alt encoder input
 pinMode(XSTP, INPUT_PULLUP);  // external STP signal



 digitalWrite(OUT, LOW);    // set OUT off

 magAlpha.begin(SPI_SCLK_FREQUENCY, MA_SPI_MODE_3, SPI_CS);  // Init Sensor
 magAlpha.writeRegister(4, 0b11111100);  // set ABZ to 100 PPT, ILIP 1111
 magAlpha.writeRegister(5, 0b00011000);  // set ABZ to 100 PPT, ILIP 1111


 test(); // show LEDs

  u8g2.begin();
  //  u8g2.setFlipMode(1);

  attachInterrupt(digitalPinToInterrupt(3), IRQ, RISING);  // ENCA on Pin 3 

  u8g2.firstPage();            // write logo and version
  do {
    hello();
  } while( u8g2.nextPage() );
 
  delay(2500);                 // for 2.5 s
  angle = magAlpha.readAngle();  
  u8g2.firstPage();            // write init screen
  do {
    draw();
  } while( u8g2.nextPage() );

  time = millis();
  lasttime = time;

  
}


void loop() {

  // angle = magAlpha.readAngle();      // read angle
  mm = count * 0.314;        // 10mm D = 31.4 mm per 100 count --> 1 count = 0.314mm

   if (mm != lastmm) { 
     time = (millis()-lasttime);
     speed = (mm - lastmm) / (time/1000.0);
     lastmm = mm;
     lasttime = millis();

     digitalWrite(RED, LOW);    // turn RED off

    u8g2.firstPage();
      do {
         draw();                          // show changed value
       } while( u8g2.nextPage() );
  }


if ((count / 10) != blink) {   // check for 3mm distance to blink

digitalWrite(BLUE, !digitalRead(BLUE));   // toggle BLUE for motion
digitalWrite(OUT, !digitalRead(OUT));   // toggle OUT for motion, one pulse per mm
blink = count / 10;
 }


if ( (millis() - lasttime) >5000) {      // hide old values

      digitalWrite(RED, HIGH);    // turn RED on
      u8g2.firstPage();
      do {
           if (lastmm > 999) {
            u8g2.drawStr( 0, 13, "length"); u8g2.setCursor(53, 13);  u8g2.println(lastmm/1000.0,2); u8g2.drawStr(110, 13, "m");}
            else { u8g2.drawStr( 0, 13, "length"); u8g2.setCursor(53, 13);  u8g2.drawStr( 0, 13, "length"); u8g2.setCursor(53, 13); u8g2.print(lastmm,0); u8g2.drawStr( 97, 13, "mm"); }
          u8g2.drawStr( 0, 30, "speed"); u8g2.drawStr( 53, 30, "---"); u8g2.drawStr( 97, 30, "mm/s");
        }  while( u8g2.nextPage() );
}
  delay(100);

}


void draw(void) {
  u8g2.setFont(u8g_font_unifont);
  
  u8g2.drawStr( 0, 30, "speed"); u8g2.setCursor(53, 30); u8g2.println(speed,1); u8g2.drawStr( 97, 30, "mm/s");
   if (lastmm > 999) {
     u8g2.drawStr( 0, 13, "length"); u8g2.setCursor(53, 13); u8g2.println(lastmm/1000.0,2); u8g2.drawStr(110, 13, "m");
   }
   else { u8g2.drawStr( 0, 13, "length"); u8g2.setCursor(53, 13); u8g2.print(lastmm,0); u8g2.drawStr( 97, 13, "mm"); }

}

void hello(void) {
  u8g2.setFont(u8g_font_unifont);
  u8g2.drawStr( 0, 14, "3BOTS  FilaSens");
  u8g2.drawStr( 0, 30, "V 0.99 - 250520");
}



void test(void) {   
  digitalWrite(RED, HIGH);    // turn RED on
    delay(500);
  digitalWrite(BLUE, HIGH);   // turn BLUE on
    delay(500);
   digitalWrite(RED, LOW);    // turn RED off
    delay(500);
  digitalWrite(BLUE, LOW);   // turn BLUE off
    delay(500);
}

void IRQ() {
if (digitalRead (ENCB))   // easy counting: electronic encoder is is bounce-free
     count++;
else count--; 
}
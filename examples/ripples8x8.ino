/* This example shows a more complex LED pattern
using a NeoTrellis board.
Note that due to memory requirements this example
will not work on boards with very limited memory such
as the Adafruit Metro (with ATMega328p)
*/

#include "Adafruit_NeoTrellis.h"
#define Y_DIM 8 //number of rows of keys
#define X_DIM 8 //number of columns of keys

//create a matrix of trellis panels
Adafruit_NeoTrellis t_array[Y_DIM/4][X_DIM/4] = {
  
  { Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F) },

  { Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x31) }};


//pass this matrix to the multitrellis object
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)t_array, Y_DIM/4, X_DIM/4);


#define MAX_RIPPLES 16

#define FALLOFF_TIME 45
#define FALLOFF (0xFF/FALLOFF_TIME)
#define NUM_POINTS 8

#define RIPPLE_RATE .5

#define INT_PIN 10

#define MATRIX_POINT(x,y) ((y)*8+(x))

uint32_t colors[] = {
  0xFF0000, 0x00FF00, 0x0000FF,
  0xFF00FF, 0x00FFFF, 0xFFFFFF
};

union color {
  struct {
    uint8_t blue:8;
    uint8_t green:8;
    uint8_t red:8;
  } bit;
  uint32_t reg;
};

color matrix[8][8];

struct point {
  float x;
  float y;
};

struct ripple {
  int8_t center;
  uint32_t t;
  color c;
  point points[NUM_POINTS];
};

static struct ripple ripples[MAX_RIPPLES];

//define a callback for key presses
TrellisCallback blink(keyEvent evt){

  for(int i=0; i<MAX_RIPPLES; i++){
    if(ripples[i].center == -1){
      //create a new ripple here
      ripples[i].center = evt.bit.NUM;
      Serial.println(evt.bit.NUM);
      ripples[i].t = 0;
      for(int j=0; j<NUM_POINTS; j++){
        ripples[i].points[j].x = ((evt.bit.NUM)%8);
        ripples[i].points[j].y = ((evt.bit.NUM)/8);
      }
      ripples[i].c.reg = colors[random(sizeof(colors)/sizeof(uint32_t))];
      
      break;
    }
  }
  return 0; 
}

void setup() {
  Serial.begin(9600);

  pinMode(INT_PIN, INPUT);
  randomSeed(analogRead(0));
  
  if(!trellis.begin()){
    Serial.println("could not start trellis");
    while(1) delay(1);
  }
  else{
    Serial.println("trellis started");
  }

  for(int i=0; i<MAX_RIPPLES; i++)
    ripples[i].center = -1;

  //activate all keys and set callbacks
  for(int i=0; i<64; i++){
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, blink);
  }
  
  //do a little animation to show we're on
  for(uint16_t i=0; i<64; i++) {
    trellis.setPixelColor(i, 0x0000FF);
    trellis.show();
    delay(50);
  }
  for(uint16_t i=0; i<64; i++) {
    trellis.setPixelColor(i, 0x000000);
    trellis.show();
    delay(50);
  }
}

void processRipples(){
  for(int x=0; x<8; x++){
    for(int y=0; y<8; y++)
      matrix[x][y].reg = 0;
  }

  bool update = false;
  for(int i=0; i<MAX_RIPPLES; i++){
    if(ripples[i].center > -1){
      update = true;
      
      //push all points out from the center
      point *p = ripples[i].points;

      p[0].x += RIPPLE_RATE;
      
      p[1].x += RIPPLE_RATE/2;
      p[1].y += RIPPLE_RATE/2;

      p[2].y += RIPPLE_RATE;

      p[3].x -= RIPPLE_RATE/2;
      p[3].y += RIPPLE_RATE/2;

      p[4].x -= RIPPLE_RATE;

      p[5].x -= RIPPLE_RATE/2;
      p[5].y -= RIPPLE_RATE/2;

      p[6].y -= RIPPLE_RATE;

      p[7].x += RIPPLE_RATE/2;
      p[7].y -= RIPPLE_RATE/2;

      for(int j=0; j<NUM_POINTS; j++){
        int x = round(p[j].x);
        int y = round(p[j].y);
        if(x < 8 && x >= 0 && y < 8 && y >= 0){
          byte red = min(255, matrix[x][y].bit.red + ripples[i].c.bit.red);
          byte green = min(255, matrix[x][y].bit.green + ripples[i].c.bit.green);
          byte blue = min(255, matrix[x][y].bit.blue + ripples[i].c.bit.blue);
          matrix[x][y].bit.red = red;
          matrix[x][y].bit.green = green;
          matrix[x][y].bit.blue = blue;
        }
      }
      

      ripples[i].t++;
      if(ripples[i].t >= FALLOFF_TIME) ripples[i].center = -1;
    }
  }

  if(update){
    for(int x=0; x<8; x++){
      for(int y=0; y<8; y++)
        trellis.setPixelColor(MATRIX_POINT(x,y), matrix[x][y].reg);
    }
    
    trellis.show();
  }
}

void loop() {
  if(!digitalRead(INT_PIN)){
    trellis.read();
  }
  processRipples();
  delay(20);
}

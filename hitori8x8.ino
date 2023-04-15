#include "Adafruit_NeoTrellis.h"

#define Y_DIM 8 //number of rows of key
#define X_DIM 8 //number of columns of keys

#define BRIGHTNESS 10

// Create a matrix of trellis panels
Adafruit_NeoTrellis trellis_array[Y_DIM/4][X_DIM/4] = {
  { Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F) },
  { Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x31) }
};

// Pass this matrix to the multitrellis object
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, Y_DIM/4, X_DIM/4);

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t wheel(byte wheel_pos) {
  if(wheel_pos < 85) {
    return seesaw_NeoPixel::Color(wheel_pos * 3, 255 - wheel_pos * 3, 0);
  } else if(wheel_pos < 170) {
    wheel_pos -= 85;
    return seesaw_NeoPixel::Color(255 - wheel_pos * 3, 0, wheel_pos * 3);
  } else {
    wheel_pos -= 170;
    return seesaw_NeoPixel::Color(0, wheel_pos * 3, 255 - wheel_pos * 3);
  }
  return 0;
}

int currentLED[X_DIM * Y_DIM];


void set_brightness(int level) {
  for (int x = 0; x < X_DIM / 4; x++) {
    for (int y = 0; y < Y_DIM / 4; y++) {
      trellis_array[y][x].pixels.setBrightness(level);
    }
  }
}

//solution board
//2 for black
//1 for white
int solboard[X_DIM][Y_DIM] = {
  { 2, 1, 1, 2, 1, 1, 1, 2 },
  { 1, 2, 1, 1, 2, 1, 1, 1 },
  { 1, 1, 1, 2, 1, 2, 1, 1 },
  { 1, 2, 1, 1, 1, 1, 1, 2 },
  { 1, 1, 1, 2, 1, 1, 2, 1 },
  { 2, 1, 2, 1, 2, 1, 1, 1 },
  { 1, 2, 1, 1, 1, 1, 2, 1 },
  { 1, 1, 1, 1, 2, 1, 1, 2 }
};

/*
int puzzleboard[X_DIM][Y_DIM] ={
  { 4, 4, 8, 4, 7, 5, 6, 2 },
  { 5, 5, 2, 8, 6, 4, 7, 6 },
  { 4, 5, 7, 1, 6, 1, 2, 3 },
  { 1, 2, 4, 5, 2, 6, 3, 6 },
  { 8, 6, 5, 6, 4, 7, 4, 2 },
  { 7, 8, 2, 6, 6, 1, 4, 5 },
  { 7, 3, 3, 1, 5, 2, 3, 4 },
  { 2, 7, 6, 4, 6, 3, 5, 3 }
};
 */

//starting board
int board[X_DIM][Y_DIM] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

int move_num = 0;

//color handler
int colorHandler(int value){
  if(value == 0){return 0x000000;}
  else if(value == 1){return 0x0000FF;}
  else if(value == 2){return 0xFF0000;}
  else{return 0;}
}

void draw_board(){
  for(int x = 0; x<8; x++){
    for(int y = 0; y<8; y++){
      trellis.setPixelColor(x, y, colorHandler(board[x][y]));
    }
  }
  trellis.show();
}

void win(){
  //win condition
  for(int i = 0; i<64; i++){
      trellis.setPixelColor(i, 0x00FF00);
      trellis.show();
      delay(50);
  } 
}

bool checkSol(){
  for(int i = 0; i<8; i++){
    for(int j = 0; j<8; j++){
      if (board[i][j] != solboard[i][j]){return 0;}
    }
  }
  return 1;
}

void printBoard(){
  for(int i = 0; i<8; i++){
    for(int j = 0; j<8; j++){
      Serial.print(board[i][j]);
    }
    Serial.println();
  }
}

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  
  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING){
    int x = evt.bit.NUM / 8;
    int y = evt.bit.NUM % 8;
    int b_val = board[x][y];
    board[x][y] = (b_val + 1)%3; //increment board, overflow
    trellis.setPixelColor(evt.bit.NUM, colorHandler(board[x][y])); //on rising

    if(checkSol()){
      win();
    }
    printBoard();
  }
  
  else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING){
    move_num ++;}
    
  trellis.show();
  return 0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  if(!trellis.begin()){
    Serial.println("Failed to begin trellis!");
    while(1);
  } else {
    Serial.println("Trellis started!");
  }

  set_brightness(BRIGHTNESS);

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

void loop(){
  trellis.read();
  delay(20);
}

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define GRIDHEIGHT 8
#define GRIDWIDTH 10

#define HEIGHT GRIDHEIGHT + 4
#define WIDTH GRIDWIDTH

#define NUMPIXELS 4*GRIDWIDTH*GRIDHEIGHT

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
bool grid[HEIGHT][WIDTH];
uint32_t gridColors[GRIDHEIGHT][GRIDWIDTH];

uint32_t blank = strip.Color(0, 0, 0);
uint32_t red = strip.Color(255, 0, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t green = strip.Color(0, 255, 0);
uint32_t cyan = strip.Color(0, 255, 255);
uint32_t purple = strip.Color(255, 0, 255);
uint32_t yellow = strip.Color(255, 255, 0);

struct posn{
  int row;
  int col;
};

struct piece{
  int width;
  uint32_t color;
  posn blocks[4];
};

piece tetris = {0, blank, {{0, 0}, {1, 1}, {2, 2}, {3, 3}}};
posn point = {0, 0};
bool falling = false;
int blocksFell = 0;
int lastPiece = 0;

void setup() {
 #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  strip.begin();
  strip.show();
  Serial.begin(9600);
  Reset();
  

  

  randomSeed(analogRead(0));
  
}

void loop() {
  if(falling){
    if(CanMovePieceDown(&tetris)){
      UpdatePiece();
      blocksFell++;
    } else {
      falling = false;
    }
  } else if(blocksFell != 0){
    InitializePiece();
    falling = true;
    blocksFell = 0;
  } else {
    FlashGrid(3);
    Reset();
  }
  
  strip.show();
  delay(1000);
}

void ClearGrid(){
  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
      grid[i][j] = false;
    }
  }
}

void Reset(){
  ClearGrid();
  SetGrid(blank);
  falling = true;
  blocksFell = 0;
}

void SetBlock(int16_t row, int16_t col, uint32_t c) {
    int pos =0;
    int offset = row*2*20;
    pos=col*2; 
    strip.setPixelColor(offset+pos, c);
    strip.setPixelColor(offset+pos+1, c);
    strip.setPixelColor(offset+39-pos, c);
    strip.setPixelColor(offset+39-(pos+1), c);
    //delay(1);
}

void SetRow(int16_t row, uint32_t c){
  for(int i = 0; i < GRIDWIDTH; i++){
    SetBlock(row, i, c);
  }
}

void SetRowFromGrid(int16_t row){
  for(int i = 0; i < GRIDWIDTH; i++){
    SetBlock(row, i, gridColors[row][i]);
  }
}

void SetGrid(uint32_t c){
  for(int i = 0; i < GRIDHEIGHT; i++){
    SetRow(i, c);
  }
}

void SetGrid(){
  for(int i = 0; i < GRIDHEIGHT; i++){
    SetRowFromGrid(i);
  }
}

void StoreBlock(int16_t row, int16_t col){
  gridColors[row][col] = strip.getPixelColor(row*2*20 + col*2);
}

void StoreRow(int16_t row){
  for(int i = 0; i < GRIDWIDTH; i++){
    StoreBlock(row, i);
  }
}

void StoreGrid(){
  for(int i = 0; i < GRIDHEIGHT; i++){
    StoreRow(i);
  }
}

void FlashRow(int16_t row, uint8_t times){
  StoreRow(row);
  for(int i = 0; i < times; i++){
    SetRow(row, blank);
    strip.show();
    delay(500);
    SetRowFromGrid(row);
    strip.show();
    delay(500);
  }
}

void FlashGrid(uint8_t times){
  StoreGrid();
  for(int i = 0; i < times; i++){
    SetGrid(blank);
    strip.show();
    delay(500);
    SetGrid();
    strip.show();
    delay(500);
  }
}

void Clear(){
  for(int i = 0; i < NUMPIXELS; i++){
    strip.setPixelColor(i, blank);
  }
}

void CreatePosn(posn * p, byte r, byte c){
  p->row = r;
  p->col = c;
}

void MovePosn(posn * p, int deltaRow, int deltaCol){
  p->row = p->row + deltaRow;
  p->col = p->col + deltaCol;
}

void CopyPosn(posn * p, posn * q){
  q->row = p->row;
  q->col = p->col;
}

void MoveandCopyPosn(posn * p, posn * q, int deltaRow, int deltaCol){
  CopyPosn(p, q);
  MovePosn(q, deltaRow, deltaCol);
}

bool PosnEqual(posn * p, posn * q){
  return p->row == q->row && p->col == q->col;
}

void DisplayPosn(posn * p){
  Serial.print("Row: ");
  Serial.print(p->row);
  Serial.print(" Col: ");
  Serial.print(p->col);
  Serial.print("\n");
}

void DisplayPiece(piece * p){
  Serial.println("Piece:");
  for(int i = 0; i < 4; i++){
    Serial.print("\t");
    DisplayPosn(&p->blocks[i]);
  }
}

bool PosnArrayContains(posn * p, posn * q){
  bool result = false;
  for(int i = 0; i < 4; i++){
    result = result || PosnEqual(&p[i], q);
  }
  return result;
}

bool CanMovePieceDown(piece * p){
  bool result = true;
  for(int i = 0; i < 4; i++){
    MoveandCopyPosn(&p->blocks[i], &point, 1, 0);
    if(point.row >= HEIGHT){
      result = false;
    } else {
      if(!PosnArrayContains(p->blocks, &point) && grid[point.row][point.col] != blank){
        result = false;
      }
    }
  }
  return result;
}


void MovePieceDown(piece * p){
  for(int i = 0; i < 4; i++){
    MovePosn(&p->blocks[i], 1, 0);
  }
}

void UpdateGrid(piece * p, bool b){
  for(int i = 0; i < 4; i++){
    grid[p->blocks[i].row][p->blocks[i].col] = b;
  }
}

void DrawPiece(piece * p, uint32_t color){
  for(int i = 0; i < 4; i++){
    if(p->blocks[i].row - 4>= 0){
      SetBlock(p->blocks[i].row - 4, p->blocks[i].col, color);
    }
  }
}

void CreateRandomPiece(piece * p, posn * q){
  int newPiece = random(0, 4);
  while(newPiece == lastPiece){
    newPiece = random(0, 4);
  }
  lastPiece = newPiece;
  switch(lastPiece){
    case 0: {
      CreateI(p, q);
      break;
    }
    case 1: {
      CreateO(p, q);
      break;
    }
    case 2: {
      CreateT(p, q);
      break;
    }
    case 3: {
      CreateS(p, q);
      break;
    }
  }
}

void UpdatePiece(){
  UpdateGrid(&tetris, false);
  DrawPiece(&tetris, blank);
  MovePieceDown(&tetris);
  UpdateGrid(&tetris, true);
  DrawPiece(&tetris, tetris.color);
}

void InitializePiece(){
  CreatePosn(&point, 3, random(0, WIDTH - tetris.width));
  CreateRandomPiece(&tetris, &point);
  UpdateGrid(&tetris, true);
  DrawPiece(&tetris, tetris.color);
}

void CreateI(piece * p, posn * q){
  p->width = 1;
  p->color = cyan;
  MoveandCopyPosn(q, &p->blocks[0], 0, 0);
  MoveandCopyPosn(q, &p->blocks[1], -1, 0);
  MoveandCopyPosn(q, &p->blocks[2], -2, 0);
  MoveandCopyPosn(q, &p->blocks[3], -3, 0);
}


void CreateO(piece * p, posn * q){
  p->width = 2;
  p->color = yellow;
  MoveandCopyPosn(q, &p->blocks[0], 0, 0);
  MoveandCopyPosn(q, &p->blocks[1], -1, 0);
  MoveandCopyPosn(q, &p->blocks[2], 0, 1);
  MoveandCopyPosn(q, &p->blocks[3], -1, 1);
}

void CreateT(piece * p, posn * q){
  p->width = 3;
  p->color = purple;
  MoveandCopyPosn(q, &p->blocks[0], 0, 0);
  MoveandCopyPosn(q, &p->blocks[1], 0, 1);
  MoveandCopyPosn(q, &p->blocks[2], 0, 2);
  MoveandCopyPosn(q, &p->blocks[3], -1, 1);
}

piece CreateS(piece * p, posn * q){
  p->width = 3;
  p->color = green;
  MoveandCopyPosn(q, &p->blocks[0], 0, 0);
  MoveandCopyPosn(q, &p->blocks[1], 0, 1);
  MoveandCopyPosn(q, &p->blocks[2], -1, 1);
  MoveandCopyPosn(q, &p->blocks[3], -1, 2);
}


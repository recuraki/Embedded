#include <M5Stack.h>
#include <driver/dac.h> //Arduino-ESP32 driver
#include <driver/rtc_io.h>
#include <esp_err.h>
#include <driver/ledc.h>

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 320
#define HEIGHT 240

#define MAX_CURRENT_COUNTER 20
#define MAX_CURRENT_STOP_COUNTER 512
#define COLOR_BACK_GROUND BLACK

// Block Size
int cellsizes[] = {16, 10, 4, 3};
int numCellSize;
int cellsize;

int delayTimes[] = {200, 100, 50, 33, 3, 0};
int numDelayTimes;
int delayTime;

int colors[] = {BLUE, WHITE, RED, YELLOW, GREEN, ORANGE, PURPLE, DARKCYAN, CYAN, BLACK};
int numColors;
int color;
int colorBackGround;
int colorCellLine;

int config_cellsize = 0;
int config_color = 0;
int config_colorCellLine = 0;
int config_colorBackGround = 0;
int config_delayTime = 2;
int marginCell = 1;
int current_stop_counter;
int current_counter;
int current;

// include waku
int cellHeight;
int cellWidth;

int worldHeight;
int worldWidth;

int world[2][ (HEIGHT/3) * (WIDTH/3)]; 
int worldstate = 0;

int dx[] = {-1,  0,  1, -1,  1, -1,  0,  1};
int dy[] = {-1, -1, -1,  0,  0,  1,  1,  1};
int numDirection = 8;

#define XY2INDEX(x, y) (y * worldHeight + x)

void updateWorld();

void initWorld() {
  int i;
  int xx, yy;

  M5.Lcd.fillScreen(colorBackGround);
  current_stop_counter = 0;
  current_counter = 0;

  for(i = 0; i < worldWidth * worldHeight; i++){
    world[worldstate][i] = (int)random(2);
  }

  for(yy = 1; yy < worldHeight + 1; yy++){
    M5.Lcd.drawLine(0, yy * cellsize, WIDTH, yy * cellsize, colorCellLine);
  }
  for(xx = 1; xx < worldWidth + 1; xx++){
    M5.Lcd.drawLine(cellsize * xx, 0, cellsize * xx, HEIGHT, colorCellLine);
  }
  updateWorld();
}

int nextWorld() {
  int isChange = 0;
  int yy, xx;
  int nx, ny;
  int dir;
  int cntAliveCell;

  for(yy = 0; yy < worldHeight; yy++){
    for(xx = 0; xx < worldWidth; xx++){
      cntAliveCell = 0;
      for(dir = 0; dir < numDirection; dir++){
        nx = xx + dx[dir];
        ny = yy + dy[dir];
        if(nx < 0) { nx = worldWidth - 1;}
        else if (nx >= worldWidth) {nx = 0;}
        if(ny < 0) { ny = worldHeight - 1;}
        else if (ny >= worldHeight) {ny = 0;}

        cntAliveCell += world[worldstate][XY2INDEX(nx, ny)];
      } /* dir */

      if(world[worldstate][XY2INDEX(xx, yy)] == 1){ // is Alive?
        if(cntAliveCell == 2 || cntAliveCell == 3){ // Alive
          world[1 - worldstate][XY2INDEX(xx, yy)] = 1;
        } else { // Death
          world[1 - worldstate][XY2INDEX(xx, yy)] = 0;
          isChange = 1;
        }
      } else { // is Death?
        if(cntAliveCell == 3) { // Alive
          world[1 - worldstate][XY2INDEX(xx, yy)] = 1;
          isChange = 1;
        } else {
          world[1 - worldstate][XY2INDEX(xx, yy)] = 0;
        }
      }

    } /* ww */
  } /* hh */

  if(isChange == 0) {
    current_counter += 1;
    if(current_counter > MAX_CURRENT_COUNTER) {
      initWorld();
      return(0);
    }
  }

  current_stop_counter += 1;
  if(current_stop_counter > MAX_CURRENT_STOP_COUNTER) {
    initWorld();
    return(0);
  }

  worldstate = 1 - worldstate;

  return(0);
}

void updateWorld() {
  int xx, yy;
  for(yy = 0; yy < worldHeight; yy++){
    for(xx = 0; xx < worldWidth; xx++){
      if(world[worldstate][XY2INDEX(xx, yy)] == world[1- worldstate][XY2INDEX(xx, yy)] ) continue;
      if(world[worldstate][XY2INDEX(xx, yy)] == 1) {
        M5.Lcd.fillRect(xx * cellsize + marginCell, yy * cellsize + marginCell,
         cellsize - marginCell, cellsize - marginCell, colors[config_color]);
      } else {
        M5.Lcd.fillRect(xx * cellsize + marginCell, yy * cellsize + marginCell,
         cellsize - marginCell, cellsize - marginCell, colorBackGround);
      }
    }
  }
}

void colorChange() {
  config_color += 1;
  if (config_color >= numColors) config_color  = 0;
  color = colors[config_color];
  initWorld();
}

void colorLineChange() {
  config_colorCellLine += 1;
  if (config_colorCellLine >= numColors) config_colorCellLine  = 0;
  colorCellLine = colors[config_colorCellLine];
  initWorld();
}

void colorBGChange() {
  config_colorBackGround += 1;
  if (config_colorBackGround >= numColors) config_colorBackGround  = 0;
  colorBackGround = colors[config_colorBackGround];
  initWorld();
}


void blocksizeChange() {
  config_cellsize += 1;
  if (config_cellsize >= numCellSize) config_cellsize = 0;
  cellsize = cellsizes[config_cellsize];
  worldWidth = WIDTH / cellsize;
  worldHeight = HEIGHT / cellsize;
  initWorld();
}

void delayTimeChange() {
  config_delayTime += 1;
  if (config_delayTime >= numDelayTimes) config_delayTime = 0;
  delayTime = delayTimes[config_delayTime];
}

void setup() {
  M5.begin();
  numCellSize = sizeof(cellsizes) / sizeof(cellsizes[0]);
  numColors = sizeof(colors) / sizeof(colors[0]);
  numDelayTimes = sizeof(delayTimes) / sizeof(delayTimes[0]);

  config_cellsize = 0;
  config_color = 0;
  config_colorCellLine = 3;
  config_colorBackGround = numColors - 2;
  
  current = 0;
  colorBackGround = COLOR_BACK_GROUND;
 
  colorChange();
  colorBGChange();
  colorLineChange();
  blocksizeChange();
  delayTimeChange();
  initWorld();
}

void loop() {
  if (M5.BtnA.wasPressed()) {
    delay(200);
    M5.update();
    if (M5.BtnB.wasPressed()){
      delay(200);
      M5.update();
      if (M5.BtnC.wasPressed()){ // A+B+C
      } else { // A+B
        delayTimeChange();
      } 

    } else {
      if (M5.BtnC.wasPressed()){ // A+C
        colorBGChange();
      } else { // A
        colorChange();
      } 
    }
  }
  else if (M5.BtnB.wasPressed()) {
      delay(200);
      M5.update();
      if (M5.BtnC.wasPressed()){ // B+C
        colorLineChange();
      } else { // B
        blocksizeChange();
      } 
  }
  else if (M5.BtnC.wasPressed()) {
    initWorld();
  }
  nextWorld();
  updateWorld();
  delay(delayTime);
  M5.update();
}


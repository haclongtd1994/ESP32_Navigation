#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include "stdint.h"


void DrawBottomMessage(const char* msg, uint16_t color);
void DrawMessage(const char* msg, int xStart, int yStart, int scale, bool overwrite, uint16_t color);
void DrawDirection(uint8_t direction);
void DrawSpeed(uint8_t speed);
const uint8_t* ImageFromDirection(uint8_t direction);
void RedrawFromCanvas();
void DrawImageProgmem(int xStart, int yStart, int width, int height, const uint16_t* pBmp);
uint16_t Color4To16bit(uint16_t color4bit);
void Draw4bitImageProgmem(int x, int y, int width, int height, const uint8_t* pBmp);
void SetPixelCanvas(int16_t x, int16_t y, uint16_t value);
void SetPixelCanvasIfNot0(int16_t x, int16_t y, uint16_t value);
void DrawColumn8(int16_t x, int16_t y, uint8_t columnData, int scale, bool overwrite, uint16_t color);
void FillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color);

#endif
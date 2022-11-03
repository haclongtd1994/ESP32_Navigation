#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include "stdint.h"

/* OLED function support */
void DrawDirection(uint8_t direction);
void DrawSpeed(uint8_t speed);
const uint8_t* ImageFromDirection(uint8_t direction);
void DrawLogo(void);

/* Wakeup function */
void print_wakeup_reason(void);


#endif
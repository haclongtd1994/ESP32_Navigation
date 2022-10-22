/*
 * 	Display.h
 *
 *	Version: 1.0.0
 *  Created on: Dec 14, 2018
 *  Author: Julian Stiefel
 *  License: BSD 3-Clause
 *  Description: Class providing all the display outputs.
 */

#ifndef SRC_DISPLAY_H_
#define SRC_DISPLAY_H_

//for a connecting display via I2C
#include <Wire.h>
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

class Display
{
public:
	//Constructor:
	Display(SSD1306Wire* oled_);

	//Destructor:
	virtual ~Display();

	int counter;

	void init();
	void drawWelcomeScreen();
    void clearscreen();
    void displayscreen();
	void drawConnectionScreen(const uint8_t* direction);
	void drawDirection(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t* direction);
    void drawMessage(int16_t x, int16_t y, uint16_t maxLineWidth, const char *text, const uint8_t *font, OLEDDISPLAY_TEXT_ALIGNMENT align);
	void displayOn();
	void displayOff();

private:
	SSD1306Wire* oled;

};

#endif /* SRC_DISPLAY_H_ */
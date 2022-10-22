/*
 * 	Display.cpp
 *
 *	Version: 1.0.0
 *  Created on: Dec 14, 2018
 *  Author: Julian Stiefel
 *  License: BSD 3-Clause
 *  Description: Class providing all the display outputs.
 */

#include "Display.h"

Display::Display(SSD1306Wire* oled_)
	: counter(0), oled(oled_) 	//initialization list
{

}

Display::~Display()
{

}

void Display::init()
{
	//initialize OLED display
	oled->init();
	oled->flipScreenVertically();
	oled->setFont(ArialMT_Plain_10);
}

void Display::drawWelcomeScreen()
{
	// clear the display
	oled->clear();
	oled->displayOn();
	oled->setTextAlignment(TEXT_ALIGN_CENTER);
	oled->setFont(ArialMT_Plain_24);
	oled->drawStringMaxWidth(64,10,128, "Welcome");
	oled->setTextAlignment(TEXT_ALIGN_RIGHT);
	oled->setFont(ArialMT_Plain_10);
	oled->drawStringMaxWidth(128,50,128, "Haclongtd1994 Navigation Project");
	oled->display();
}

void Display::clearscreen()
{
	// clear the display
	oled->clear();
}

void Display::displayscreen()
{
	// display to canvas
	oled->display();
}

void Display::drawConnectionScreen(const uint8_t* direction)
{
	// clear the display
	oled->clear();
	oled->setTextAlignment(TEXT_ALIGN_RIGHT);
	oled->setFont(ArialMT_Plain_16);
	oled->drawStringMaxWidth(128,5,128, "60Km");

	oled->setTextAlignment(TEXT_ALIGN_RIGHT);
	oled->setFont(ArialMT_Plain_16);
	oled->drawStringMaxWidth(128,40,128, "1000m");


	oled->drawXbm(0, 0, 64, 64, direction);
	//int progress = (counter / 5) % 100;
	// int progress = ((counter % 5)+1) * 20;
	// draw the progress bar
	// oled->drawProgressBar(0, 50, 120, 10, progress);

	oled->display();
}

void Display::drawDirection(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t* direction)
{
	oled->drawXbm(x, y, width, height, direction);
}

void Display::drawMessage(int16_t x, int16_t y, uint16_t maxLineWidth, const char *text, const uint8_t *font, OLEDDISPLAY_TEXT_ALIGNMENT align)
{
	oled->setTextAlignment(align);
	oled->setFont(font);
    String s = "";
    for (int i = 0; i < strlen(text); i++) {
        if (text[i] != '\n')
            s = s + text[i];
        else
            break;
    }
	oled->drawStringMaxWidth(x, y, maxLineWidth, (const String) s);
}

void Display::displayOn()
{
	oled->displayOn();
}

void Display::displayOff()
{
	oled->displayOff();
}
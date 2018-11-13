// **********************************************************************************
// OLED display management header file for remora project
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : 22/01/2015 : First release
//           15/09/2015 Charles-Henri Hallard : Ajout compatibilité ESP8266
//           12/08/2017 Manuel Hervo: Changement de library Adafruit => OLEDDisplay
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "remora.h"

#define I2C_DISPLAY_ADDRESS 0x3C
#define SDA_PIN  4
#define SDC_PIN  5

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// Variables exported for other source file
// ========================================
extern OLEDDisplay *display;
extern OLEDDisplayUi *ui;
extern FrameCallback frames[];

// Number frames to display
#define DISPLAY_FRAME_COUNT 3
// Frame RF is activated if MOD_RF69 is defined
#ifdef MOD_RF69
  #define DISPLAY_FRAME_COUNT 4
#endif
#define DISPLAY_FPS 50 // Time to display a frame


// Function exported for other source file
// =======================================
bool initDisplay(void);
void initDisplayUI(void);
void updateData(OLEDDisplay *display) ;
void drawProgress(OLEDDisplay *display, int percentage, String labeltop, String labelbot);
void drawProgress(OLEDDisplay *display, int percentage, String labeltop);
void drawProgressBarVert(OLEDDisplay *display, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);
void drawFrameWifi(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrameTinfo(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrameLogo(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrameRF(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void setReadyForUpdate();

void doDisplay(void * extented = NULL);
void displaySplash(bool _config_ok);
void displayCommand(void);
void displayTest(void);

#endif

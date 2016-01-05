// **********************************************************************************
// Remora WEB Client include source code
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// This program works with the Remora board
// see schematic here https://github.com/thibdct/programmateur-fil-pilote-wifi
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History 2016-01-04 - Creation
//
// All text above must be included in any redistribution.
//
// **********************************************************************************


#ifndef WEBCLIENT_H
#define WEBCLIENT_H

// Include main project include file
#include "remora.h"

// Exported variables/object instancied in main sketch
// ===================================================

// declared exported function from route.cpp
// ===================================================
boolean httpPost(char * host, uint16_t port, char * url);
boolean emoncmsPost(void);
boolean jeedomPost(void);

#endif

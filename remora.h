// **********************************************************************************
// Programmateur Fil Pilote et Suivi Conso
// **********************************************************************************
// Copyright (C) 2014 Thibault Ducret
// Licence MIT
//
// History : 15/01/2015 Charles-Henri Hallard (http://hallard.me)
//                      Intégration de version 1.2 de la carte electronique
//           15/09/2015 Charles-Henri Hallard : Ajout compatibilité ESP8266
//           02/12/2015 Charles-Henri Hallard : Ajout API WEB ESP8266 et Remora V1.3
//           04/01/2016 Charles-Henri Hallard : Ajout Interface WEB GUIT
//
// **********************************************************************************
#ifndef REMORA_h
#define REMORA_h

// Spark Core main firmware include file
#ifdef SPARK
#include "application.h"
#endif

// Définir ici le type de carte utilisé
//#define REMORA_BOARD_V10  // Version 1.0
//#define REMORA_BOARD_V11  // Version 1.1
//#define REMORA_BOARD_V12  // Version 1.2
#define REMORA_BOARD_V13  // Version 1.3

//  Définir ici les modules utilisés sur la carte Remora
#define MOD_RF69      /* Module RF  */
//#define MOD_OLED      /* Afficheur  */
#define MOD_TELEINFO  /* Teleinfo   */
//#define MOD_RF_OREGON   /* Reception des sondes orégon */
#define MOD_ADPS          /* Délestage */

// Version logicielle remora
#define REMORA_VERSION "1.3.2"

// Définir ici votre authentification blynk, cela
// Activera automatiquement blynk http://blynk.cc
//#define BLYNK_AUTH "YourBlynkAuthToken"

// Librairies du projet remora Pour Particle
#ifdef SPARK
  #include "LibMCP23017.h"
  #include "LibSSD1306.h"
  #include "LibGFX.h"
  #include "LibULPNode_RF_Protocol.h"
  #include "LibLibTeleinfo.h"
  //#include "WebServer.h"

  #include "display.h"
  #include "i2c.h"
  #include "pilotes.h"
  #include "rfm.h"
  #include "tinfo.h"
  #include "linked_list.h"
  #include "route.h"
  #include "LibRadioHead.h"
  #include "LibRH_RF69.h"
  #include "LibRHDatagram.h"
  #include "LibRHReliableDatagram.h"

  //#include "OLED_local.h"
  //#include "mfGFX_local.h"

  #define _yield()  Particle.process()
  #define _wdt_feed {}
  #define DEBUG_SERIAL  Serial
#endif

// Librairies du projet remora Pour Particle
#ifdef ESP8266
  #if defined (REMORA_BOARD_V10) || defined (REMORA_BOARD_V11)
  #error "La version ESP8266 NodeMCU n'est pas compatible avec les cartes V1.1x"
  #endif

  // Définir ici les identifiants de
  // connexion à votre réseau Wifi
  // =====================================
//  #define DEFAULT_WIFI_SSID "VotreSSID"
//  #define DEFAULT_WIFI_PASS "VotreClé"
  #define DEFAULT_WIFI_AP_PASS "Remora_WiFi"
  // =====================================
  #define DEFAULT_OTA_PORT  8266
  #define DEFAULT_OTA_PASS  "Remora_OTA"
  #define DEFAULT_HOSTNAME  "remora"
  #include "Arduino.h"
  #include <EEPROM.h>
  #include <FS.h>
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266WebServer.h>
  #include <Ticker.h>
  #include <NeoPixelBus.h>
  
extern "C" {
#include "user_interface.h"
}

  #include "./LibMCP23017.h"
  //#include "./RFM69registers.h"
  //#include "./RFM69.h"
  #include "./LibSSD1306.h"
  #include "./LibGFX.h"
  #include "./LibULPNode_RF_Protocol.h"
  #include "./LibLibTeleinfo.h"
  #include "./LibRadioHead.h"
  #include "./LibRHReliableDatagram.h"

  #define _yield  yield
  #define _wdt_feed ESP.wdtFeed
  #define DEBUG_SERIAL  Serial
  //#define DEBUG_INIT          // Décommenter cette ligne si DEBUG_SERIAL est Serial1
#endif

#define DEBUG

// I prefix debug macro to be sure to use specific for THIS library
// debugging, this should not interfere with main sketch or other 
// libraries
#ifdef DEBUG
#define Debug(x)    DEBUG_SERIAL.print(x)
#define Debugln(x)  DEBUG_SERIAL.println(x)
#define DebugF(x)   DEBUG_SERIAL.print(F(x))
#define DebuglnF(x) DEBUG_SERIAL.println(F(x))
#define Debugf(...) DEBUG_SERIAL.printf(__VA_ARGS__)
#define Debugflush  DEBUG_SERIAL.flush
#else
#define Debug(x)    {}
#define Debugln(x)  {}
#define DebugF(x)   {}
#define DebuglnF(x) {}
#define Debugf(...) {}
#define Debugflush(){}
#endif

// Includes du projets remora
#include "config.h"
#include "linked_list.h"
#include "i2c.h"
#include "rfm.h"
#include "display.h"
#include "pilotes.h"
#include "tinfo.h"
#include "webserver.h"
#include "webclient.h"

// RGB LED related MACROS
#if defined (SPARK)
  #define COLOR_RED     255,   0,   0
  #define COLOR_ORANGE  255, 127,   0
  #define COLOR_YELLOW  255, 255,   0
  #define COLOR_GREEN     0, 255,   0
  #define COLOR_CYAN      0, 255, 255
  #define COLOR_BLUE      0,   0, 255
  #define COLOR_MAGENTA 255,   0, 255

  #define LedRGBOFF() RGB.color(0,0,0)
  #define LedRGBON(x) RGB.color(x)

  // RFM69 Pin mapping
  #define RF69_CS  SS // default SPI SS Pin
  #define RF69_IRQ 2

#elif defined (ESP8266)
  #define COLOR_RED     rgb_brightness, 0, 0
  #define COLOR_ORANGE  rgb_brightness, rgb_brightness>>1, 0
  #define COLOR_YELLOW  rgb_brightness, rgb_brightness, 0
  #define COLOR_GREEN   0, rgb_brightness, 0
  #define COLOR_CYAN    0, rgb_brightness, rgb_brightness
  #define COLOR_BLUE    0, 0, rgb_brightness
  #define COLOR_MAGENTA rgb_brightness, 0, rgb_brightness

  // On ESP8266 we use NeopixelBus library to drive neopixel RGB LED
  #define RGB_LED_PIN 0 // RGB Led driven by GPIO0
  #define LedRGBOFF() { rgb_led.SetPixelColor(0,0); rgb_led.Show(); }
  #define LedRGBON(x) { RgbColor color(x); rgb_led.SetPixelColor(0,color); rgb_led.Show(); }
  //#define LedRGBOFF() {}
  //#define LedRGBON(x) {}

  // RFM69 Pin mapping
  #define RF69_CS   15
  #define RF69_IRQ  2
#endif

// Ces modules ne sont pas disponibles sur les carte 1.0 et 1.1
#if defined (REMORA_BOARD_V10) || defined (REMORA_BOARD_V11)
  #undef MOD_RF69
  #undef MOD_OLED
  #undef MOD_RF_OREGON

  // en revanche le relais l'est sur la carte 1.1
  #ifdef REMORA_BOARD_V11
    #define RELAIS_PIN A1
  #endif

  // Creation macro unique et indépendante du type de
  // carte pour le controle des I/O
  #define _digitalWrite(p,v)  digitalWrite(p,v)
  #define _pinMode(p,v)       pinMode(p,v)

// Carte 1.2
#elif defined (REMORA_BOARD_V12)
  #define LED_PIN    8
  #define RELAIS_PIN 9

  // Creation macro unique et indépendante du type de
  // carte pour le controle des I/O
  #define _digitalWrite(p,v)  mcp.digitalWrite(p,v)
  #define _pinMode(p,v)       mcp.pinMode(p,v)

// Carte 1.3+
#elif defined (REMORA_BOARD_V13)
  #define LED_PIN    8
  #define RELAIS_PIN 9

  // Creation macro unique et indépendante du type de
  // carte pour le controle des I/O
  // sur la V1.3 on pilote à l'inverse (met à 0 pour passer)
  #define _digitalWrite(p,v)  mcp.digitalWrite(p,v==0?1:0)
  #define _pinMode(p,v)       mcp.pinMode(p,v)
#endif

// Masque de bits pour le status global de l'application
#define STATUS_MCP    0x0001 // I/O expander detecté
#define STATUS_OLED   0x0002 // Oled detecté
#define STATUS_RFM    0x0004 // RFM69  detecté
#define STATUS_TINFO  0x0008 // Trame téléinfo detecté

// Variables exported to other source file
// ========================================
// define var for whole project

// status global de l'application
extern uint16_t status;
extern unsigned long uptime ;


#ifdef SPARK
  // Particle WebServer
  //extern WebServer server("", 80);
#endif

#ifdef ESP8266

  typedef NeoPixelBus<NeoRgbFeature, NeoEsp8266BitBang800KbpsMethod> MyPixelBus;

  // ESP8266 WebServer
  extern ESP8266WebServer server;
    // RGB LED
  //extern NeoPixelBus rgb_led;
  //extern NeoPixelBus rgb_led(1, RGB_LED_PIN);
  //extern template ReallyBigFunction<int>();
  //extern  class NeoPixelBus rgb_led();
  extern MyPixelBus rgb_led;
  //extern  template class NeoPixelBus<NeoRgbFeature, NeoEsp8266BitBang800KbpsMethod> rgb_led;

  // define whole brigtness level for RGBLED
  extern uint8_t rgb_brightness;

  extern Ticker Tick_emoncms;
  extern Ticker Tick_jeedom;
#endif


extern uint16_t status; // status global de l'application

// Function exported for other source file
// =======================================
char * timeAgo(unsigned long);
void Task_emoncms();
void Task_jeedom();

#endif

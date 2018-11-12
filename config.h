// **********************************************************************************
// Remora Configuration include file
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
// History 2016-02-04 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#ifndef __CONFIG_H__
#define __CONFIG_H__

// Include main project include file
#include "remora.h"

#ifdef ESP8266

#define CFG_SSID_SIZE 		32
#define CFG_PSK_SIZE  		64
#define CFG_HOSTNAME_SIZE 16

#define CFG_EMON_HOST_SIZE 		32
#define CFG_EMON_APIKEY_SIZE 	32
#define CFG_EMON_URL_SIZE 		32
#define CFG_EMON_DEFAULT_PORT 80
#define CFG_EMON_DEFAULT_HOST "emoncms.org"
#define CFG_EMON_DEFAULT_URL  "/input/post.json"

#define CFG_JDOM_HOST_SIZE         32
#define CFG_JDOM_APIKEY_SIZE       48
#define CFG_JDOM_URL_SIZE          64
#define CFG_JDOM_ADCO_SIZE         12
#define CFG_JDOM_FINGER_PRINT_SIZE 20
#define CFG_JDOM_DEFAULT_PORT      80
#define CFG_JDOM_DEFAULT_HOST      "jeedom.local"
#define CFG_JDOM_DEFAULT_URL       "/jeedom/plugins/teleinfo/core/php/jeeTeleinfo.php"
#define CFG_JDOM_DEFAULT_ADCO      "000011112222"

#define DEFAULT_LED_BRIGHTNESS  50                // 50%

// Port pour l'OTA
#define DEFAULT_OTA_PORT     8266
#define DEFAULT_OTA_AUTH     "OTA_Remora"
//#define DEFAULT_OTA_AUTH     ""

// Bit definition for different configuration modes
#define CFG_LCD				  0x0001	// Enable display
#define CFG_DEBUG			  0x0002	// Enable serial debug
#define CFG_RGB_LED     0x0004  // Enable RGB LED
#define CFG_LED_AWAKE   0x0008 // Enable led blink when awake (take care, consumption)
#define CFG_LED_TX      0x0010 // Enable led blink when RF transmitting (take care, consumption)
#define CFG_FLIP_LCD    0x0020 // Flip display
#define CFG_RF_NOSEND   0x0040 // Disable this gateway to send packet (including ACK)
#define CFG_RF_ACK      0x0080 // Enable this gateway to request ACK on send
#define CFG_BAD_CRC     0x8000  // Bad CRC when reading configuration

// Web Interface Configuration Form field names
#define CFG_FORM_SSID     FPSTR("ssid")
#define CFG_FORM_PSK      FPSTR("psk")
#define CFG_FORM_HOST     FPSTR("host")
#define CFG_FORM_AP_PSK   FPSTR("ap_psk")
#define CFG_FORM_OTA_AUTH FPSTR("ota_auth")
#define CFG_FORM_OTA_PORT FPSTR("ota_port")

#define CFG_FORM_EMON_HOST  FPSTR("emon_host")
#define CFG_FORM_EMON_PORT  FPSTR("emon_port")
#define CFG_FORM_EMON_URL   FPSTR("emon_url")
#define CFG_FORM_EMON_KEY   FPSTR("emon_apikey")
#define CFG_FORM_EMON_NODE  FPSTR("emon_node")
#define CFG_FORM_EMON_FREQ  FPSTR("emon_freq")

#define CFG_FORM_JDOM_HOST  FPSTR("jdom_host")
#define CFG_FORM_JDOM_PORT  FPSTR("jdom_port")
#define CFG_FORM_JDOM_URL   FPSTR("jdom_url")
#define CFG_FORM_JDOM_KEY   FPSTR("jdom_apikey")
#define CFG_FORM_JDOM_ADCO  FPSTR("jdom_adco")
#define CFG_FORM_JDOM_FREQ  FPSTR("jdom_freq")
#define CFG_FORM_JDOM_FING  FPSTR("jdom_finger")

#define CFG_FORM_LED_BRIGHT FPSTR("cfg_led_bright");

#define CFG_FORM_IP  FPSTR("wifi_ip");
#define CFG_FORM_GW  FPSTR("wifi_gw");
#define CFG_FORM_MSK FPSTR("wifi_msk");

#pragma pack(push)  // push current alignment to stack
#pragma pack(1)     // set alignment to 1 byte boundary

// Config for emoncms
// 128 Bytes
typedef struct
{
  char  host[CFG_EMON_HOST_SIZE+1]; 		// FQDN
  char  apikey[CFG_EMON_APIKEY_SIZE+1]; // Secret
  char  url[CFG_EMON_URL_SIZE+1];  			// Post URL
  uint16_t port;    								    // Protocol port (HTTP/HTTPS)
  uint16_t node;     									  // optional node
  uint32_t freq;                        // refresh rate
  uint8_t filler[21];									  // in case adding data in config avoiding loosing current conf by bad crc*/
} _emoncms;

// Config for jeedom
// 256 Bytes
typedef struct
{
  char     host[CFG_JDOM_HOST_SIZE+1];              // FQDN
  char     apikey[CFG_JDOM_APIKEY_SIZE+1];          // Secret
  char     url[CFG_JDOM_URL_SIZE+1];                // Post URI
  char     adco[CFG_JDOM_ADCO_SIZE+1];              // Identifiant compteur
  uint8_t  fingerprint[CFG_JDOM_FINGER_PRINT_SIZE]; // Finger print SHA1 SSL
  uint16_t port;                                    // Protocol port (HTTP/HTTPS)
  uint32_t freq;                                    // refresh rate
  uint8_t  filler[70];                              // in case adding data in config avoiding loosing current conf by bad crc*/
} _jeedom;

// Config saved into eeprom
// 1024 bytes total including CRC
typedef struct
{
  char  ssid[CFG_SSID_SIZE+1]; 		 // SSID
  char  psk[CFG_PSK_SIZE+1]; 		   // Pre shared key
  char  host[CFG_HOSTNAME_SIZE+1]; // Hostname
  char  ap_psk[CFG_PSK_SIZE+1];    // Access Point Pre shared key
  char  ota_auth[CFG_PSK_SIZE+1];  // OTA Authentication password
  uint32_t config;           		   // Bit field register
  uint16_t ota_port;         		   // OTA port
  uint8_t led_bright;              // RGB Led brightness 252
  uint8_t  filler[130];      		   // in case adding data in config avoiding loosing current conf by bad crc
  _emoncms emoncms;                // Emoncms configuration
  _jeedom  jeedom;                 // jeedom configuration
  uint8_t  filler1[256];           // Another filler in case we need more
  uint16_t crc;
} _Config;


// Exported variables/object instancied in main sketch
// ===================================================
extern _Config config;

#pragma pack(pop)

// Declared exported function from route.cpp
// ===================================================
bool readConfig(bool clear_on_error=true);
bool saveConfig(void);
void showConfig(void);
void resetConfig(void);
String getFingerPrint(void);

#endif // ESP8266
#endif // CONFIG_h

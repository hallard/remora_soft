// **********************************************************************************
// Remora WEB Server, route web function
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
// History 2015-06-14 - First release
//         2015-11-31 - Added Remora API
//         2016-01-04 - Added Web Interface part
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

// Include header
#include "webserver.h"

// Optimize string space in flash, avoid duplication
const char FP_JSON_START[] PROGMEM = "{\r\n";
const char FP_JSON_END[] PROGMEM = "\r\n}\r\n";
const char FP_QCQ[] PROGMEM = "\":\"";
const char FP_QCNL[] PROGMEM = "\",\r\n\"";
const char FP_RESTART[] PROGMEM = "OK, Redémarrage en cours\r\n";
const char FP_NL[] PROGMEM = "\r\n";


#ifdef ESP8266

/* ======================================================================
Function: getContentType
Purpose : return correct mime content type depending on file extension
Input   : -
Output  : Mime content type
Comments: -
====================================================================== */
String getContentType(String filename) {
  if(filename.endsWith(".htm")) return F("text/html");
  else if(filename.endsWith(".html")) return F("text/html");
  else if(filename.endsWith(".css")) return F("text/css");
  else if(filename.endsWith(".json")) return F("application/json");
  else if(filename.endsWith(".js")) return F("application/javascript");
  else if(filename.endsWith(".png")) return F("image/png");
  else if(filename.endsWith(".gif")) return F("image/gif");
  else if(filename.endsWith(".jpg")) return F("image/jpeg");
  else if(filename.endsWith(".ico")) return F("image/x-icon");
  else if(filename.endsWith(".xml")) return F("text/xml");
  else if(filename.endsWith(".pdf")) return F("application/x-pdf");
  else if(filename.endsWith(".zip")) return F("application/x-zip");
  else if(filename.endsWith(".gz")) return F("application/x-gzip");
  else if(filename.endsWith(".otf")) return F("application/x-font-opentype");
  else if(filename.endsWith(".eot")) return F("application/vnd.ms-fontobject");
  else if(filename.endsWith(".svg")) return F("image/svg+xml");
  else if(filename.endsWith(".woff")) return F("application/x-font-woff");
  else if(filename.endsWith(".woff2")) return F("application/x-font-woff2");
  else if(filename.endsWith(".ttf")) return F("application/x-font-ttf");
  return "text/plain";
}

/* ======================================================================
Function: formatSize
Purpose : format a asize to human readable format
Input   : size
Output  : formated string
Comments: -
====================================================================== */
String formatSize(size_t bytes)
{
  if (bytes < 1024){
    return String(bytes) + F(" Byte");
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0) + F(" KB");
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0) + F(" MB");
  } else {
    return String(bytes/1024.0/1024.0/1024.0) + F(" GB");
  }
}

/* ======================================================================
Function: handleFileRead
Purpose : return content of a file stored on SPIFFS file system
Input   : file path
Output  : true if file found and sent
Comments: -
====================================================================== */
bool handleFileRead(String path, AsyncWebServerRequest *request) {
  if ( path.endsWith("/") )
    path += "index.htm";

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  bool gzip = false;

  DebugF("handleFileRead ");
  Debug(path);

  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)){
      path += ".gz";
      DebugF(".gz");
      gzip = true;
    }

    DebuglnF(" found on FS");
    DebugF("ContentType: "); Debugln(contentType);

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    if (gzip) {
      DebuglnF("Add header content-encoding: gzip");
      response->addHeader("Content-Encoding", "gzip");
    }
    request->send(response);
    return true;
  }

  Debugln("");

  request->send(404, "text/plain", "File Not Found");
  return false;
}

/* ======================================================================
Function: formatNumberJSON
Purpose : check if data value is full number and send correct JSON format
Input   : String where to add response
          char * value to check
Output  : -
Comments: 00150 => 150
          ADCO  => "ADCO"
          1     => 1
====================================================================== */
void formatNumberJSON( String &response, char * value)
{
  // we have at least something ?
  if (value && strlen(value))
  {
    boolean isNumber = true;
    uint8_t c;
    char * p = value;

    // just to be sure
    if (strlen(p)<=16) {
      // check if value is number
      while (*p && isNumber) {
        if ( *p < '0' || *p > '9' )
          isNumber = false;
        p++;
      }

      // this will add "" on not number values
      if (!isNumber) {
        response += '\"' ;
        response += value ;
        response += '\"' ;
      } else {
        // this will remove leading zero on numbers
        p = value;
        while (*p=='0' && *(p+1) )
          p++;
        response += p ;
      }
    } else {
      response += "\"Error Value too long\"" ;
      DebuglnF("formatNumberJSON Value too long!");
    }
  } else {
    response += "\"Error Bad Value\"" ;
    DebuglnF("formatNumberJSON Bad Value!");
  }
}

/* ======================================================================
Function: tinfoJSONTable
Purpose : dump all teleinfo values in JSON table format for browser
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : -
Comments: -
====================================================================== */
void tinfoJSONTable(AsyncWebServerRequest *request)
{

  // Just to debug where we are
  DebugF("Serving /tinfo page...\r\n");

  #ifdef MOD_TELEINFO

  ValueList * me = tinfo.getList();
  String response = "";

  // Got at least one ?
  if (me) {
    uint8_t index=0;

    boolean first_item = true;
    // Json start
    response += F("[\r\n");

    // Loop thru the node
    while (me->next) {

      // we're there
      ESP.wdtFeed();

      // go to next node
      me = me->next;

      // First item do not add , separator
      if (first_item)
        first_item = false;
      else
        response += F(",\r\n");

      response += F("{\"na\":\"");
      response +=  me->name ;
      response += F("\", \"va\":\"") ;
      response += me->value;
      response += F("\", \"ck\":\"") ;
      if (me->checksum == '"' || me->checksum == '\\' || me->checksum == '/')
        response += '\\';
      response += (char) me->checksum;
      response += F("\", \"fl\":");
      response += me->flags ;
      response += '}' ;

    }
   // Json end
   response += F("\r\n]");

  } else {
    DebuglnF("sending 404...");
    request->send(404, "text/plain", "No data");
  }
  DebugF("sending...");
  request->send(200, "application/json", response);
  DebuglnF("OK!");

  #else
    DebuglnF("sending 404...");
    request->send(404, "text/plain", "Teleinfo non activée");
  #endif // MOD_TELEINFO

}

/* ======================================================================
Function: getSysJSONData
Purpose : Return JSON string containing system data
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void getSysJSONData(String & response)
{
  response = "";
  char buffer[32];
  char fp;
  int32_t adc = ( 1000 * analogRead(A0) / 1024 );

  // Json start
  response += F("[\r\n");

  response += "{\"na\":\"Uptime\",\"va\":\"";
  response += uptime;
  response += "\"},\r\n";

  response += "{\"na\":\"Version Logiciel\",\"va\":\"" REMORA_VERSION "\"},\r\n";

  response += "{\"na\":\"Compilé le\",\"va\":\"" __DATE__ " " __TIME__ "\"},\r\n";

  response += "{\"na\":\"Version Matériel\",\"va\":\"";
  #if defined (REMORA_BOARD_V10)
    response += F("V1.0");
  #elif defined (REMORA_BOARD_V11)
    response += F("V1.1");
  #elif defined (REMORA_BOARD_V12)
    response += F("V1.2 avec MCP23017");
  #elif defined (REMORA_BOARD_V13)
    response += F("V1.3 avec MCP23017");
  #elif defined (REMORA_BOARD_V14)
    response += F("V1.4 avec MCP23017");
  #else
    response += F("Non définie");
  #endif
  response += "\"},\r\n";

  response += "{\"na\":\"Modules activés\",\"va\":\"";
  #ifdef MOD_OLED
    response += F("OLED ");
  #endif
  #ifdef MOD_TELEINFO
    response += F("TELEINFO ");
  #endif
  #ifdef MOD_RF69
    response += F("RFM69 ");
  #endif
  #ifdef MOD_ADPS
    response += F("ADPS");
  #endif
  response += "\"},\r\n";

  response += "{\"na\":\"SDK Version\",\"va\":\"";
  response += system_get_sdk_version() ;
  response += "\"},\r\n";

  response += "{\"na\":\"Chip ID\",\"va\":\"";
  sprintf_P(buffer, "0x%0X",system_get_chip_id() );
  response += buffer ;
  response += "\"},\r\n";

  response += "{\"na\":\"Boot Version\",\"va\":\"";
  sprintf_P(buffer, "0x%0X",system_get_boot_version() );
  response += buffer ;
  response += "\"},\r\n";

  response += "{\"na\":\"Flash Real Size\",\"va\":\"";
  response += formatSize(ESP.getFlashChipRealSize()) ;
  response += "\"},\r\n";

  response += "{\"na\":\"Firmware Size\",\"va\":\"";
  response += formatSize(ESP.getSketchSize()) ;
  response += "\"},\r\n";

  response += "{\"na\":\"Free Size\",\"va\":\"";
  response += formatSize(ESP.getFreeSketchSpace()) ;
  response += "\"},\r\n";

  response += "{\"na\":\"Analog\",\"va\":\"";
  adc = ( (1000 * analogRead(A0)) / 1024);
  sprintf_P( buffer, PSTR("%d mV"), adc);
  response += buffer ;
  response += "\"},\r\n";

  FSInfo info;
  SPIFFS.info(info);

  response += "{\"na\":\"SPIFFS Total\",\"va\":\"";
  response += formatSize(info.totalBytes) ;
  response += "\"},\r\n";

  response += "{\"na\":\"SPIFFS Used\",\"va\":\"";
  response += formatSize(info.usedBytes) ;
  response += "\"},\r\n";

  response += "{\"na\":\"SPIFFS Occupation\",\"va\":\"";
  sprintf_P(buffer, "%d%%",100*info.usedBytes/info.totalBytes);
  response += buffer ;
  response += "\"},\r\n";

  // regarder l'état de tous les fils Pilotes
  for (uint8_t i=1; i<=NB_FILS_PILOTES; i++)
  {
    fp = etatFP[i-1];
    response += "{\"na\":\"Fil Pilote #";
    response += String(i);
    response += "\",\"va\":\"";
    if      (fp=='E') response += "Eco";
    else if (fp=='A') response += "Arrêt";
    else if (fp=='H') response += "Hors Gel";
    else if (fp=='1') response += "Eco - 1";
    else if (fp=='2') response += "Eco - 2";
    else if (fp=='C') response += "Confort";
    response += "\"},\r\n";
  }

  response += "{\"na\":\"Etat Relais\",\"va\":\"";
  response += etatrelais ? "Fermé":"Ouvert";
  response += "\"},\r\n";

  response += "{\"na\":\"Fnct Relais\",\"va\":\"";
  response += (fnctRelais == FNCT_RELAIS_AUTO) ? "Auto" : (fnctRelais == FNCT_RELAIS_FORCE) ? "Force" : "Stop";
  response += "\"},\r\n";

  response += "{\"na\":\"Delestage\",\"va\":\"";
  response += String(myDelestLimit);
  response += "A";
  response += "\"},\r\n";

  response += "{\"na\":\"Relestage\",\"va\":\"";
  response += String(myRelestLimit);
  response += "A";
  response += "\"},\r\n";

  response += "{\"na\":\"Etat Delestage\",\"va\":\"";
  #ifdef MOD_ADPS
    response += "Niveau ";
    response += String(nivDelest);
    response += " Zone ";
    response += String(plusAncienneZoneDelestee);
  #else
    response += "désactivé";
  #endif
  response += "\"},\r\n";

  // Free mem should be last one
  response += "{\"na\":\"Free Ram\",\"va\":\"";
  response += formatSize(system_get_free_heap_size()) ;
  response += "\"}\r\n"; // Last don't have comma at end

  // Json end
  response += F("]\r\n");
}

/* ======================================================================
Function: sysJSONTable
Purpose : dump all sysinfo values in JSON table format for browser
Input   : -
Output  : -
Comments: -
====================================================================== */
void sysJSONTable(AsyncWebServerRequest *request)
{
  String response = "";

  getSysJSONData(response);

  // Just to debug where we are
  DebugF("Serving /system page...");
  request->send(200, "application/json", response);
  DebuglnF("Ok!");
}

/* ======================================================================
Function: getConfigJSONData
Purpose : Return JSON string containing configuration data
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void getConfJSONData(String & r)
{
  // Json start
  r = FPSTR(FP_JSON_START);

  r+="\"";
  r+=CFG_FORM_SSID;       r+=FPSTR(FP_QCQ); r+=config.ssid;           r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_PSK;        r+=FPSTR(FP_QCQ); r+=config.psk;            r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_HOST;       r+=FPSTR(FP_QCQ); r+=config.host;           r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_AP_PSK;     r+=FPSTR(FP_QCQ); r+=config.ap_psk;         r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_HOST;  r+=FPSTR(FP_QCQ); r+=config.emoncms.host;   r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_PORT;  r+=FPSTR(FP_QCQ); r+=config.emoncms.port;   r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_URL;   r+=FPSTR(FP_QCQ); r+=config.emoncms.url;    r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_KEY;   r+=FPSTR(FP_QCQ); r+=config.emoncms.apikey; r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_NODE;  r+=FPSTR(FP_QCQ); r+=config.emoncms.node;   r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_FREQ;  r+=FPSTR(FP_QCQ); r+=config.emoncms.freq;   r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_OTA_AUTH;   r+=FPSTR(FP_QCQ); r+=config.ota_auth;       r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_OTA_PORT;   r+=FPSTR(FP_QCQ); r+=config.ota_port;       r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_LED_BRIGHT; r+=FPSTR(FP_QCQ); r+=config.led_bright;     r+= FPSTR(FP_QCNL);

  r+=CFG_FORM_JDOM_HOST;  r+=FPSTR(FP_QCQ); r+=config.jeedom.host;    r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_PORT;  r+=FPSTR(FP_QCQ); r+=config.jeedom.port;    r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_URL;   r+=FPSTR(FP_QCQ); r+=config.jeedom.url;     r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_KEY;   r+=FPSTR(FP_QCQ); r+=config.jeedom.apikey;  r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_ADCO;  r+=FPSTR(FP_QCQ); r+=config.jeedom.adco;    r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_FREQ;  r+=FPSTR(FP_QCQ); r+=config.jeedom.freq;

  r+= F("\"");
  // Json end
  r += FPSTR(FP_JSON_END);

}

/* ======================================================================
Function: confJSONTable
Purpose : dump all config values in JSON table format for browser
Input   : -
Output  : -
Comments: -
====================================================================== */
void confJSONTable(AsyncWebServerRequest *request)
{
  String response = "";
  getConfJSONData(response);
  // Just to debug where we are
  DebugF("Serving /config page...");
  request->send(200, "application/json", response);
  DebuglnF("Ok!");
}

/* ======================================================================
Function: getSpiffsJSONData
Purpose : Return JSON string containing list of SPIFFS files
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void getSpiffsJSONData(String & response)
{
  char buffer[32];
  bool first_item = true;

  // Json start
  response = FPSTR(FP_JSON_START);

  // Files Array
  response += F("\"files\":[\r\n");

  // Loop trough all files
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    if (first_item)
      first_item=false;
    else
      response += ",";

    response += F("{\"na\":\"");
    response += fileName.c_str();
    response += F("\",\"va\":\"");
    response += fileSize;
    response += F("\"}\r\n");
  }
  response += F("],\r\n");


  // SPIFFS File system array
  response += F("\"spiffs\":[\r\n{");

  // Get SPIFFS File system informations
  FSInfo info;
  SPIFFS.info(info);
  response += F("\"Total\":");
  response += info.totalBytes ;
  response += F(", \"Used\":");
  response += info.usedBytes ;
  response += F(", \"ram\":");
  response += system_get_free_heap_size() ;
  response += F("}\r\n]");

  // Json end
  response += FPSTR(FP_JSON_END);
}

/* ======================================================================
Function: spiffsJSONTable
Purpose : dump all spiffs system in JSON table format for browser
Input   : -
Output  : -
Comments: -
====================================================================== */
void spiffsJSONTable(AsyncWebServerRequest *request)
{
  String response = "";
  getSpiffsJSONData(response);
  request->send(200, "application/json", response);
}

/* ======================================================================
Function: wifiScanJSON
Purpose : scan Wifi Access Point and return JSON code
Input   : -
Output  : -
Comments: -
====================================================================== */
void wifiScanJSON(AsyncWebServerRequest *request)
{
  String response = "";
  bool first = true;
  int scanStatus = WiFi.scanComplete();

  // Just to debug where we are
  DebugF("Serving /wifiscan page...");

  // Json start
  response += F("{\r\n");

  //int n = WiFi.scanNetworks();
  if (scanStatus == WIFI_SCAN_FAILED) {
    WiFi.scanNetworks(true);
    response += F("\"status\": \"Scan in progess\"");
  } else if (scanStatus >= 0) {
    response += F("\"result\": [");
    for (uint8_t i = 0; i < scanStatus; ++i) {
      int8_t rssi = WiFi.RSSI(i);

      uint8_t percent;

      // dBm to Quality
      if(rssi<=-100)      percent = 0;
      else if (rssi>=-50) percent = 100;
      else                percent = 2 * (rssi + 100);

      if (first)
        first = false;
      else
        response += F(",");

      response += F("{\"ssid\":\"");
      response += WiFi.SSID(i);
      response += F("\",\"rssi\":") ;
      response += rssi;
      response += FPSTR(FP_JSON_END);
    }
    response += F("],\"status\": \"OK\"");
    WiFi.scanDelete();
  }

  // Json end
  response += FPSTR("}\r\n");

  Debugln(response);

  DebugF("sending...");
  request->send(200, "application/json", response);
  DebuglnF("Ok!");
}



/* ======================================================================
Function: tinfoJSON
Purpose : dump all teleinfo values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void tinfoJSON(AsyncWebServerRequest *request)
{
  #ifdef MOD_TELEINFO
    ValueList * me = tinfo.getList();
    String response = "";

    // Got at least one ?
    if (me) {
      char * p;
      long value;

      // Json start
      response += FPSTR(FP_JSON_START);
      response += F("\"_UPTIME\":");
      response += uptime;
      response += FPSTR(FP_NL) ;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;

        if (tinfo.calcChecksum(me->name,me->value) == me->checksum) {
          response += F(",\"") ;
          response += me->name ;
          response += F("\":") ;

          // Check if value is a number
          value = strtol(me->value, &p, 10);

          // conversion failed, add "value"
          if (*p) {
            response += F("\"") ;
            response += me->value ;
            response += F("\"") ;

          // number, add "value"
          } else {
            response += value ;
          }
          //formatNumberJSON(response, me->value);
        } else {
          response = F(",\"_Error\":\"");
          response = me->name;
          response = "=";
          response = me->value;
          response = F(" CHK=");
          response = (char) me->checksum;
          response = "\"";
        }

        // Add new line to see more easier end of field
        response += FPSTR(FP_NL) ;

      }
      // Json end
      response += FPSTR(FP_JSON_END) ;
    } else {
      request->send(404, "text/plain", "No data");
    }
    request->send(200, "application/json", response);
  #else
    request->send(404, "text/plain", "teleinfo not enabled");
  #endif
}


/* ======================================================================
Function: fpJSON
Purpose : return fp values in JSON
Input   : string
          fp number 1 to NB_FILS_PILOTE (0=ALL)
Output  : -
Comments: -
====================================================================== */
void fpJSON(String & response, uint8_t fp)
{
  bool first_elem = true;

  // petite verif
  if (fp>=0 && fp<=NB_FILS_PILOTES) {
    response = FPSTR(FP_JSON_START);

    // regarder l'état de tous les fils Pilotes
    for (uint8_t i=1; i<=NB_FILS_PILOTES; i++)
    {
      // Tout les fils pilote ou juste celui demandé
      if (fp==0 || fp==i) {
        if (!first_elem)
          response+= ",\r\n";
        else
          first_elem=false;

        response+= "\"fp";
        response+= String(i);
        response+= "\": \"";
        response+= etatFP[i-1];
        response+= "\"";
      }
    }
    response+= FPSTR(FP_JSON_END);
  }
}


/* ======================================================================
Function: relaisJSON
Purpose : return relais value in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void relaisJSON(String & response)
{
  response = FPSTR(FP_JSON_START);
  response+= "\"relais\": ";
  response+= String(etatrelais);
  response+= ",\r\n\"fnct_relais\": ";
  response+= String(fnctRelais);
  response+= FPSTR(FP_JSON_END);
}

/* ======================================================================
Function: delestageJSON
Purpose : return delestage values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void delestageJSON(String & response)
{
    response = FPSTR(FP_JSON_START);
    #ifdef MOD_ADPS
      response += FPSTR("\"niveau\": ");
      response += String(nivDelest);
      response += FPSTR(", \"zone\": ");
      response += String(plusAncienneZoneDelestee);
    #else
      response += FPSTR("\"etat\": \"désactivé\"");
    #endif
    response += FPSTR(FP_JSON_END);
}


/* ======================================================================
Function: handleFactoryReset
Purpose : reset the module to factory settingd
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFactoryReset(AsyncWebServerRequest *request)
{
  // Just to debug where we are
  DebugF("Serving /factory_reset page...");
  resetConfig();
  ESP.eraseConfig();
  request->send(200, "text/plain", FPSTR(FP_RESTART));
  delay(1000);
  ESP.restart();
  while (true)
    delay(1);
}

/* ======================================================================
Function: handleReset
Purpose : reset the module
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleReset(AsyncWebServerRequest *request)
{
  // Just to debug where we are
  DebugF("Serving /reset page...");
  request->send(200, "text/plain", FPSTR(FP_RESTART));
  delay(1000);
  ESP.restart();
  // This will fire watchdog
  while (true)
    delay(1);
}

/* ======================================================================
Function: handleFormConfig
Purpose : handle main configuration page
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFormConfig(AsyncWebServerRequest *request)
{
  String response="";
  int ret ;
  boolean showconfig = false;

  // We validated config ?
  if (request->hasParam("save", true)) {
    int itemp;
    DebuglnF("===== Posted configuration");

    // WifInfo
    strncpy(config.ssid ,   request->getParam("ssid", true)->value().c_str(),     CFG_SSID_SIZE );
    strncpy(config.psk ,    request->getParam("psk", true)->value().c_str(),      CFG_PSK_SIZE );
    strncpy(config.host ,   request->getParam("host", true)->value().c_str(),     CFG_HOSTNAME_SIZE );
    strncpy(config.ap_psk , request->getParam("ap_psk", true)->value().c_str(),   CFG_PSK_SIZE );
    if (config.ota_auth != request->getParam("ota_auth", true)->value().c_str()) {
      strncpy(config.ota_auth, request->getParam("ota_auth", true)->value().c_str(), CFG_PSK_SIZE );
      reboot = true;
    }
    if (request->hasParam("ota_port", true)) {
      itemp = request->getParam("ota_port", true)->value().toInt();
      config.ota_port = (itemp>=0 && itemp<=65535) ? itemp : DEFAULT_OTA_PORT;
    }
    if (request->hasParam("cfg_led_bright", true)) {
      DebugF("cfg_led_bright: "); Debugln(request->getParam("cfg_led_bright", true)->value()); Debugflush();
      config.led_bright = map(request->getParam("cfg_led_bright", true)->value().toInt(), 0, 100, 0, 255);
      rgb_brightness = config.led_bright;
    }

    // Emoncms
    strncpy(config.emoncms.host,   request->getParam("emon_host", true)->value().c_str(),  CFG_EMON_HOST_SIZE );
    strncpy(config.emoncms.url,    request->getParam("emon_url", true)->value().c_str(),   CFG_EMON_URL_SIZE );
    strncpy(config.emoncms.apikey, request->getParam("emon_apikey", true)->value().c_str(),CFG_EMON_APIKEY_SIZE );
    itemp = request->getParam("emon_node", true)->value().toInt();
    config.emoncms.node = (itemp>=0 && itemp<=255) ? itemp : 0 ;
    itemp = request->getParam("emon_port", true)->value().toInt();
    config.emoncms.port = (itemp>=0 && itemp<=65535) ? itemp : CFG_EMON_DEFAULT_PORT ;
    itemp = request->getParam("emon_freq", true)->value().toInt();
    if (itemp>0 && itemp<=86400){
      // Emoncms Update if needed
      Tick_emoncms.detach();
      Tick_emoncms.attach(itemp, Task_emoncms);
    } else {
      itemp = 0 ;
    }
    config.emoncms.freq = itemp;

    // jeedom
    strncpy(config.jeedom.host,   request->getParam("jdom_host", true)->value().c_str(),  CFG_JDOM_HOST_SIZE );
    strncpy(config.jeedom.url,    request->getParam("jdom_url", true)->value().c_str(),   CFG_JDOM_URL_SIZE );
    strncpy(config.jeedom.apikey, request->getParam("jdom_apikey", true)->value().c_str(),CFG_JDOM_APIKEY_SIZE );
    strncpy(config.jeedom.adco,   request->getParam("jdom_adco", true)->value().c_str(),CFG_JDOM_ADCO_SIZE );
    if (request->hasParam("jdom_port", true)) {
			itemp = request->getParam("jdom_port", true)->value().toInt();
			config.jeedom.port = (itemp>=0 && itemp<=65535) ? itemp : CFG_JDOM_DEFAULT_PORT;
		}
   if (request->hasParam("jdom_freq", true)) {
      itemp = request->getParam("jdom_freq", true)->value().toInt();
      if (itemp>0 && itemp<=86400){
        // Emoncms Update if needed
        Tick_jeedom.detach();
        Tick_jeedom.attach(itemp, Task_jeedom);
      } else {
        itemp = 0 ;
      }
      config.jeedom.freq = itemp;
   }

    if ( saveConfig() ) {
      ret = 200;
      response = "OK";
    } else {
      ret = 412;
      response = "Unable to save configuration";
    }

    showconfig = true;
  }
  else
  {
    ret = 400;
    response = "Missing Form Field";
  }

  DebugF("Sending response ");
  Debug(ret);
  Debug(":");
  Debugln(response);
  request->send (ret, "text/plain", response);

  // This is slow, do it after response sent
  if (showconfig)
    showConfig();
}

void handle_fw_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Update.runAsync(true);
    WiFiUDP::stopAll();
    DebugF("* Upload Started: "); Debugln(filename.c_str());
    LedRGBON(COLOR_MAGENTA);
    ota_blink = true;
    int command = U_FLASH;
    //Debugf("Magic Byte: %02X\n", data[0]);
    if (data[0] != 0xE9) {
      command = U_SPIFFS;
      SPIFFS.end();
      //DebuglnF("Command U_SPIFFS");
    }
    if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000, command)) {
      Update.printError(DEBUG_SERIAL);
    }
  }

  if (!Update.hasError()) {
    if (Update.write(data, len) != len) {
      DebugF("*** UPDATE ERROR: ");
      Update.printError(DEBUG_SERIAL);
      if (ota_blink) {
        LedRGBON(COLOR_RED);
      } else {
        LedRGBOFF();
      }
      ota_blink = !ota_blink;
    } else {
      if (ota_blink) {
        LedRGBON(COLOR_MAGENTA);
      } else {
        LedRGBOFF();
      }
      ota_blink = !ota_blink;
      Debug(".");
    }
  }

  if (final) {
    DebuglnF("* Upload Finished.");
    if (Update.end(true)) {
      Debugf("Update Success: %uB\n", index+len);
    } else {
      Update.printError(DEBUG_SERIAL);
    }
    LedRGBOFF();
  }
}

/* ======================================================================
Function: handleNotFound
Purpose : default WEB routing when URI is not found
Input   : -
Output  : -
Comments: We search is we have a name that match to this URI, if one we
          return it's pair name/value in json
====================================================================== */
void handleNotFound(AsyncWebServerRequest *request)
{
  String response = "";

  String sUri = request->url();
  const char * uri;
  bool found = false;
  bool first_elem = true;

  // convert uri to char * for compare
  uri = sUri.c_str();

  Debug("URI[");
  Debug(strlen(uri));
  Debug("]='");
  Debug(uri);
  Debugln("'");

  // Got consistent URI, skip fisrt / ?
  // Attention si ? dans l'URL çà ne fait pas partie de l'URI
  // mais de hasArg traité plus bas
  if (uri && *uri=='/' && *++uri ) {
    uint8_t len = strlen(uri);

    #ifdef MOD_TELEINFO
      // We check for an known label
      ValueList * me = tinfo.getList();

      // Got at least one ?
      if (me) {

        // Loop thru the linked list of values
        while (me->next && !found) {
          // go to next node
          me = me->next;

          //Debugf("compare to '%s' ", me->name);
          // Do we have this one ?
          if (!strcasecmp(me->name, uri)) {
            // no need to continue
            found = true;

            // Add to respone
            response += FPSTR("{\r\n\"") ;
            response += me->name ;
            response += F("\":") ;
            formatNumberJSON(response, me->value);
            response += FPSTR(FP_JSON_END);
          }
        }
      }
    #endif

    // Requêtes d'interrogation
    // ========================

    // http://ip_remora/relais
    if (!strcasecmp("relais", uri)) {
      relaisJSON(response);
      found = true;
    // http://ip_remora/delestage
    } else if (!strcasecmp("delestage", uri)) {
      delestageJSON(response);
      found = true;
    // http://ip_remora/fp ou http://ip_remora/fpx
    } else if ( (len==2 || len==3) && (uri[0]=='f'||uri[0]=='F') && (uri[1]=='p'||uri[1]=='P') ) {
      int8_t fp = -1;

      // http://ip_remora/fp
      if (len==2) {
        fp=0;

      // http://ip_remora/fpx
      } else if ( len==3 ) {
        fp = uri[2];
        if ( fp>='1' && fp<=('0'+NB_FILS_PILOTES) )
         fp -= '0';
      }

      if (fp>=0 && fp<=NB_FILS_PILOTES) {
        fpJSON(response, fp);
        found = true;
      }
    }
  } // valide URI


  // Requêtes modifiantes (cumulable)
  // ================================
  if (  request->hasParam("fp") ||
        request->hasParam("setfp") ||
        request->hasParam("relais") ||
        request->hasParam("frelais")) {

    int error = 0;
    response = FPSTR(FP_JSON_START);

    // http://ip_remora/?setfp=CMD
    if ( request->hasParam("setfp") ) {
      String value = request->getParam("setfp")->value();
      error += setfp(value);
    }
    // http://ip_remora/?fp=CMD
    if ( request->hasParam("fp") ) {
      String value = request->getParam("fp")->value();
      error += fp(value);
    }

    // http://ip_remora/?relais=n
    if ( request->hasParam("relais") ) {
      String value = request->getParam("relais")->value();
      // La nouvelle valeur n'est pas celle qu'on vient de positionner ?
      if ( relais(value) != request->getParam("relais")->value().toInt() )
        error--;
    }

    // http://ip_remora/?frelais=n (n: 0 | 1 | 2)
    if ( request->hasParam("frelais") ) {
      String value = request->getParam("frelais")->value();
      // La nouvelle valeur n'est pas celle qu'on vient de positionner ?
      if ( fnct_relais(value) != request->getParam("frelais")->value().toInt() )
        error--;
    }

    response += F("\"response\": ") ;
    response += String(error) ;

    response += FPSTR(FP_JSON_END);
    found = true;
  }

  // Got it, send json
  if (found) {
    request->send(200, "application/json", response);
  } else {
    // le fichier demandé existe sur le système SPIFFS ?
    found = handleFileRead(request->url(), request);
  }

  // send error message in plain text
  if (!found) {
    String message = F("File Not Found\n\n");
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += ( request->method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->params();
    message += "\n";

    uint8_t params = request->params();

    for ( uint8_t i = 0; i < params; i++ ) {
      AsyncWebParameter* p = request->getParam(i);
      message += " " + p->name() + ": " + p->value() + "\n";
    }
    request->send(404, "text/plain", message);
  }

}

#endif

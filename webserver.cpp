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
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

// Include header
#include "webserver.h"

#ifdef ESP8266
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
        response += F("\":") ;
      } else {
        // this will remove leading zero on numbers
        p = value;
        while (*p=='0' && *(p+1) )
          p++;
        response += p ;
      }
    } else {
      Serial.println(F("formatNumberJSON error!"));
    }
  }
}

/* ======================================================================
Function: tinfoJSON
Purpose : dump all values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void tinfoJSON(void)
{
  #ifdef MOD_TELEINFO
    ValueList * me = tinfo.getList();
    String response = "";

    // Got at least one ?
    if (me) {
      // Json start
      response += F("{\"_UPTIME\":");
      response += uptime;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;

        response += F(",\"") ;
        response += me->name ;
        response += F("\":") ;
        formatNumberJSON(response, me->value);
      }
     // Json end
     response += F("}\r\n") ;

    } else {
      server.send ( 404, "text/plain", "No data" );
    }
    server.send ( 200, "text/json", response );
  #else
    server.send ( 404, "text/plain", "teleinfo not enabled" );
  #endif
}

/* ======================================================================
Function: handleNotFound
Purpose : default WEB routing when URI is not found
Input   : -
Output  : -
Comments: We search is we have a name that match to this URI, if one we
          return it's pair name/value in json
====================================================================== */
void handleNotFound(void)
{
  String response = "";

  const char * uri;
  boolean found = false;

  // convert uri to char * for compare
  uri = server.uri().c_str();

  #ifdef MOD_TELEINFO
    // We check for an known label
    ValueList * me = tinfo.getList();

    // Got at least one and consistent URI ?
    if (me && uri && *uri=='/' && *++uri ) {

      // Loop thru the linked list of values
      while (me->next && !found) {

        // we're there
        ESP.wdtFeed();

        // go to next node
        me = me->next;

        //Debugf("compare to '%s' ", me->name);
        // Do we have this one ?
        if (stricmp (me->name, uri) == 0 )
        {
          // no need to continue
          found = true;

          // Add to respone
          response += F("{\"") ;
          response += me->name ;
          response += F("\":") ;
          formatNumberJSON(response, me->value);
          response += F("}\r\n");
        }
      }
    }
  #endif

  if ( server.hasArg("setfp") ) {
    response = "{";
    response+= "\"setfp\": ";
    response+= setfp(server.arg("setfp"));
    response+= "}";
    found = true;
  }

  if ( server.hasArg("fp") ) {
    response = "{";
    response+= "\"fp\": ";
    response+= setfp(server.arg("fp"));
    response+= "}";
    found = true;
  }

  if ( server.hasArg("relais") || server.hasArg("etatrelais")) {
    response = "{";
    response+= "\"relais\": ";

    if (server.hasArg("relais")) {
      relais(server.arg("relais"));
    }

    response+= etatrelais;
    response+= "}";
    found = true;
  }


  // Got it, send json
  if (found) {
    server.send ( 200, "text/json", response );
  } else {
    // send error message in plain text
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for ( uint8_t i = 0; i < server.args(); i++ ) {
      message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
    }

    server.send ( 404, "text/plain", message );
  }

}
#endif



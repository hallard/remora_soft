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
        response += F("\"") ;
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
Purpose : dump all teleinfo values in JSON
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
      response += FPSTR("{\r\n");
      response += F("\"_UPTIME\":");
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
      response += FPSTR("\r\n}\r\n") ;
    } else {
      server.send ( 404, "text/plain", "No data" );
    }
    server.send ( 200, "text/json", response );
  #else
    server.send ( 404, "text/plain", "teleinfo not enabled" );
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
    response = FPSTR("{\r\n");

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
    response+= FPSTR("\r\n}\r\n");
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
  response = FPSTR("{\r\n");
  response+= "\"relais\": ";
  response+= String(etatrelais);
  response+= FPSTR("\r\n}\r\n");
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
    response = FPSTR("{\r\n");
    response += FPSTR("\"niveau\": ");
    response += String(nivDelest);
    response += FPSTR(", \"zone\": ");
    response += String(plusAncienneZoneDelestee);
    response += FPSTR("\r\n}\r\n");
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
  bool found = false;
  bool first_elem = true;

  // convert uri to char * for compare
  uri = server.uri().c_str();

  Serial.print("URI[");
  Serial.print(strlen(uri));
  Serial.print("]='");
  Serial.print(uri);
  Serial.println("'");

  // Got consistent URI, skip fisrt / ?
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
          if (!stricmp(me->name, uri)) {
            // no need to continue
            found = true;

            // Add to respone
            response += FPSTR("{\r\n\"") ;
            response += me->name ;
            response += F("\":") ;
            formatNumberJSON(response, me->value);
            response += FPSTR("\r\n}\r\n");
          }
        }
      }
    #endif

    // Requêtes d'interrogation
    // ========================

    // http://ip_remora/relais    
    if (!stricmp("relais", uri)) {
      relaisJSON(response);
      found = true;
    // http://ip_remora/delestage
    } else if (!stricmp("delestage", uri)) {
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
    
    // Requêtes modifiantes (cumulable)
    // ================================
    if (  server.hasArg("fp") || 
          server.hasArg("setfp") || 
          server.hasArg("relais")) {

        int error = 0;
        response = FPSTR("{\r\n");

        // http://ip_remora/?setfp=CMD
        if ( server.hasArg("setfp") ) {
          error += setfp(server.arg("setfp"));
        }
        // http://ip_remora/?fp=CMD
        if ( server.hasArg("fp") ) {
          error += fp(server.arg("fp"));
          found = true;
        }

        // http://ip_remora/?relais=n
        if ( server.hasArg("relais") ) {
          // La nouvelle valeur n'est pas celle qu'on vient de positionner ?
          if ( relais( server.arg("relais")) != server.arg("relais").toInt() ) 
            error -=1;
        }

        response += FPSTR("\"response\": ") ;
        response += String(error) ;

        response += FPSTR("\r\n}\r\n");
        found = true;
    }

  } // valide URI


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



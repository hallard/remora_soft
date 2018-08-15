// **********************************************************************************
// Remora WEB Client source code
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
// History  2016-01-04 - Creation
//          2018-08-15 - Ajout connexion SSL
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#include "webclient.h"

#ifdef ESP8266

/* ======================================================================
Function: httpPost
Purpose : Do a http post
Input   : hostname
          port
          url
Output  : true if received 200 OK
Comments: -
====================================================================== */
boolean httpPost(const char * host, const uint16_t port, const char * url, const uint8_t fingerprint[])
{
  bool ret = false;
  unsigned long start = millis();

  // Code pour une connexion sur un serveur SSL
  if (port == 443) {
    bool TOUT;
    int c_len;
    char c_buf[512];
    int bytes;
    String line;
//    int myWdt;
//    String content;
    int code;

    Debugf("Request to server SSL %s:%d\n", host, port);
//    myWdt = millis();
//    Debugln("Client Secure started");
//    Debugln("Free HEAP: " + String(ESP.getFreeHeap()));
    const int API_TIMEOUT = 5000;
    // client BearSSL
    BearSSL::WiFiClientSecure client;
    client.setTimeout(API_TIMEOUT);
    // Empreinte numérique SHA1 du serveur sécurisé 
    // (à renseigner dans le formulaire de configuration HTML de la Remora)
    client.setFingerprint(fingerprint);       // Permet de vérifier l'empreinte numérique du serveur SSL
    //=== Vous devez choisir entre la vérification du fingerprint ou insecure
    //client.setInsecure();          // Permet de ne pas avoir de vérification du serveur SSL
    client.setBufferSizes(512, 512); // Limite l'espace mémoire utilisé par la classe BearSSL pour la vérification de certificats x509
    
    client.setTimeout(API_TIMEOUT);
//    Debugln("Client Secure started " + String(millis() - myWdt) + " ms");
//    Debugln("Free HEAP: " + String(ESP.getFreeHeap()));
//    Debugln("connecting to " + String(host));
//    myWdt = millis();
    if (!client.connect(host, port)) {
//      Debugln("Connection failed " + String(millis() - myWdt) + " ms");
      Debugf("Connection failed to %s:%d\n", host, port);
      return ret;
    }
//    Debugln("Free HEAP: " + String(ESP.getFreeHeap()));
//    Debugln("Connected " + String(millis() - myWdt) + " ms");
//    Debugln("requesting URL: " + String(url));
//    myWdt = millis();

    Debugf("Requesting URL: %s", url);
    client.print(String("GET ") + url + " HTTP/1.0\r\n"
               + "Host: " + host + "\r\n"
               + "User-Agent: ESP8266\r\n\r\n");

//    Debugln("Request sent " + String(millis() - myWdt) + " ms");
//    Debugln("Free HEAP: " + String(ESP.getFreeHeap()));
    client.setTimeout(API_TIMEOUT);

//    myWdt = millis();
    TOUT = 1;
    while (client.connected()) {
      line = client.readStringUntil('\n');
//      Debugln("Line: " + line);
      if (line.startsWith("HTTP/1.")) {
        code = line.substring(9, 12).toInt();
//        Debugln("Got HTTP code " + String(code));
      }
      if (line.startsWith("Content-Length: ")) {
        c_len = line.substring(15).toInt();
//        Debugln("Got Content-length: " + String(c_len));
      }
      if (line == "\r") {
//        Debugln("Headers received " + String(millis() - myWdt) + " ms");
//        Debugln("Free HEAP: " + String(ESP.getFreeHeap()));
        TOUT = 0;
        break;
      }
    }
    if (TOUT) { 
      DebugF("\n*** Timeout receiving headers\n");
      return ret;
    }
//    myWdt = millis();
    if (client.available()) {
      TOUT = 1;
      //res.content += String(client.read());
      bytes = client.readBytes(c_buf, c_len);
      c_buf[c_len] = '\0';
      TOUT = 0;
    }
    if (TOUT) { 
      DebugF("\n*** Timeout receiving body\n");
      return ret;
    }
//    content = String(c_buf);
//    Debugln("Result length: " + String(content.length()) + " | " + String(bytes));
//    Debugln("Content received " + String(millis() - myWdt) + " ms");
//    Debugln("Free HEAP: " + String(ESP.getFreeHeap()));
//    myWdt = millis();
    client.stop();
//    Debugln("Client stop " + String(millis() - myWdt) + " ms");
//    Debugln("Free HEAP: " + String(ESP.getFreeHeap()));
//    Debugln("\n*** Result: " + content);
    ret = true;
  }
  // Code pour une connexion standard
  else {
    Debugf("Request to server %s:%d\n", host, port);
    HTTPClient http;
  
    // configure traged server and url
    http.begin(host, port, url);
    ESP.wdtFeed();
  
    Debugf("http://%s:%d%s => ", host, port, url);
    
    // start connection and send HTTP header
    int httpCode = http.sendRequest("GET");
    if(httpCode) {
        // HTTP header has been send and Server response header has been handled
        Debug(httpCode);
        DebugF(" ");
        // file found at server
        if(httpCode == 200) {
          String payload = http.getString();
          Debug(payload);
          ret = true;
        }
    } else {
        DebugF("failed!");
    }
    http.end();
  }
  
  Debugf(" in %d ms\r\n",millis()-start);
  Debugflush();
  return ret;
}

/* ======================================================================
Function: emoncmsPost
Purpose : Do a http post to emoncms
Input   : 
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean emoncmsPost(void)
{
  boolean ret = false;

  #ifdef MOD_TELEINFO  

  // Some basic checking
  if (*config.emoncms.host) {
    ValueList * me = tinfo.getList();
    // Got at least one ?
    if (me && me->next) {
      String url ; 
      boolean first_item;

      url = *config.emoncms.url ? config.emoncms.url : "/";
      url += "?";
      if (config.emoncms.node>0) {
        url+= F("node=");
        url+= String(config.emoncms.node);
        url+= "&";
      } 

      url += F("apikey=") ;
      url += config.emoncms.apikey;
      url += F("&json={") ;

      first_item = true;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;
        // First item do not add , separator
        if (first_item)
          first_item = false;
        else
          url += ",";

        url +=  me->name ;
        url += ":" ;

        // EMONCMS ne sais traiter que des valeurs numériques, donc ici il faut faire une 
        // table de mappage, tout à fait arbitraire, mais c"est celle-ci dont je me sers 
        // depuis mes débuts avec la téléinfo
        if (!strcmp(me->name, "OPTARIF")) {
          // L'option tarifaire choisie (Groupe "OPTARIF") est codée sur 4 caractères alphanumériques 
          /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
          je mets le 4eme char à 0, trop de possibilités
          BASE => Option Base. 
          HC.. => Option Heures Creuses. 
          EJP. => Option EJP. 
          BBRx => Option Tempo
          */
          char * p = me->value;
            
               if (*p=='B'&&*(p+1)=='A'&&*(p+2)=='S') url += "1";
          else if (*p=='H'&&*(p+1)=='C'&&*(p+2)=='.') url += "2";
          else if (*p=='E'&&*(p+1)=='J'&&*(p+2)=='P') url += "3";
          else if (*p=='B'&&*(p+1)=='B'&&*(p+2)=='R') url += "4";
          else url +="0";
        } else if (!strcmp(me->name, "HHPHC")) {
          // L'horaire heures pleines/heures creuses (Groupe "HHPHC") est codé par un caractère A à Y 
          // J'ai choisi de prendre son code ASCII
          int code = *me->value;
          url += String(code);
        } else if (!strcmp(me->name, "PTEC")) {
          // La période tarifaire en cours (Groupe "PTEC"), est codée sur 4 caractères 
          /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
          TH.. => Toutes les Heures. 
          HC.. => Heures Creuses. 
          HP.. => Heures Pleines. 
          HN.. => Heures Normales. 
          PM.. => Heures de Pointe Mobile. 
          HCJB => Heures Creuses Jours Bleus. 
          HCJW => Heures Creuses Jours Blancs (White). 
          HCJR => Heures Creuses Jours Rouges. 
          HPJB => Heures Pleines Jours Bleus. 
          HPJW => Heures Pleines Jours Blancs (White). 
          HPJR => Heures Pleines Jours Rouges. 
          */
               if (!strcmp(me->value, "TH..")) url += "1";
          else if (!strcmp(me->value, "HC..")) url += "2";
          else if (!strcmp(me->value, "HP..")) url += "3";
          else if (!strcmp(me->value, "HN..")) url += "4";
          else if (!strcmp(me->value, "PM..")) url += "5";
          else if (!strcmp(me->value, "HCJB")) url += "6";
          else if (!strcmp(me->value, "HCJW")) url += "7";
          else if (!strcmp(me->value, "HCJR")) url += "8";
          else if (!strcmp(me->value, "HPJB")) url += "9";
          else if (!strcmp(me->value, "HPJW")) url += "10";
          else if (!strcmp(me->value, "HPJR")) url += "11";
          else url +="0";
        } else {
          url += me->value;
        }
      } // While me

      // Json end
      url += "}";

      // TODO: Gérer la connexion sécurisée pour EmonCMS.
      ret = httpPost( config.emoncms.host, config.emoncms.port, (char *) url.c_str(), {}) ;
    } // if me
  } // if host

  #endif // MOD_TELEINFO

  return ret;
}

/* ======================================================================
Function: jeedomPost
Purpose : Do a http post to jeedom server
Input   : 
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean jeedomPost(void)
{
  boolean ret = false;

  #ifdef MOD_TELEINFO  

  // Some basic checking
  if (*config.jeedom.host) {
    ValueList * me = tinfo.getList();
    // Got at least one ?
    if (me && me->next) {
      String url ; 
      boolean skip_item;

      url = *config.jeedom.url ? config.jeedom.url : "/";
      url += "?";

      // Config identifiant forcée ?
      if (*config.jeedom.adco) {
        url+= F("ADCO=");
        url+= config.jeedom.adco;
        url+= "&";
      } 

      url += F("api=") ;
      url += config.jeedom.apikey;
      url += F("&") ;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;
        skip_item = false;

        // Si ADCO déjà renseigné, on le remet pas
        if (!strcmp(me->name, "ADCO")) {
          if (*config.jeedom.adco)
            skip_item = true;
        }

        // Si Item virtuel, on le met pas
        if (*me->name =='_')
          skip_item = true;

        // On doit ajouter l'item ?
        if (!skip_item) {
          url +=  me->name ;
          url += "=" ;
          url +=  me->value;
          url += "&" ;
        }
      } // While me

      ret = httpPost(config.jeedom.host, config.jeedom.port, 
        (char *) url.c_str(), config.jeedom.fingerprint);
    } // if me
  } // if host

  #endif // MOD_TELEINFO

  return ret;
}

#endif // ESP8266

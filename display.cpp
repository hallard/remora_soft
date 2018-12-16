// **********************************************************************************
// OLED display management source file for remora project
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
// **********************************************************************************

#include "./display.h"

SSD1306Wire * ssd1306 = NULL; // for SSD1306 Instance
SH1106Wire  * sh1106 = NULL;  // for SH1106 Instance
OLEDDisplay * display = NULL; // Display will point on the OLED instance
OLEDDisplayUi * ui = NULL;    // Display User Interface


#ifdef MOD_OLED
  // this array keeps function pointers to all frames
  // frames are the single views that slide from right to left
  FrameCallback frames[] = {
      drawFrameLogo,
      drawFrameTinfo,
      drawFrameWifi,
      // Frame RF is activated if MOD_RF69 is defined
      #ifdef MOD_RF69
      drawFrameRF
      #endif
  };

  /* ======================================================================
  Function: initDisplay
  Purpose : Initialise l'afficheur OLED
  Input   : -
  Output  : Etat de l'initialisation
  Comments: -
  ====================================================================== */
  bool initDisplay(void) {
    uint8_t oled_addr;

    // in case of dynamic change of OLED display
    delete ui;
    delete ssd1306;
    delete sh1106;
    ui  = NULL;
    display = NULL;
    ssd1306 = NULL;
    sh1106 = NULL;

    DebugF("config & CFG_LCD ...");
    // Clear display configuration to force init
    // and OLED detection
    config.config &= ~CFG_LCD;

    // default OLED I2C address (0x3C or 0x3D)
    oled_addr = I2C_DISPLAY_ADDRESS;

    // Scan I2C Bus to check for OLED
    if (i2c_detect(oled_addr)) {
      config.config |= CFG_LCD;
      DebugF(" SSD1306");
    } else if (i2c_detect(++oled_addr)) {
      config.config |= CFG_LCD;
      DebugF(" SH1106");
    } else {
      DebuglnF("\ni2c_scan not found display");
    }

    if (config.config & CFG_LCD) {
      DebuglnF(" ... Display found");

      // Instantiate the display
      if (config.oled_type == 1306) {
        ssd1306 = new SSD1306Wire(oled_addr, SDA, SCL);
        display = ssd1306;
      } else if (config.oled_type == 1106) {
        sh1106 = new SH1106Wire(oled_addr, SDA, SCL);
        display = sh1106;
      }

      // We got all fine
      if (display) {
        // Instantiate the User Interface
        ui = new OLEDDisplayUi( display );

        // initialize display
        display->init();
        //display->flipScreenVertically();
        display->clear();
        // Display CH2I Logo
        display->drawXbm((128-ch2i_width)/2, 0, ch2i_width, ch2i_height, ch2i_bits);
        display->display();

        display->setFont(ArialMT_Plain_10);
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setContrast(255);
        return true;
      }
    }

    return false;
  }

  /* ======================================================================
  Function: initDisplayUI
  Purpose : Initialise la bibliothèque UI pour les frames
  Input   : -
  Output  : -
  Comments: -
  ====================================================================== */
  void initDisplayUI(void) {
    if ( display && (config.config & CFG_LCD) ) {
      ui->setTargetFPS(DISPLAY_FPS);
      ui->setFrameAnimation(SLIDE_LEFT);
      ui->setFrames(frames, DISPLAY_FRAME_COUNT);
      ui->disableAllIndicators();
      ui->init();
      if (config.config & CFG_FLIP_LCD) {
        //display->flipScreenVertically();
      }
    }
  }

  /* ======================================================================
  Function: drawProgress
  Purpose : Permet de créer une barre de progression avec un texte
            au dessus et en dessous de la barre de progression
  Input   : Pointeur sur l'instance de l'afficheur
            Pourcentage de la progression
            Titre au dessus de la barre de progression
            Titre en dessous de la barre de progression
  Output  : -
  Comments: -
  ====================================================================== */
  void drawProgress(OLEDDisplay *display, int percentage, String labeltop, String labelbot) {
    if (config.config & CFG_LCD) {
      display->clear();
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->setFont(Roboto_Condensed_Bold_Bold_16);
      display->drawString(64, 8, labeltop);
      display->drawProgressBar(10, 28, 108, 12, percentage);
      display->drawString(64, 48, labelbot);
      display->display();
    }
  }

  /* ======================================================================
  Function: drawProgress
  Purpose : Permet de créer une barre de progression avec un texte
            au dessus de la barre de progression
  Input   : Pointeur sur l'instance de l'afficheur
            Pourcentage de la progression
            Titre au dessus de la barre de progression
  Output  : -
  Comments: -
  ====================================================================== */
  void drawProgress(OLEDDisplay *display, int percentage, String labeltop ) {
    if (config.config & CFG_LCD) {
      drawProgress(display, percentage, labeltop, String(""));
    }
  }

  /* ======================================================================
  Function: drawFrameWifi
  Purpose : Fonction d'affichage de l'adresse IP de la Remora
  Input   : Pointeur sur l'instance de l'afficheur
            Etat de la frame
            Coordonnées X
            Coordonnées Y
  Output  : -
  Comments: -
  ====================================================================== */
  void drawFrameWifi(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(Roboto_Condensed_Bold_Bold_16);
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    display->drawXbm(x + (128-WiFi_width)/2, y, WiFi_width, WiFi_height, WiFi_bits);
    display->drawString(x + 64, y + WiFi_height+4, WiFi.localIP().toString());
    //ui->disableIndicator();
  }

  /* ======================================================================
  Function: drawProgressBarVert
  Purpose : Fonction d'affichage d'une barre de progression verticale
  Input   : Pointeur sur l'instance de l'afficheur
            Coordonnées X du point haut de la barre
            Coordonnées Y du point gauche de la barre
            Largeur de la barre
            Hauteur de la partie centrale de la barre
            Pourcentage de la progression
  Output  : -
  Comments: -
  ====================================================================== */
  void drawProgressBarVert(OLEDDisplay *display, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) {
    uint16_t radius = width >> 1;
    uint16_t xRadius = x + radius;
    uint16_t yRadius = y + radius;
    uint16_t doubleRadius = 2 * radius;
    uint16_t innerRadius = radius - 2;

    display->setColor(WHITE);
    display->drawCircleQuads(xRadius, yRadius, radius, 0b0000011);
    display->drawVerticalLine(x, yRadius, height);
    display->drawVerticalLine(x+width, yRadius, height);
    display->drawCircleQuads(xRadius, yRadius+height-1, radius, 0b0001100);

    uint16_t maxProgress = ceil((progress * (height + doubleRadius)) / 100);
    uint16_t maxProgressHeight = ceil((progress * height) / 100);

    if (maxProgress > 0 && maxProgress <= radius) {
      //                    120         46              5
      display->fillCircle(xRadius, yRadius+height, radius-1);
    }
    else if (maxProgress > 0 && maxProgress <= height + radius) {
      display->fillCircle(xRadius, yRadius+height, radius-1);
      display->fillRect(x, (y  + height) - maxProgressHeight, width, maxProgressHeight);
    }
    else if (maxProgress > 0 && maxProgress <= height + doubleRadius) {
      display->fillCircle(xRadius, yRadius+height, radius-1);
      display->fillRect(x, (y  + height) - maxProgressHeight, width, maxProgressHeight);
      display->fillCircle(xRadius, yRadius, radius-1);
    }
  }

  /* ======================================================================
  Function: drawFrameTinfo
  Purpose : Fonction d'affichage de la téléinfo
  Input   : Pointeur sur l'instance de l'afficheur
            Etat de la frame
            Coordonnées X
            Coordonnées Y
  Output  : -
  Comments: -
  ====================================================================== */
  void drawFrameTinfo(OLEDDisplay *oled, OLEDDisplayUiState *state, int16_t x, int16_t y) {
    uint8_t percent = 0;
    char buff[20] = "";

    oled->clear();
    oled->setFont(Roboto_Condensed_Bold_Bold_16);

    if (!(status & STATUS_TINFO)) {
      oled->setTextAlignment(TEXT_ALIGN_CENTER);
      oled->setColor(INVERSE);
      oled->drawString(x + 64, 10, "Teleinfo");
      oled->drawString(x + 64, 24, "not");
      oled->drawString(x + 64, 38, "initialized");
    }
    else {
      // Effacer le buffer de l'affichage
      oled->setTextAlignment(TEXT_ALIGN_LEFT);
      //oled->setFont(Roboto_Condensed_Bold_14);
      oled->setFont(ArialMT_Plain_10);
      oled->setColor(WHITE);

      // si en heure pleine inverser le texte sur le compteur HP
      //if (ptec == PTEC_HP )
        //oled->setColor(BLACK); // 'inverted' text

      //uint16_t lenLabel = oled->getStringWidth("Pleines ");
      sprintf_P(buff, PSTR("Pleines %09ld"), myindexHP);
      oled->drawString(x + 0, y + 0, buff);
      //oled->setColor(WHITE); // normaltext

      // si en heure creuse inverser le texte sur le compteur HC
      //if (ptec == PTEC_HC )
        //oled->setColor(INVERSE); // 'inverted' text

      //oled->drawString(x + 0, 14 + y, "Creuses ");
      memset(buff, 0, 20);
      sprintf_P(buff, PSTR("Creuses %09ld"), myindexHC);
      oled->drawString(x + 0, 14 + y, buff);
      oled->setColor(WHITE); // normaltext

      // Poucentrage de la puissance totale
      percent = (uint) myiInst * 100 / myisousc;

      //Serial.print("myiInst="); Serial.print(myiInst);
      //Serial.print("  myisousc="); Serial.print(myisousc);
      //Serial.print("  percent="); Serial.println(percent);

      // Information additionelles
      memset(buff, 0, 20);
      sprintf_P(buff, PSTR("%d W %d%%  %3d A"), mypApp, percent, myiInst);
      oled->drawString(x + 0, 28 + y, buff);

      // etat des fils pilotes
      // On transcrit l'état de fonctionnement du relais en une lettre
      // S: arrêt, F: marche forcée, A: auto
      char dFnctRelais = 'A';
      if (fnctRelais == FNCT_RELAIS_ARRET) {
        dFnctRelais = 'S';
      } else if (fnctRelais == FNCT_RELAIS_FORCE) {
        dFnctRelais = 'F';
      }
      memset(buff, 0, 20);
      sprintf_P(buff, PSTR("%s %c%c"), etatFP, dFnctRelais, etatrelais+'0');
      oled->drawString(x + 0, 48 + y, buff);

      // Bargraphe de puissance
      drawProgressBarVert(oled, 114, 6, 12, 40, percent);
    }
  }

  /* ======================================================================
  Function: drawFrameLogo
  Purpose : Fonction d'affichage du logo de la Remora
  Input   : Pointeur sur l'instance de l'afficheur
            Etat de la frame
            Coordonnées X
            Coordonnées Y
  Output  : -
  Comments: -
  ====================================================================== */
  void drawFrameLogo(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->clear();
    display->drawXbm(x + (128-remora_width)/2, y, remora_width, remora_height, remora_bits);
    //ui->disableIndicator();
  }

  /* ======================================================================
  Function: drawFrameRF
  Purpose : Fonction d'affichage des données radio
  Input   : Pointeur sur l'instance de l'afficheur
            Etat de la frame
            Coordonnées X
            Coordonnées Y
  Output  : -
  Comments: -
  ====================================================================== */
  void drawFrameRF(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
      char buff[32];
      int16_t percent;
      volatile uint8_t * p;
      int8_t line = 0;
      byte n;

      n = rfData.size;
      p = rfData.buffer;

      display->clear();
      display->setFont(Roboto_Condensed_Bold_Bold_16);

      if (!(status & STATUS_RFM)) {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->drawString(x + 64, 10, "Radio");
        display->drawString(x + 64, 24, "not");
        display->drawString(x + 64, 38, "initialized");
      }
      else if (!got_first) {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->drawString(x + 64, 14, "No radio data");
        display->drawString(x + 64, 34, "received yet");
      } else {

        display->setTextAlignment(TEXT_ALIGN_LEFT);
        if (rfData.type == RF_MOD_RFM69) {
          sprintf_P(buff, PSTR("NODE %d"), rfData.nodeid );
          display->drawString(x + 0, 0, buff);

          // rssi
          percent = (int16_t) rfData.rssi;

          // rssi for RFM69 is -115 (0%) to 0 (100%)
    //      if (driver) {
            // rssi range now 0 (0%) to 115 (100%)
            percent += 115;
            // now calc percentage
            percent = (percent * 100) / 115;
    //      }

          if (percent > 100) percent = 100;
          if (percent < 0  ) percent = 0;

          // display bargraph on lcd
          display->drawProgressBar( x + 62, 4, 64 , 12, percent);

          //display->setFont(Roboto_Condensed_12);
          display->setTextAlignment(TEXT_ALIGN_LEFT);
          /*
          line++; *buff='\0';
          if (sensorData.temp != SENSOR_NOT_A_TEMP) {
            sprintf_P(buff+strlen(buff), PSTR("%.1f°C  "), sensorData.temp/100.0f );
          }
          if (sensorData.bat != SENSOR_NOT_A_BAT) {
            //sprintf_P(buff+strlen(buff), PSTR("%.2fV"), sensorData.bat/1000.0f );
            display->drawXbm(x + 127 - bat_width, y + 20, bat_width, bat_height,
                              sensorData.bat>1500?bat_100_bits:
                              sensorData.bat>1400?bat_090_bits:
                              sensorData.bat>1300?bat_080_bits:
                              sensorData.bat>1200?bat_070_bits:
                              sensorData.bat>1100?bat_060_bits:
                              sensorData.bat>1000?bat_050_bits:
                              sensorData.bat> 900?bat_040_bits:
                              sensorData.bat> 800?bat_030_bits:
                              sensorData.bat> 700?bat_020_bits:
                              sensorData.bat> 600?bat_010_bits:bat_000_bits
                              );
          }
          display->drawString(x + 0, y + line * 16, buff);

          display->setTextAlignment(TEXT_ALIGN_CENTER);
          line++; *buff='\0';
          if (sensorData.lux != SENSOR_NOT_A_LUX) {
            sprintf_P(buff+strlen(buff), PSTR("%.0f Lux  "), sensorData.lux/10.0f );
          }
          if (sensorData.hum != SENSOR_NOT_A_HUM) {
            sprintf_P(buff+strlen(buff), PSTR("%.0f %%RH"), sensorData.hum/10.0f );
          }*/
          display->drawString(x + 64, y + line * 16, buff);

          display->setFont(Roboto_Condensed_12);
          display->setTextAlignment(TEXT_ALIGN_CENTER);
          display->drawString(x + 64, 48, timeAgo(uptime-packet_last_seen));
        }
      }
    //}

    //ui->disableIndicator();
  }

#endif // #ifdef MOD_OLED

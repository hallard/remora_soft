// **********************************************************************************
// Programmateur Fil Pilote et Suivi Conso
// **********************************************************************************
// Copyright (C) 2014 Thibault Ducret
// Licence MIT
//
// History : 15/01/2015 Charles-Henri Hallard (http://hallard.me)
//                      Intégration de version 1.2 de la carte electronique
//           15/09/2015 Charles-Henri Hallard : Ajout compatibilité ESP8266
//
// **********************************************************************************

#include "pilotes.h"

#if (NB_FILS_PILOTES==7)
  int SortiesFP[NB_FILS_PILOTES*2] = { FP1,FP2,FP3,FP4,FP5,FP6,FP7 };
#elif (NB_FILS_PILOTES==6)
  int SortiesFP[NB_FILS_PILOTES*2] = { FP1,FP2,FP3,FP4,FP5,FP6 };
#else
  #error "Définition du nombre de fils pilotes inccorect"
#endif

char etatFP[NB_FILS_PILOTES+1] = "";
char memFP[NB_FILS_PILOTES+1] = ""; //Commandes des fils pilotes mémorisées (utile pour le délestage/relestage)
int nivDelest = 0; // Niveau de délestage actuel (par défaut = 0, pas de délestage)
// Correspond au nombre de fils pilotes délestés (entre 0 et nombre de zones)
uint8_t plusAncienneZoneDelestee = 1;
// Numéro de la zone qui est délestée depuis le plus de temps (entre 1 et nombre de zones)
// C'est la première zone à être délestée
unsigned long timerDelestRelest = 0; // Timer de délestage/relestage

// Instanciation de l'I/O expander
Adafruit_MCP23017 mcp;


/* ======================================================================
Function: setfp
Purpose : selectionne le mode d'un des fils pilotes
Input   : commande numéro du fil pilote + commande optionelle
          C=Confort, A=Arrêt, E=Eco, H=Hors gel, 1=Eco-1, 2=Eco-2
          ex: 1A => FP1 Arrêt
              41 => FP4 eco -1 (To DO)
              6C => FP6 confort
              72 => FP7 eco -2 (To DO)
          Si la commande est absente la fonction retourne l'état du FP
          ex: 1 => si état FP1 est "arret" retourne code ASCII du "A" (65)
Output  : 0 ou etat commande, si ok -1 sinon
Comments: exposée par l'API spark donc attaquable par requête HTTP(S)
====================================================================== */
int setfp(String command)
{
  command.trim();
  command.toUpperCase();

  Debug("setfp=");
  Debugln(command);

  int returnValue = -1;

  // Vérifier que l'on demande l'état d'un seul fil pilote
  if (command.length() == 1)
  {
    uint8_t fp = command[0]-'0';

    // demande valide
    // retourner l'état du fil pilote (char)
    if (fp >= 1 && fp <= NB_FILS_PILOTES)
      returnValue = (etatFP[fp-1])  ;
  }
  else if (command.length() == 2)
  {
    // numéro du fil pilote concerné, avec conversion ASCII > entier
    // la commande est vérifiée dans fpC, pas besoin de traiter ici
    uint8_t fp = command[0]-'0';
    char cOrdre= command[1];
    if ( (fp < 1 || fp > NB_FILS_PILOTES) ||
        (cOrdre!='C' && cOrdre!='E' && cOrdre!='H' && cOrdre!='A') )
    {
        // erreur
        Debugln("Argument incorrect");
    }
    else
    {
      memFP[fp-1] = cOrdre; // On mémorise toujours la commande demandée
      char cOrdreEnCours = etatFP[fp-1]; // Quel est l'état actuel du fil pilote?
      if (cOrdreEnCours != 'D')
      {
        // Si un délestage est en cours sur ce fil pilote, on n'exécute pas la commande
        // La commande est néanmoins mémorisé
        // Elle sera appliquée lors du relestage
        returnValue = setfp_interne(fp, cOrdre);
      }
    }
  }
  return(returnValue);
}

/* ======================================================================
Function: setfp_interne
Purpose : selectionne le mode d'un des fils pilotes
Input   : numéro du fil pilote (1 à NB_FILS_PILOTE)
          ordre à appliquer
          C=Confort, A=Arrêt, E=Eco, H=Hors gel, 1=Eco-1, 2=Eco-2, D=Délestage
          ex: 1,'A' => FP1 Arrêt
              4,'1' => FP4 eco -1 (To DO)
              6,'C' => FP6 confort
              7,'2' => FP7 eco -2 (To DO)
              5,'D' => FP5 délestage (=> hors-gel et blocage des nouvelles commandes)
Output  : 0 si ok -1 sinon
Comments: non exposée par l'API spark car on y gère le délestage
====================================================================== */
int setfp_interne(uint8_t fp, char cOrdre)
{
  // Vérifier que le numéro du fil pilote ne dépasse le MAX et
  // que la commande est correcte
  // Pour le moment les ordres Eco-1 et Eco-2 ne sont pas traités
  // 'D' correspond à délestage

  Debug("setfp_interne : fp=");
  Debug(fp);
  Debug(" ; cOrdre=");
  Debugln(cOrdre);

  if ( (fp < 1 || fp > NB_FILS_PILOTES) ||
      (cOrdre!='C' && cOrdre!='E' && cOrdre!='H' && cOrdre!='A' && cOrdre!='D') )
  {
      // erreur
      return (-1);
  }
  // Ok ici tout est correct
  else
  {
    // Commande à passer
    uint8_t fpcmd1, fpcmd2;

    // tableau d'index de 0 à 6 pas de 1 à 7
    // on en profite pour Sauver l'état
    etatFP[fp-1]=cOrdre;
    Debug("etatFP=");
    Debugln(etatFP);

    switch (cOrdre)
    {
        // Confort => Commande 0/0
        case 'C': fpcmd1=LOW;  fpcmd2=LOW;  break;
        // Eco => Commande 1/1
        case 'E': fpcmd1=HIGH; fpcmd2=HIGH; break;
        // Hors gel => Commande 1/0
        case 'H': fpcmd1=HIGH; fpcmd2=LOW;  break;
        // Arrêt => Commande 0/1
        case 'A': fpcmd1=LOW;  fpcmd2=HIGH; break;
        // Eco - 1
        case '1': { /* to DO */ } ; break;
        // Eco - 2
        case '2': { /* to DO */ }; break;
        // Délestage => Hors gel => Commande 1/0
        case 'D': fpcmd1=HIGH; fpcmd2=LOW;  break;
    }

    // On positionne les sorties physiquement
    _digitalWrite(SortiesFP[2*(fp-1)], fpcmd1);
    _digitalWrite(SortiesFP[2*(fp-1)+1], fpcmd2);
    return (0);
  }
}

/* ======================================================================
Function: initFP
Purpose : met tous les fils pilotes en mode hors-gel
Input   : -
Output  : -
Comments: -
====================================================================== */
void initFP(void)
{
  // buffer contenant la commande à passer à setFP
  char cmd[] = "xH" ;

  // On positionne tous les FP en Hors-Gel
  for (uint8_t i=1; i<=NB_FILS_PILOTES; i+=1)
  {
    cmd[0]='0' + i;
    setfp(cmd);

    // Feed the dog
    _wdt_feed();
  }
}

/* ======================================================================
Function: delester1zone
Purpose : déleste une zone de plus
Input   : variables globales nivDelest et plusAncienneZoneDelestee
Output  : màj variable globale nivDelest
Comments: -
====================================================================== */
void delester1zone(void)
{
  uint8_t numFp; // numéro du fil pilote à délester

  Debug("delester1zone() : avant : nivDelest=");
  Debug(nivDelest);
  Debug(" ; plusAncienneZoneDelestee=");
  Debugln(plusAncienneZoneDelestee);

  if (nivDelest < NB_FILS_PILOTES) // On s'assure que l'on n'est pas au niveau max
  {
    nivDelest += 1;
    numFp = ((plusAncienneZoneDelestee-1 + nivDelest-1) % NB_FILS_PILOTES)+1;
    setfp_interne(numFp, 'D');
  }

  Debug("delester1zone() : apres : nivDelest=");
  Debug(nivDelest);
  Debug(" ; plusAncienneZoneDelestee=");
  Debugln(plusAncienneZoneDelestee);
}

/* ======================================================================
Function: relester1zone
Purpose : retire le délestage d'une zone
Input   : variables globales nivDelest et plusAncienneZoneDelestee
Output  : màj variable globale nivDelest et plusAncienneZoneDelestee
Comments: -
====================================================================== */
void relester1zone(void)
{
  uint8_t numFp; // numéro du fil pilote à passer HORS-GEL

  Debug("relester1zone() : avant : nivDelest=");
  Debug(nivDelest);
  Debug(" ; plusAncienneZoneDelestee=");
  Debugln(plusAncienneZoneDelestee);

  if (nivDelest > 0) // On s'assure qu'un délestage est en cours
  {
    nivDelest -= 1;
    numFp = plusAncienneZoneDelestee;
    char cOrdreMemorise = memFP[numFp-1]; //On récupére la dernière valeur de commande pour cette zone
    setfp_interne(numFp,cOrdreMemorise);
    plusAncienneZoneDelestee = (plusAncienneZoneDelestee % NB_FILS_PILOTES) + 1;
  }

  Debug("relester1zone() : apres : nivDelest=");
  Debug(nivDelest);
  Debug(" ; plusAncienneZoneDelestee=");
  Debugln(plusAncienneZoneDelestee);
}

/* ======================================================================
Function: decalerDelestage
Purpose : fait tourner la ou les zones délestées
Input   : variables globales nivDelest et plusAncienneZoneDelestee
Output  : màj variable globale plusAncienneZoneDelestee
Comments: -
====================================================================== */
void decalerDelestage(void)
{
  Debug("decalerDelestage() : avant : nivDelest=");
  Debug(nivDelest);
  Debug(" ; plusAncienneZoneDelestee=");
  Debugln(plusAncienneZoneDelestee);

  if (nivDelest > 0 && nivDelest < NB_FILS_PILOTES)
  // On ne peut pas faire tourner les zones délestées s'il n'y en a aucune en cours
  // de délestage, ou si elles le sont toutes
  {
    relester1zone();
    delester1zone();
  }

  Debug("decalerDelestage() : apres : nivDelest=");
  Debug(nivDelest);
  Debug(" ; plusAncienneZoneDelestee=");
  Debugln(plusAncienneZoneDelestee);
}

/* ======================================================================
Function: fp
Purpose : selectionne le mode d'un ou plusieurs les fils pilotes d'un coup
Input   : liste des commandes
          -=rien, C=Confort, A=Arrêt, E=Eco, H=Hors gel, 1=Eco-1, 2=Eco-2,
          ex: 1A => FP1 Arrêt
              CCCCCCC => Commande tous les fils pilote en mode confort (ON)
              AAAAAAA => Commande tous les fils pilote en mode arrêt
              EEEEEEE => Commande tous les fils pilote en mode éco
              CAAAAAA => Tous OFF sauf le fil pilote 1 en confort
              A-AAAAA => Tous OFF sauf le fil pilote 2 inchangé
              E-CHA12 => FP2 Eco  , FP2 inchangé, FP3 confort, FP4 hors gel
                        FP5 arrêt, FP6 Eco-1    , FP7 Eco-2
Output  : 0 si ok -1 sinon
Comments: exposée par l'API spark donc attaquable par requête HTTP(S)
====================================================================== */
int fp(String command)
{
  command.trim();
  command.toUpperCase();

  Debug("fp=");
  Debugln(command);


  // Vérifier que l'on a la commande de tous les fils pilotes
  if (command.length() != NB_FILS_PILOTES)
  {
      return(-1) ;
  }
  else
  {
    int8_t returnValue = 0; // Init à 0 => OK
    char   cmd[] = "xx" ; // buffer contenant la commande à passer à setFP

    // envoyer les commandes pour tous les fils pilotes
    for (uint8_t i=1; i<=NB_FILS_PILOTES; i++)
    {
      cmd[0] = '0' + i ;
      cmd[1] = command[i-1]; // l'index de la chaine commence à 0 donc i-1

      // Si on ne doit pas laisser le fil pilote inchangé
      if (cmd[1] != '-' )
      {
        // ok ici au cas ou la commande setFP n'est pas bonne
        // on positionne le code de retour à -1 mais on
        // continue le traitement, les suivantes sont
        // peut-être correctes
        if (setfp(cmd) == -1)
          returnValue = -1;
      }

      // Feed the dog
      _wdt_feed();
    }
    return returnValue;
  }
}

/* ======================================================================
Function: relais
Purpose : selectionne l'état du relais
Input   : état du relais (0 ouvert, 1 fermé)
Output  : etat du relais (0 ou 1)
Comments: exposée par l'API spark donc attaquable par requête HTTP(S)
====================================================================== */
int relais(String command)
{
  command.trim();
  uint8_t cmd = command.toInt();

  DebugF("relais=");
  Debugln(command);
  Debugflush();

  // Vérifier que l'on a la commande d'un seul caractère
  if (command.length()!=1 || cmd < 0 || cmd > 1)
    return (-1);

  // Conversion en 0,1 numerique
  etatrelais = cmd;
  int etatRelaisPin = cmd;

  // Inverse etat pin relais si definit dans remora.h
  #ifdef RELAIS_REVERSE
    etatRelaisPin = !etatRelaisPin;
  #endif

    // Allumer/Etteindre le relais et la LED
  #ifdef RELAIS_PIN
    _digitalWrite(RELAIS_PIN, etatRelaisPin);
  #endif
  #ifdef LED_PIN
    _digitalWrite(LED_PIN, etatrelais);
  #endif

  return (etatrelais);
}

/* ======================================================================
Function: relais
Purpose : selectionne l'état du relais
Input   : état du relais (0 ouvert, 1 fermé)
Output  : etat du relais (0 ou 1)
Comments: exposée par l'API spark donc attaquable par requête HTTP(S)
====================================================================== */
int fnct_relais(String command)
{
  command.trim();
  uint8_t cmd = command.toInt();

  Debug("fnct_relais="); Debugln(command);
  Debug("command length="); Debugln(command.length());
  Debugf("cmd: %d\n", cmd);
  //Debugflush();

  // Vérifier que l'on a la commande d'un seul caractère
  if (command.length() != 1 || cmd < 0 || cmd > 2)
    return (-1);

  // On si il y a modification du fonctionnement du relais
  if (fnctRelais != cmd) {
    // Conversion en 0,1,2 numerique
    fnctRelais = cmd;
    // Si le mode est différent du mode auto, on applique la commande
    if (fnctRelais < FNCT_RELAIS_AUTO) {
      relais(command);
    }
    #ifdef MOD_TELEINFO
      else {
        ValueList * me = tinfo.getList();
        bool found = false;   // flag for PTEC found
        // Got at least one ?
        if (me) {
          // Loop thru the node
          while (me->next) {
            // go to next node
            me = me->next;
            // we're there
            _wdt_feed();
            //DebugF("me->name: "); Debug(me->name); DebugF(" - value: "); Debugln(me->value);
            // Check PTEC label
            if (me->name && !strcmp(me->name, "PTEC")) {
              //DebuglnF("PTEC found");
              // If "heures creuses", close relay
              if (me->value && !strcmp(me->value, "HC..")) {
                //DebuglnF("PTEC == HC..");
                relais("1");
              } else {
                relais("0");
              }
              found = true;
              break;
            }
          }
          // If PTEC not found, close relay
          if (!found) {
            relais("0");
          }
        }
      }
    #endif
  }

  return (fnctRelais);
}

/* ======================================================================
Function: pilotes_Setup
Purpose : prepare and init stuff, configuration, ..
Input   : -
Output  : true if MCP23017 module found, false otherwise
Comments: -
====================================================================== */
bool pilotes_setup(void)
{
  // Cartes Version 1.0 et 1.1 pilotage part port I/O du spark
  #if defined (REMORA_BOARD_V10) || defined (REMORA_BOARD_V11)

    // 2*nbFilPilotes car 2 pins pour commander 1 fil pilote
    for (uint8_t i=0; i < (NB_FILS_PILOTES*2); i++)
      _pinMode(SortiesFP[i], OUTPUT); // Chaque commande de fil pilote est une sortie

  // Cartes Version 1.2+ pilotage part I/O Expander
  #else
    Debug("Initializing MCP23017...Searching...");
    Debugflush();

    // Détection du MCP23017
    if (!i2c_detect(MCP23017_ADDRESS))
    {
      Debugln("Not found!");
      Debugflush();
      return (false);
    }
    else
    {
      Debug("Setup...");
      Debugflush();

      // et l'initialiser
      mcp.begin();

      // Mettre les 16 I/O PIN en sortie
      mcp.writeRegister(MCP23017_IODIRA,0x00);
      mcp.writeRegister(MCP23017_IODIRB,0x00);
      Debugln("OK!");
      Debugflush();
    }
  #endif

  // ou l'a trouvé
  return (true);
}

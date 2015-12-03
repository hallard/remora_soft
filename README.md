Version Logicielle pour toutes les versions du matériel
=======================================================

Cette version logicielle est compatible avec la version matérielle [1.2][1] ainsi que les anciennes versions des cartes electroniques. Elle est aussi compatible avec les cartes à base d'ESP8266 via un [adaptateur][5]

Installation (Particle)
-----------------------

- Télécharger l'environnement de développement [Particle-Dev][2] puis lancer l'IDE.
- Télécharger l'archive du repo logiciel [github][3] et le copier sur votre disque dur puis le décompresser
- Une fois l'IDE ouvert, menu File/Open et ouvrir le dossier programmateur-fil-pilote-wifi-master\Logiciel\remora
- Dans le menu Particle/Log in to Particle Cloud, entrez vos identifiants Particle pour vous connecter
- Dans le menu Particle/Select Device selectionnez votre Spark Core ou Particle Photon
- Ouvrir ensuite depuis l'IDE les fichers remora.ino et remora.h
- Selectionner la version de carte utilisé dans le fichier remora.h (les defines REMORA_BOARD_Vxx)
- Selectionner les modules utilisés dans le fichier remora.h (les defines MOD_xxxx)
- Selectionner l'onglet remora.ino
- Lancer la compilation+upload (icone éclair en haut à gauche)
- Si vous avez une erreur de type "App code was invalid" durant la compilation, supprimer le ficher README.md du dossier

Installation (Arduino ESP8266)
------------------------------

- Suivre la procédure détaillée [ici](https://github.com/esp8266/Arduino), Attention Arduino 1.6.5 pas 1.6.6
- Ouvrir ensuite depuis l'IDE le ficher `remora_soft.ino`
- Selectionner la version de carte utilisé dans le fichier remora.h (les defines REMORA_BOARD_Vxx)
- Selectionner les modules utilisés dans le fichier remora.h (les defines MOD_xxxx)
- choisir la carte NodeMCU 1.0 (ESP12E-Module) ainsi que le bon serial port
- mettre votre SSID et mot de passe Wifi dans le fichier remora.h
````
		// Définir ici les identifiants de
		// connexion à votre réseau Wifi
		// =====================================
		#define DEFAULT_WIFI_SSID "VotreSSID"
		#define DEFAULT_WIFI_PASS "VotreClé"
		#define DEFAULT_WIFI_AP_PASS "Remora_WiFi"
		// =====================================		
````
- Lancer la compilation + upload 
- La procédure OTA sera détaillée ultérieurement


API Exposée (Particle)
----------------------

Toutes les API se font via des requêtes HTTP sur le cloud Particle. 

A documenter!


API Exposée (ESP8266 NodeMCU)
-----------------------------

Toutes les API se font via des requêtes HTTP sur le Remora. Il existe deux formats possibles si l'on veut récupérer des données ou exécuter des action avec le Remora. Chaque requête se verra retourner des données (ou un code de bonne éxécution) au format JSON.

Toute requête sera donc adressée sous la forme     
`http://adresse_ip_du_remora/requete_plus_ou_moins_longue` dans les exemples ci dessous l'adresse IP de mon Remora est la 192.168.1.201, veillez à bien la changer pour mettre la vôtre. Les exemples ont été exécutés depuis la ligne de commande avec curl mais ils pourraient l'être depuis la barre d'addresse de votre navigateur.

**Les Etats de fil pilote**

Les différents états possibles de fil pilote dans l'API correspondent à la notation suivante, une lettre représente l'état lu ou le mode à positionner tel que :
```
C = Confort
A = Arrêt
E = Eco
H = Hors gel
1 = Eco-1 (non géré pour le moment)
2 = Eco-2 (non géré pour le moment)
```

**Les API d'intérrogation**

Les API d'intérrogation se presentent sous la forme 
`http://adresse_ip_du_remora/ma_donnee` et la/les donnée(s) sont retournées au format JSON (j'ai volontairement supprimé certains sauts de lignes de sortie pour une meilleure lecture)

- Etat du relais `http://ip_du_remora/relais`
````shell
    ~ # curl http://192.168.1.201/relais
    { "relais": 0 }
````
- Etat du délestage `http://ip_du_remora/delestage`
````shell
		~ # curl http://192.168.1.201/delestage
		{ "niveau": 0, "zone": 1 }
````
- Etat d'un fil pilote `http://ip_du_remora/fpn` avec n = numéro du fil pilote (1 à 7)
````shell
		~ # curl http://192.168.1.201/fp3
		{ "fp3": "E" }
````
- Etat de tous les fils pilotes **http://ip_du_remora/fp**
````shell
		~ # curl http://192.168.1.201/fp
		{
		"fp1": "A",
		"fp2": "E",
		"fp3": "E",
		"fp4": "E",
		"fp5": "C",
		"fp6": "H",
		"fp7": "C"
		}
````
- Récupérer une étiquette Téléinfo par non nom `http://ip_du_remora/Nom_Etiquette`
````shell
		~ # curl http://192.168.1.201/PAPP
		{ "PAPP": 170 }
		~ # curl http://192.168.1.201/IINST
		{ "IINST": 1 }
		~ # curl http://192.168.1.201/PTEC
		{ "PTEC":"HC.." }
````
- Récupérer toutes les étiquettes Téléinfo en une fois `http://ip_du_remora/tinfo`
````shell
		~ # curl http://192.168.1.201/tinfo
		{ "_UPTIME":1614,
			"ADCO":31428067147,
			"OPTARIF":"HC..",
			"ISOUSC":15,
			"HCHC":410994,
			"HCHP":0,
			"PTEC":"HC..",
			"IINST":1,
			"IMAX":1,
			"PAPP":170,
			"HHPHC":3,
			"MOTDETAT":0 
		}
````
A noter la présence de certaines étiquettes virtuelles commencant par un `_`


**Les API d'action**

Les API d'action se presentent sous la forme 
`http://adresse_ip_du_remora/?action=ma_donnee`, notez la différence avec les intérrogations, le `?`. Le résultat est retourné au format JSON avec un code réponse, il est :     
- négatif en cas d'erreur
- à 0 si tout est OK
- positif pour indiquer un code retour OK différent si besoin.

Note, il est possible d'enchainer les actions en une requête mais un seul code d'erreur sera retourné pour l'ensemble, si une des commandes échoue, il faudra intérroger afin de savoir laquelle n'a pas fonctionnée.

- Activer le relais `http://ip_du_remora/?relais=1`
````shell
		# curl http://192.168.1.201/?relais=1
		{ "response": 0 }
````
- désactiver le relais `http://ip_du_remora/?relais=0`
````shell
		# curl http://192.168.1.201/?relais=0
		{ "response": 0 }
````
- Exemple d'erreur avec le relais `http://ip_du_remora/?relais=3`
````shell
		# curl http://192.168.1.201/?relais=3
		{ "response": -1 }
````
- selectionne le mode d'un des fils pilotes `http://ip_du_remora/?setfp=na` avec n=numéro du fil pilote et a=le mode à positionner (non sensible à la casse)    
  Fil pilote 1 en arret
````shell
		# curl http://192.168.1.201?setfp=1a
		{ "response": 0 }
````
  Fil pilote 7 en Eco
````shell
		# curl http://192.168.1.201?setfp=7E
		{ "response": 0 }
````
  Mauvaise commande
````shell
		curl http://192.168.1.201?setfp=5X
		{ "response": -1 }
````
- Selectionne le mode d'un ou plusieurs les fils pilotes d'un coup `http://ip_du_remora/?fp=CMD` avec 7 commandes de fil pilote.
  Tous les fils pilote en confort
````shell
		curl http://192.168.1.201/?fp=CCCCCCC
		{ "response": 0 }
````
  Tous les fils pilote en arret
````shell
		curl http://192.168.1.201/?fp=AAAAAAA
		{ "response": 0 }
````
  Tous les fils pilote en eco
````shell
		curl http://192.168.1.201/?fp=EEEEEEE
		{ "response": 0 }
		~ #
````
  Tous OFF sauf le fil pilote 1 en confort
````shell
		curl http://192.168.1.201/?fp=CAAAAAA
		{ "response": 0 }
		~ #
````
  Tous OFF sauf le fil pilote 2 A-AAAAA
````shell
		curl http://192.168.1.201/?fp=CCCCCCC
		{ "response": 0 }
		~ #
````
  FP1 Eco, FP2 inchangé, FP3 confort, FP4 hors gel, FP5 arrêt, FP6 Eco-1, FP7 Eco-2
````shell
		curl http://192.168.1.201/?fp=E-CHA12
		{ "response": -1 }
		~ #
````
Erreur car les modes ECO-1 et ECO-1 ne sont pas gérés pour le moment.

A faire
-------

- Mettre des icones plus sympas sur l'afficheur
- Gérer les retour de sondes RF, pour le moment seules les trames sont affichées sur la serial, rien n'est encore fait
- Gérer le RF des sondes Oregon Scientific
- pour le moment seul les [OLED][4] I2C sont gérés
- tout autre idée est bienvenue

Historiques des Modifications
-----------------------------
16/02/2015 : Ajout délestage cascadocyclique / Possibilité de ne récupérer l'état que d'un seul fil pilote

14/04/2015 : Ajout d'une variable spark pour la teleinfo. Passage en un seul appel pour script jeedom. Les variables d'origine restent utilisables.

30/09/2015 : voir le post [dédié][6].   

02/12/2015 : Ajout [Version 1.3][7] Remora NodeMCU


Exemple
-------

Pour les photos la téléinfo n'était pas branchée sur la carte, c'est pour celà que les compteurs et le bargraphe sont à 0

<img src="https://github.com/thibdct/programmateur-fil-pilote-wifi/blob/master/Logiciel/images/remora01.jpg" alt="Top"    width="45%" height="45%">&nbsp;
<img src="https://github.com/thibdct/programmateur-fil-pilote-wifi/blob/master/Logiciel/images/remora02.jpg" alt="Bottom" width="45%" height="45%">
<br>

Avec la téléinfo branchée sur la carte, le bargraph indique la puissance instantanée par rapport au contrat souscrit, ici soit 11% car on consomme 5A sur un contrat de 45A. Nous sommes en heures pleines (affichage sur fond blanc)
<img align="center" src="https://github.com/thibdct/programmateur-fil-pilote-wifi/blob/master/Logiciel/images/remora03.jpg" alt="Bottom" width="45%" height="45%">


[1]: https://github.com/thibdct/programmateur-fil-pilote-wifi/tree/master/Mat%C3%A9riel/1.2
[2]: https://www.particle.io/dev
[3]: https://github.com/thibdct/programmateur-fil-pilote-wifi/archive/master.zip
[4]: http://www.ebay.com/itm/291216700457
[5]: https://github.com/hallard/Particle2NodeMCU/blob/master/README.md
[6]: https://community.hallard.me/topic/92/nouvelle-version-remora-compatible-esp8266
[7]: https://github.com/thibdct/programmateur-fil-pilote-wifi/tree/master/Mat%C3%A9riel/1.3

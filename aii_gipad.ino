// Ces deux bibliothèques sont indispensables pour le shield
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include "Adafruit_DRV2605.h"

unsigned long currentMillis;
int i;
//---------- init requeteHTTP ----------
// L'adresse MAC du shield
byte mac[] = {0x90, 0xA2 , 0xDA, 0x10, 0x7A, 0x45};
// L'adresse IP que prendra le shield
IPAddress ip(192, 168, 137, 1);
// L'objet qui nous servira a la communication
EthernetClient client;
// Le serveur à interroger
char serveur[] = "172.17.3.140";

// Pour lire les caractères
char carlu = 0;
String strlu;
String temps;
int freqResp, urine, pressionA;
// Moment de la dernière requête
long derniereRequete = 0;
// Temps minimum entre deux requêtes
const long updateInterval = 2000;
// Mémorise l'état de la connexion entre deux tours de loop
bool etaitConnecte = false;

//---------- init coeur ----------------------
  unsigned long previousMillisCoeur = 0 ;
  const uint8_t effect = 2;
  int lance = 0;
  long ecart_battement = 300; // Equivalent à 200 battements minutes
  
  long ecart_battement2 = 100;
  Adafruit_DRV2605 drv;

//---------- init urine ------------------------
unsigned long previousMillisSon = 0;
const int speakerPin = 12;
const int ton = 311;
const long duree_son = 500;
int sound = 0;

//----------init moteur--------------------------
#define Ain1 2
#define Ain2 3
int vit = 255; //A choisir entre 65-255*/


// ============================ SETUP ==================================================
void setup() {
  Serial.begin(9600);

  //------------ setup requete HTTP ----------------
  char erreur = 0;
  // On démarre le shield Ethernet SANS adresse IP (donc donnée via DHCP)
  erreur = Ethernet.begin(mac);
  Serial.print(erreur);

  if (erreur == 0) {
    Serial.println("Parametrage avec ip fixe...");
    // si une erreur a eu lieu cela signifie que l'attribution DHCP
    // ne fonctionne pas. On initialise donc en forçant une IP
    Ethernet.begin(mac, ip);
  }
  Serial.println("Init...");
  // Donne une seconde au shield pour s'initialiser
  delay(1000);
  Serial.println("Pret !");

   //------------ setup coeur ----------------
    drv.begin(); // Demarrage du driver du coeur
    drv.selectLibrary(1);
    drv.setMode(DRV2605_MODE_INTTRIG);
    drv.setWaveform(0, effect);  // play effect
    drv.setWaveform(1, 0);       // end waveform

  //------------ setup poumon ----------------
  pinMode(Ain1, OUTPUT);  //Ain1
  pinMode(Ain2, OUTPUT);  //Ain2
}

// ================================= MAIN LOOP ==========================================
void loop()
{
  currentMillis = millis();

  // ------------------- Requete HTTP et lecture des valeurs ---------------
  // on lit les caractères s'il y en a de disponibles
  if (client.available()) {
    carlu = client.read();
    strlu += carlu;
    //Serial.print(carlu);
  }

  // SI on était connecté au tour précédent
  // ET que maintenant on est plus connecté
  // ALORS on ferme la connexion
  if (etaitConnecte && !client.connected()) {

    // Récupération des valeurs recues par la base de données
    strlu = recupVal(strlu);
    Serial.println(strlu);

    temps = supFinVirg(strlu);
    Serial.print("Date : ");
    Serial.println(temps);

    strlu = supDebVirg(strlu);

    freqResp = supFinVirg(strlu).toInt();
    Serial.print("Frequence respiratoire: ");
    Serial.println(freqResp);

    strlu = supDebVirg(strlu);

    urine = supFinVirg(strlu).toInt();
    Serial.print("Urine: ");
    Serial.println(urine);
    i = 1;

    pressionA = strlu.substring(strlu.indexOf(",") + 1).toInt();
    Serial.print("Pression Arterielle: ");
    Serial.println(pressionA);

    Serial.println();
    Serial.println("Deconnexion !");
    Serial.println();
    // On ferme le client
    client.stop();
  }

  // Si on est déconnecté
  // et que cela fait plus de xx secondes qu'on a pas fait de requête
  if (!client.connected() && ((millis() - derniereRequete) > updateInterval)) {
    requete();
  }
  // enregistre l'état de la connexion (ouvert ou fermé)
  etaitConnecte = client.connected();

  // ------------------ Controle poumon ----------------------------------------
  analogWrite(Ain1, vit);

  // ------------------ Controle son ----------------------------------------

  if (urine == 1 && i == 1) {
    Serial.println("On allume le son");
    sound = 1;
    i = 0;
    previousMillisSon = currentMillis;
  }

  if (sound == 1) {
    if (currentMillis - previousMillisSon < duree_son) {
      tone(speakerPin, ton);
    }
    else {
      Serial.println("On eteint le son");
      noTone(speakerPin);
      sound = 0;
    }
  }





  // ------------------ Controle coeur ----------------------------------------
   if (lance == 0 && (currentMillis - previousMillisCoeur >= ecart_battement)) {
     drv.go();
     lance = 1;
     previousMillisCoeur = currentMillis;
    }
    if (lance == 1 && (currentMillis - previousMillisCoeur >= ecart_battement2)) {
     drv.go();
     lance = 0;
    }

}



//  ============================= FONCTIONS ===================================
void requete() {
  // On connecte notre Arduino sur le serveur local de notre ordi et le port 8086 (defaut pour influxDB)
  char erreur = client.connect(serveur, 8086);

  if (erreur == 1) {
    // Pas d'erreur ? on continue !
    Serial.println("Connexion OK, envoi en cours...");

    // On construit l'en-tête de la requête
    client.println("GET /query?db=PressionA&q=SELECT%20time,frequence,urine,valeur%20FROM%20pression%20order%20by%20time%20desc%20limit%201 HTTP/1.1");
    client.println("Host: 172.17.3.140");
    client.println("Connection: close");
    client.println();

    // On enregistre le moment d'envoi de la dernière requête
    derniereRequete = millis();
  } else {
    // La connexion a échoué :(
    // On ferme notre client
    client.stop();
    // On avertit l'utilisateur
    Serial.println("Echec de la connexion");
    switch (erreur) {
      case (-1):
        Serial.println("Time out");
        break;
      case (-2):
        Serial.println("Serveur invalide");
        break;
      case (-3):
        Serial.println("Tronque");
        break;
      case (-4):
        Serial.println("Reponse invalide");
        break;
    }
  }
}


// Fonction qui enlèvent ce qui nous intéresse pas dans la requête
String recupVal(String str) {
  int index1 = str.indexOf("values");
  int index2 = str.indexOf("]]}]}]}");
  return str.substring(index1 + 11, index2);
}
//Fonction qui permet de garder ce qu'il y a après la virgule
String supDebVirg(String str) {
  int iVir = str.indexOf(",");
  return str.substring(iVir + 2);

}
//Fonction qui permet de garder ce qu'il y a avant la virgule
String supFinVirg(String str) {
  int iVir = str.indexOf(",");
  return str.substring(0, iVir - 1);
}

// Code permettant de lancer le Shield Ethernet et de le connecter au serveur d'influxDB
// grâce à des requêtes HTTP à l'infini
// Ces deux bibliothèques sont indispensables pour le shield
#include <SPI.h>
#include <Ethernet.h>

// L'adresse MAC du shield
byte mac[] = {0x90, 0xA2 , 0xDA, 0x10, 0x7A, 0x45};
// L'adresse IP que prendra le shield si attribution automatique ne fonctionne pas
IPAddress ip(192, 168, 137, 1);
// L'objet qui nous servira a la communication
EthernetClient client;
// Le serveur à interroger (ici : l'adresse IPv4 de votre ordinateur)
char serveur[] = "172.17.3.140";

// Pour la lecture de la requête HTTP
char carlu = 0;
String strlu;
String temps;
int freqResp, urine, pressionA;
// Moment de la dernière requête
long derniereRequete = 0;
// Temps minimum entre deux requêtes min (500ms)
const long updateInterval = 1000; //(en ms)
// Mémorise l'état de la connexion entre deux tours de loop
bool etaitConnecte = false;

void setup() {
  Serial.begin(9600);

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
}

void loop()
{
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
}

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

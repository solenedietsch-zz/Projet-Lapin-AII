// Code permettant de tester le moteur à courant continu
#define Ain1 2
#define Ain2 3
#define Bin1 4 // Nécessaire si connexion de 2 moteurs en même temps
#define Bin2 5
int vit;

void setup() {
pinMode(Ain1, OUTPUT);  //Ain1
pinMode(Ain2, OUTPUT);  //Ain2
pinMode(Bin1, OUTPUT);  //Bin1
pinMode(Bin2, OUTPUT);  //Bin2
}

void loop() {
    //Permet de faire tourner dans un sens ou dans l'autre au maximum de vitesse
    digitalWrite(Ain1,HIGH);
    digitalWrite(Ain2,LOW);
    // Permet de faire tourner l'autre moteur 
    digitalWrite(Bin1,HIGH);
    digitalWrite(Bin2,LOW);
    delay(2000);
    digitalWrite(Ain1,LOW);
    digitalWrite(Bin1,LOW);
    
    // Pour faire varier la vitesse du moteur (utilisé dans la suite)
    for( vit = 65 ; vit<=255;vit++){
      analogWrite(Ain1,vit); // analogWrite(pinMoteur,vitesse)
      analogWrite(Bin1,vit);
      delay(100);
    }
    
    digitalWrite(Ain1,LOW);
    digitalWrite(Ain2,LOW);
    digitalWrite(Bin1,LOW);
    digitalWrite(Bin2,LOW);
    delay(2000);
}





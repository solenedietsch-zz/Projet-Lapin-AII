// Code permettant de tester le moteur haptique
#include <Wire.h>
#include "Adafruit_DRV2605.h" //Bibliothèque à télécharger préalablemnt
Adafruit_DRV2605 drv;

void setup() {
  Serial.begin(9600); // Pour lancer le Serial
  Serial.println("DRV test");
  drv.begin();
  drv.selectLibrary(1);
  
  // I2C trigger by sending 'go' command 
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG); 
}
// Choisir 2 ou 4
uint8_t effect = 0;

void loop() {
  Serial.print("Effect #"); Serial.println(effect); // set the effect to play
  drv.setWaveform(0, effect);  // play effect 
  drv.setWaveform(1, 0);       // end waveform
  
  drv.go(); // lance l'effet
  delay(200);
  drv.go(); // lance l'effet à nouveau pour ressembler à un coeur (2 contractions)
  delay(100);
  
  effect++; // permet de tester les différents effet (117 différents)
  if (effect > 117) effect = 1;}

/*
 * Besturing van twee stappenmotoren met Arduino en een joystick
 */
 
//Opname van bibliotheken
#include <AccelStepper.h>
#include <Bounce2.h>
 
//definitie van de constanten van de Arduino-pinnen
const int ledEnable = 13; //de led aan boord zal ons de activeringsstatus van de motoren laten zien
const int pinSwEnable = 7;  //de knop in de joystickmodule die het besturingselement in- of uitschakelt
const int pinEnable = 8;  //de pinnen die de ENABLE-status van de A4988-stuurprogramma's regelen, zijn in serie geschakeld, dus er is slechts één pin nodig om beide te beheren
 
unsigned long debounceDelay = 10; //milliseconden voor de knop debonuce

const int jX = A0;  //analoge pen die de waarden voor de X leest
const int stepX = 3;  //digitale pen die de STEP-signalen naar de X-driver stuurt
const int dirX = 4; //digitale pen die het DIRECTION-signaal naar de X-driver stuurt
long speedX, valX, mapX;  //motor X management variabelen
 
const int jY = A1;  //analoge pen die de waarden voor Y leest
const int stepY = 5;  //digitale pen die de STEP-signalen naar de Y-driver stuurt
const int dirY = 6; //digitale pen die het DIRECTION-signaal naar de Y-driver stuurt
long speedY, valY, mapY;  //Y variabelen voor beheer van motorbewegingen
 
//variabelen gebruikt door de AccelStepper-bibliotheek
const int maxSpeed = 1000;  //volgens de documentatie van de bibliotheek kan deze waarde worden ingesteld op 4000 voor een Arduino UNO
const int minSpeed = 0; //minimum motortoerental
const float accelerazione = 50.0; //aantal stappen per seconde bij acceleratie
 
const int treshold = 30;  //het lezen van de potentiometers is nooit 100% betrouwbaar, deze waarde helpt om het punt te bepalen dat moet worden beschouwd als "Blijf stil" in de bewegingen
long tresholdUp, tresholdDown;  //servicevariabelen om de hierboven beschreven taak te volbrengen
 
boolean abilitato, muoviX, muoviY, enable;  //variabelen voor bewegingsbeheer
 
Bounce btnEnable = Bounce();  //een knop van de Bounce-bibliotheek instantiëren
 
//de motoren aansteken
AccelStepper motoreX(AccelStepper::DRIVER, stepX, dirX);
AccelStepper motoreY(AccelStepper::DRIVER, stepY, dirY);
 
void setup() {
  //initialiseer waarden
  speedX = speedY = 0;
  enable = false;
 
  //definitie van de modaliteiten van de pinnen
  pinMode(ledEnable, OUTPUT);
  pinMode(pinEnable, OUTPUT);
 
  pinMode(pinSwEnable, INPUT_PULLUP); //de invoer van de schakelaar moet worden ingesteld als INPUT_PULLUP
 
  digitalWrite(ledEnable, enable);
  digitalWrite(pinEnable, !enable); //De A4988-drivers schakelen de commando's naar de motor uit als op de ENABLE-pin een HOOG signaal wordt ontvangen, daarom is de waarde tegengesteld aan die van de LED
 
  //configureer de joystickknop met behulp van de Bounce-bibliotheek
  btnEnable.attach(pinSwEnable);
  btnEnable.interval(debounceDelay);
 
  //berekent afstandswaarden waarbinnen de positie van de joystick als "stilstaand" kan worden beschouwd
  tresholdDown = (maxSpeed / 2) - treshold;
  tresholdUp = (maxSpeed / 2) + treshold;
 
  //stel de motorparameters in
  motoreX.setMaxSpeed(maxSpeed);
  motoreX.setSpeed(minSpeed);
  motoreX.setAcceleration(accelerazione);
 
  motoreY.setMaxSpeed(maxSpeed);
  motoreY.setSpeed(minSpeed);
  motoreY.setAcceleration(accelerazione);
}
 
void loop() {
 
  //voer de controle- en leesfunctie uit van de knop die de activeringsstatus bepaalt
  checkEnable();
 
  digitalWrite(ledEnable, enable);  //toont de activeringsstatus via de LED op pin 13
  digitalWrite(pinEnable, !enable); //stel de tegenovergestelde waarde in op de ENABLE-pinnen van de stuurprogramma's
 
  //voer een analoge uitlezing uit van de waarden die afkomstig zijn van de joystick-potentiometers
  valX = analogRead(jX);
  valY = analogRead(jY);
 
  //brengt de waarden in kaart die worden gelezen volgens de maximale en de hoogste snelheid
  mapX = map(valX, 0, 1023, minSpeed, maxSpeed);
  mapY = map(valY, 0, 1023, minSpeed, maxSpeed);
 
  //voer de motorcommandofunctie uit
  pilotaMotori(mapX, mapY);
 
}
 
void pilotaMotori(long mapX, long mapY) {
 
  if (mapX <= tresholdDown) {
    //x gaat terug
    speedX = -map(mapX, tresholdDown, minSpeed,   minSpeed, maxSpeed);
    muoviX = true;
  } else if (mapX >= tresholdUp) {
    //x gaat door
    speedX = map(mapX,  maxSpeed, tresholdUp,  maxSpeed, minSpeed);
    muoviX = true;
  } else {
    //x staat stil
    speedX = 0;
    muoviX = false;
  }
 
  if (mapY <= tresholdDown) {
    //y daalt
    speedY = -map(mapY, tresholdDown, minSpeed,   minSpeed, maxSpeed);
    muoviY = true;
  } else if (mapY >= tresholdUp) {
    //y gaat omhoog
    speedY = map(mapY,  maxSpeed, tresholdUp,  maxSpeed, minSpeed);
    muoviY = true;
  } else {
    //y staat stil
    speedY = 0;
    muoviY = false;
  }
 
  if (muoviX) {
    motoreX.setSpeed(speedX);
    motoreX.run();
  } else {
    motoreX.stop();
  }
 
  if (muoviY) {
    motoreY.setSpeed(speedY);
    motoreY.run();
  } else {
    motoreY.stop();
  }
}
 
 
void checkEnable() {
 
  btnEnable.update();
 
  if (btnEnable.fell()) {
    enable = !enable;
  }
 
}

//https://www.lombardoandrea.com/motori-passo-passo-arduino-joystick/ (italiaans)

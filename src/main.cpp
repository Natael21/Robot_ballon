#include <Arduino.h>
#include <LibRobus.h>
#include <Adafruit_TCS34725.h>

//Constantes Défi du parcours:
const float MAGIC_NUMBER = 0.00035;
const int MAGIC_DELAY_LD = 25;
const uint32_t PPT = 3200;
const float RAYON = 18.9;
const float SPEED_ANGLE = 0.25;
const float SPEED_LIGNE = 0.5;
//Constantes Épreuve du combattant:
const int ABAISSER = 0;
const int MONTER = 150;

float speed = 0;

//map de l'arduino
//PWM
const int PIN_B = 8;
const int PIN_R = 3;
const int PIN_J = 2;

//float SONAR_GetRange(uint8_t 0); un seul sonnar, dans port 1

//Capteur Couleur:
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void reset_ENCODEUR();

void setSameSpeed_MOTORS(float);

uint32_t distance_pulse(float);

float distance_angle(float);

void correction_moteurs(uint32_t, uint32_t);

void ligne_droite(int);

void tourne180();

void tourne(uint8_t idMoteur, float angle);

void afficher_led(char);

void detection_couleur();

void bouger_bras(int degree);

void setup() 
{
  BoardInit();
  
  //Déroulement du parcours:
  ligne_droite(20);
  tourne(RIGHT, 90);
  ligne_droite(25);
  tourne(LEFT, 90);
  ligne_droite(100);
  char couleur = detection_couleur();
  ligne_droite(150);
  bouger_bras(ABAISSER);
  
  if(couleur == 'r'){
    ligne_droite(250);
    tourne(RIGHT, 90);
    ligne_droite(25);
    delay(1000);
    bouger_bras(MONTER);
  }
  else if(couleur == 'y'){
    tourne(RIGHT, 90);
    ligne_droite(40);
    tourne(LEFT, 90);
    ligne_droite(60);
    delay(1000);
    bouger_bras(MONTER);
  }
  else{
    tourne(LEFT, 90);
    ligne_droite(40);
    tourne(RIGHT, 90);
    ligne_droite(150);
    delay(1000);
    bouger_bras(MONTER);
  }
  
  /*
  if (tcs.begin()) {
        Serial.println("Found sensor");
  } else {
        Serial.println("No TCS34725 found ... check your connections");
  }
  */
}

void loop() {
  detection_couleur();
}

//-----------------------Fonctions Capteurs:----------------------------

void detection_couleur(){
  uint16_t clear, red, green, blue;
  tcs.getRawData(&red, &green, &blue, &clear);
  delay(1000);
  Serial.print("C:\t"); Serial.println(clear);
  Serial.print("\tR:\t"); Serial.println(red);
  Serial.print("\tG:\t"); Serial.println(green);
  Serial.print("\tB:\t"); Serial.println(blue);
  
  //afficher_led(couleur);
}

void afficher_led(char couleur)
{
  //initialisation des modes
  pinMode(PIN_J, OUTPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  digitalWrite(PIN_J, 0);
  digitalWrite(PIN_R, 0);
  digitalWrite(PIN_B, 0);

  if (couleur == 'r')
  {
    digitalWrite(PIN_R, HIGH); 
    delay(5000);
    digitalWrite(PIN_R, 0);
  }
 
  else if (couleur == 'j')
  {
    digitalWrite(PIN_J, HIGH);
    delay(5000);
    digitalWrite(PIN_J, 0);
  }
    
  else if (couleur == 'b')
  {
    digitalWrite(PIN_B, HIGH);
    delay(5000);
    digitalWrite(PIN_B, 0);
  }
   
  else 
  {
    digitalWrite(PIN_J, HIGH);
    digitalWrite(PIN_R, HIGH);
    digitalWrite(PIN_B, HIGH);
  }

}

//----------------------Fonction Servomoteur:-----------------------------
void bouger_bras(int degree)
{

}

//--------------------Fonctions Défi du parcours:-------------------------

void reset_ENCODERS()
{
  ENCODER_Reset(RIGHT);
  ENCODER_Reset(LEFT);
}

void setSameSpeed_MOTORS(float speed)
{
  MOTOR_SetSpeed(RIGHT, speed);
  MOTOR_SetSpeed(LEFT, speed);
}

uint32_t distance_pulse(float distance)
{
  uint32_t pulse_attendu = distance / 23.94 * PPT;
  return pulse_attendu;
}

float distance_angle(float angle)
{
  float distance = (2 * PI * angle * RAYON) / 360;
  return distance;
}

void correction_moteurs(uint32_t pulse_gauche, uint32_t pulse_droit)
{
  float erreur;
  
  if(pulse_droit > pulse_gauche) {
    erreur = (pulse_droit - pulse_gauche) * MAGIC_NUMBER;
    MOTOR_SetSpeed(LEFT, speed + erreur);
    MOTOR_SetSpeed(RIGHT, speed - erreur);
  }
  else {
    erreur = (pulse_gauche - pulse_droit) * MAGIC_NUMBER;
    MOTOR_SetSpeed(LEFT, speed - erreur);
    MOTOR_SetSpeed(RIGHT, speed + erreur);
  }
}

void ligne_droite(int distance)
{
  reset_ENCODERS();
  
  uint32_t pulse_droit = 0;
  uint32_t pulse_gauche = 0;
  uint32_t pulse_attendu = distance_pulse(distance);

  while (pulse_droit <= pulse_attendu)
  {
    pulse_droit = ENCODER_Read(RIGHT);
    pulse_gauche = ENCODER_Read(LEFT);
    
    if(pulse_droit <= pulse_attendu * 0.5 )// || pulse_gauche <= pulse_attendu * 0.8
    {
      if(speed <= SPEED_LIGNE)
      {
        speed += 0.005;//2 secondes: 0 => 0.4 ou 2 secondes: 0 => 0.5 pour 0.00625
      }
      else 
      {
        speed = SPEED_LIGNE;
      }
    }

    else
    {
      speed -= 0.005;//1 seconde: 0.5 => 0.3 (-0.2)
    }

    setSameSpeed_MOTORS(speed);

    correction_moteurs(pulse_gauche, pulse_droit);
    delay (MAGIC_DELAY_LD);//40 fois / seconde
  }
}

void tourne180()
{
  delay(200);
  reset_ENCODERS();
  setSameSpeed_MOTORS(0);
  
  float angle = 93;

  uint32_t pulse_distance = distance_pulse(distance_angle(angle));
  uint32_t pulse_droit = 0;
  int32_t pulse_gauche = 0;
  
  MOTOR_SetSpeed(RIGHT, SPEED_ANGLE);
  MOTOR_SetSpeed(LEFT, -SPEED_ANGLE);
  
  while (pulse_droit <= pulse_distance)
  {
    pulse_droit = ENCODER_Read(RIGHT);
    pulse_gauche = ENCODER_Read(LEFT);
    
    if (pulse_droit > pulse_distance)
      MOTOR_SetSpeed(RIGHT, 0);
    if (pulse_gauche < - (int) pulse_distance)
      MOTOR_SetSpeed(LEFT, 0);
    
  }
  
void tourne(uint8_t idMoteur, float angle)
{
  reset_ENCODERS();
  
  uint32_t pulse_distance = distance_pulse(distance_angle(angle));
  uint32_t pulse = 0;
  
  MOTOR_SetSpeed(idMoteur, 0);

  uint8_t autreMoteur;

  if(idMoteur == 0){
    autreMoteur = 1;
    Serial.print("tourne à gauche de ");
    Serial.println(angle);
    Serial.print("Moteur: ");
    Serial.println(autreMoteur);
  }
  else {
    autreMoteur = 0;
    Serial.print("tourne à droite de ");
    Serial.println(angle);
    Serial.print("Moteur: ");
    Serial.println(autreMoteur);
  }

  while (pulse <= pulse_distance)
  {
    pulse = ENCODER_Read(autreMoteur);
  }
  
  MOTOR_SetSpeed(idMoteur, SPEED_ANGLE);
}
  
  
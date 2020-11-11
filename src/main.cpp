#include <Arduino.h>
#include <LibRobus.h>
#include <Adafruit_TCS34725.h>

//Constantes Défi du parcours:
const float MAGIC_NUMBER = 0.00035;
const int MAGIC_DELAY_LD = 25;
const uint32_t PPT = 3200;
const float RAYON = 18.9;
const float SPEED_ANGLE = 0.25;

//Constantes Épreuve du combattant:
const int ABAISSER = 160;
const int MONTER = 125;
const float SPEED_BALLON = 0.2;
//const int SIFFLET_MIN = 555;
//const int SIFFLET_MAX = 625;

float speed = 0;

//map de l'arduino
//PWM
const int PIN_B = 12;
const int PIN_R = 10;
const int PIN_J = 11;

//ANALOG IN
const int PIN_SIFFLET = 13;
const int SIFFLET_MIN = 620;
const int SIFFLET_MAX = 710;

//float SONAR_GetRange(uint8_t 0); un seul sonnar, dans port 1

//Capteur Couleur:
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void reset_ENCODEUR();

void setSameSpeed_MOTORS(float);

uint32_t distance_pulse(float);

float distance_angle(float);

void correction_moteurs(uint32_t, uint32_t);

void ligne_droite(int, float, float);

void tourne180();

void tourne(uint8_t idMoteur, float angle);

void afficher_led(char);

char detection_couleur();

void bouger_servomoteur(int degree);

int detection_sifflet();

void suiveur_ligne();

void setup() 
{
  BoardInit();
  
  SERVO_Enable(1);
  bouger_servomoteur(MONTER);

  //Déroulement du parcours:
  //while(detection_sifflet() != 1){}
  while(!ROBUS_IsBumper(3)){}

  ligne_droite(20, SPEED_BALLON, SPEED_BALLON);
  tourne(RIGHT, 92);
  ligne_droite(7, SPEED_BALLON, SPEED_BALLON);
  tourne(LEFT, 92);
  ligne_droite(30, SPEED_BALLON, 0.15);
  char couleur;
  for(int i = 0; i < 3; i++){
    couleur = detection_couleur();
    afficher_led(couleur);
  }
  ligne_droite(105, SPEED_BALLON, 0.15);
  bouger_servomoteur(ABAISSER);
  
  if(couleur == 'r'){
    
    ligne_droite(225, SPEED_BALLON, SPEED_BALLON);
    tourne(RIGHT, 92);
    ligne_droite(5, SPEED_BALLON, 0.15);
  }
  else if(couleur == 'j'){
    
    tourne(RIGHT, 92);
    ligne_droite(7, SPEED_BALLON, SPEED_BALLON);
    tourne(LEFT, 92);
    ligne_droite(53, SPEED_BALLON, 0.15);
  }
  else{
    
    tourne(LEFT, 92);
    ligne_droite(20, SPEED_BALLON, SPEED_BALLON);
    tourne(RIGHT, 91);
    ligne_droite(140, SPEED_BALLON, 0.15);
  }
  speed = 0;
  setSameSpeed_MOTORS(speed);
  delay(1000);
  bouger_servomoteur(MONTER);
  
}

void loop() {}

//-----------------------Fonctions Capteurs:----------------------------
void suiveur_ligne(){
  pinMode(8, OUTPUT);
  digitalWrite(8, 1);
  int pin3 = analogRead(A9);
  int pin2 = analogRead(A10);
  int pin1 = analogRead(A11);
  Serial.print(pin1);
  Serial.print("\t");
  Serial.print(pin2);
  Serial.print("\t");
  Serial.print(pin3);
  Serial.print("\n");
  delay(1000);
}

char detection_couleur(){
  char couleur;
  uint16_t clear, red, green, blue;
  tcs.getRawData(&red, &green, &blue, &clear);
  delay(250);
  Serial.print("C:\t"); Serial.println(clear);
  Serial.print("\tR:\t"); Serial.println(red);
  Serial.print("\tG:\t"); Serial.println(green);
  Serial.print("\tB:\t"); Serial.println(blue);

  if(green > red && blue > red){
    Serial.println("Bleu");
    couleur = 'b';
  }
  else if(red > blue && green > blue){
    Serial.println("Jaune");
    couleur = 'j';
  }
  else if(red > green && blue > green){
    Serial.println("Rose");
    couleur = 'r';
  }
  
  //afficher_led(couleur);
  return couleur;
}

int detection_sifflet(){
  int voltage;
  voltage = analogRead(PIN_SIFFLET);
  Serial.print(voltage);
  Serial.print("\n");
  if (voltage >= SIFFLET_MIN)
  {
    return 1;
  }
  else 
  {
    return 0;
  }
}

void afficher_led(char couleur){
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
    delay(1000);
    /*digitalWrite(PIN_R, 0);*/
  }
 
  else if (couleur == 'j')
  {
    digitalWrite(PIN_J, HIGH);
    delay(1000);
    /*digitalWrite(PIN_J, 0);*/
  }
    
  else if (couleur == 'b')
  {
    digitalWrite(PIN_B, HIGH);
    delay(1000);
    /*digitalWrite(PIN_B, 0);*/
  }
   
  else 
  {
    digitalWrite(PIN_J, HIGH);
    digitalWrite(PIN_R, HIGH);
    digitalWrite(PIN_B, HIGH);
  }
}

//----------------------Fonctions Motrices:-----------------------------
void bouger_servomoteur(int degree){
  if(degree == ABAISSER){
    for(int i = MONTER; i <= degree; i+=5){
      delay(50);
      SERVO_SetAngle(1, i);
    }
  } 
  else {
    for(int i = ABAISSER; i >= degree; i-=5){
      delay(50);
      SERVO_SetAngle(1, i);
    }
  }
  
}

void reset_ENCODERS(){
  ENCODER_Reset(RIGHT);
  ENCODER_Reset(LEFT);
}

void setSameSpeed_MOTORS(float speed){
  MOTOR_SetSpeed(RIGHT, speed);
  MOTOR_SetSpeed(LEFT, speed);
}

uint32_t distance_pulse(float distance){
  uint32_t pulse_attendu = distance / 23.94 * PPT;
  return pulse_attendu;
}

float distance_angle(float angle){
  float distance = (2 * PI * angle * RAYON) / 360;
  return distance;
}

void correction_moteurs(uint32_t pulse_gauche, uint32_t pulse_droit){
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

void ligne_droite(int distance, float vitesseMax, float vitesseMin){
  reset_ENCODERS();
  
  uint32_t pulse_droit = 0;
  uint32_t pulse_gauche = 0;
  uint32_t pulse_attendu = distance_pulse(distance);

  while (pulse_droit <= pulse_attendu)
  {
    pulse_droit = ENCODER_Read(RIGHT);
    pulse_gauche = ENCODER_Read(LEFT);
    
    if(pulse_droit <= pulse_attendu * 0.6 )// || pulse_gauche <= pulse_attendu * 0.8
    {
      if(speed <= vitesseMax)
      {
        speed += 0.005;//2 secondes: 0 => 0.4 ou 2 secondes: 0 => 0.5 pour 0.00625
      }
    }

    else if(pulse_droit >= pulse_attendu * 0.6 && pulse_droit <= pulse_attendu * 0.9)
    {
      if(speed >= vitesseMin)
      {
        speed -= 0.005;//1 seconde: 0.5 => 0.3 (-0.2)
      }
    }
    setSameSpeed_MOTORS(speed);

    correction_moteurs(pulse_gauche, pulse_droit);
    delay (MAGIC_DELAY_LD);//40 fois / seconde
  }
}

void tourne180(){
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
}
  
void tourne(uint8_t idMoteur, float angle){
  reset_ENCODERS();
  
  uint32_t pulse_distance = distance_pulse(distance_angle(angle));
  uint32_t pulse = 0;
  
  MOTOR_SetSpeed(idMoteur, 0);

  uint8_t autreMoteur;

  if(idMoteur == 0){
    autreMoteur = 1;
  }
  else {
    autreMoteur = 0;
  }

  while (pulse <= pulse_distance)
  {
    pulse = ENCODER_Read(autreMoteur);
  }
  
  MOTOR_SetSpeed(idMoteur, speed);
}
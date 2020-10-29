#include <Arduino.h>
#include <LibRobus.h>

//Constants:
const float MAGIC_NUMBER = 0.00035;
const int MAGIC_DELAY_LD = 25;
const uint32_t PPT = 3200;
const float RAYON = 18.9;
const float SPEED_ANGLE = 0.25;
const float SPEED_LIGNE = 0.5;
//Ligne verte: 2.5 cm
float speed = 0;



//float SONAR_GetRange(uint8_t 0); un seul sonnar, dans port 1

void reset_ENCODEUR();

void setSameSpeed_MOTORS(float);

uint32_t distance_pulse(float);

float distance_angle(float);

void correction_moteurs(uint32_t, uint32_t);

void ligne_droite(int);


void setup() 
{
  BoardInit();
  //initialisation des modes
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
}

void loop() {
digitalWrite(2, HIGH);
digitalWrite(3, LOW);
digitalWrite(4, LOW);
delay(3000);

digitalWrite(2, LOW);
digitalWrite(3, HIGH);
digitalWrite(4, LOW);
delay(1000);
  
digitalWrite(2, LOW);
digitalWrite(3, LOW);
digitalWrite(4, HIGH);
delay(1000);
}


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


  

#include <Wire.h>

#define PWM_PIN_L_A 2
#define PWM_PIN_L_B 10
#define PWM_PIN_R_A 6
#define PWM_PIN_R_B 5

#define left_motor_channel_a 0
#define left_motor_channel_b 1
#define right_motor_channel_a 2
#define right_motor_channel_b 3

#define SENSOR_1_PIN 4
#define SENSOR_2_PIN 3
#define SENSOR_3_PIN 1
#define SENSOR_4_PIN 0

#define W_LED_ON 20
#define IR_LED_ON 21

#define threshold 2000

uint8_t vspeed = 250 ; //50    
uint8_t tspeed = 250; //70
uint8_t mini_speed = 45;
uint8_t half_speed = 100; // 65
uint8_t vspeed_l = vspeed;
uint8_t tspeed_l = 250;
uint8_t vspeed_r = vspeed ;
uint8_t tspeed_r = 250;

bool isTurnLeft = false,
     isTurnRight = false;

void stop()
{
    ledcWrite(left_motor_channel_a, 0);
    ledcWrite(left_motor_channel_b, 0);
    ledcWrite(right_motor_channel_a, 0);
    ledcWrite(right_motor_channel_b, 0); 
    Serial.println("stop");
}

void forward()
{
    ledcWrite(left_motor_channel_a, vspeed_l);
    ledcWrite(left_motor_channel_b, 0);
    ledcWrite(right_motor_channel_a, vspeed_r);
    ledcWrite(right_motor_channel_b, 0);    
    Serial.println("forward");
} 

void backward()
{
    ledcWrite(left_motor_channel_a, 0);
    ledcWrite(left_motor_channel_b, vspeed_l);
    ledcWrite(right_motor_channel_a, 0);
    ledcWrite(right_motor_channel_b, vspeed_r);
    Serial.println("backward");
} 

void left()
{
    ledcWrite(left_motor_channel_a, 0);
    ledcWrite(left_motor_channel_b, tspeed_l);
    ledcWrite(right_motor_channel_a, tspeed_r);
    ledcWrite(right_motor_channel_b, 0);      
    Serial.println("left");
}

void right()
{
    ledcWrite(left_motor_channel_a, tspeed_l);
    ledcWrite(left_motor_channel_b, 0);
    ledcWrite(right_motor_channel_a, 0);
    ledcWrite(right_motor_channel_b, tspeed_r);  
    Serial.println("right");
}

void setup() {

  Serial.begin(115200);
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWM_PIN_L_A, left_motor_channel_a);
  ledcAttachPin(PWM_PIN_L_B, left_motor_channel_b);
  ledcAttachPin(PWM_PIN_R_A, right_motor_channel_a);
  ledcAttachPin(PWM_PIN_R_B, right_motor_channel_b);

  pinMode(W_LED_ON,OUTPUT);
  pinMode(IR_LED_ON,OUTPUT);
  digitalWrite(W_LED_ON, 0);
  digitalWrite(IR_LED_ON, 1);
}

void ForwardAndTurn(uint8_t speedL, uint8_t speedR)
{
    ledcWrite(left_motor_channel_a, speedL);
    ledcWrite(left_motor_channel_b, 0);
    ledcWrite(right_motor_channel_a, speedR);
    ledcWrite(right_motor_channel_b, 0);
}

void SpecialWayLeft()
{
  ledcWrite(left_motor_channel_a, 0);
  ledcWrite(left_motor_channel_b, 100);
  ledcWrite(right_motor_channel_a, 255);
  ledcWrite(right_motor_channel_b, 0);
  while(getValueIR() != 0b1000 && getValueIR() != 0b1100);
}

uint8_t getValueIR()
{
  int sensor_1_state = analogRead(SENSOR_1_PIN);
  int sensor_2_state = analogRead(SENSOR_2_PIN);
  int sensor_3_state = analogRead(SENSOR_3_PIN);
  int sensor_4_state = analogRead(SENSOR_4_PIN);
uint8_t sensor_array = 0;
  sensor_array += (sensor_1_state <= threshold);
  sensor_array<<=1;
  sensor_array += (sensor_2_state <= threshold);
  sensor_array<<=1;
  sensor_array += (sensor_3_state <= threshold);
  sensor_array<<=1;
  sensor_array += (sensor_4_state <= threshold);
  return sensor_array;
}

// 0000 va bool working
void dirf()
{
  while(isTurnLeft)
  {
    ForwardAndTurn(tspeed_l / 30, tspeed_r);
    uint8_t sensor_array = getValueIR();
    if(sensor_array == 0b1100 || sensor_array == 0b1110) isTurnLeft = false;
  }
  while(isTurnRight)
  {
    ForwardAndTurn(tspeed_l, tspeed_r / 30);
    uint8_t sensor_array = getValueIR();
    if(sensor_array == 0b0011 || sensor_array == 0b0111) isTurnRight = false;
  }
}

void loop() {
  switch(getValueIR()) 
  {
    case 0b1000:
      isTurnLeft = true;
      isTurnLeft = false;
      ForwardAndTurn(mini_speed, tspeed_r);
      break;
    case 0b1100:
      isTurnRight = false;
      isTurnLeft = false;
      ForwardAndTurn(half_speed, tspeed_r);
      break;
    case 0b1111:
      SpecialWayLeft();
      break;
    case 0b0001:
      isTurnRight = true;
      isTurnLeft = false;
      ForwardAndTurn(tspeed_l, mini_speed);
      break;
    case 0b0011:
      isTurnRight = false;
      isTurnLeft = false;
      ForwardAndTurn(tspeed_l, half_speed);
      break;

    case 0b0111:
    case 0b1110:
    case 0b0110:
    case 0b0100:
    case 0b0010:
      isTurnRight = false;
      isTurnLeft = false;
      forward();
      break;

    case 0b0000:
      if(isTurnLeft || isTurnRight)
        dirf();
      break;

    default:
      forward();
      break;
  }
  delay(10);
}
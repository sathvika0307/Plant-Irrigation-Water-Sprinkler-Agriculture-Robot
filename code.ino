// RemoteXY select connection mode and include library
#define REMOTEXY_MODE__HARDSERIAL

#include<RemoteXY.h>
#include<AFMotor.h>
#include<Servo.h>

// RemoteXY connection settings
#define REMOTEXY_SERIAL Serial
#define REMOTEXY_SERIAL_SPEED 9600

// RemoteXY configurate
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = 
{ 255,7,0,223,0,11,26,0,
  1,0,73,30,10,10,175,31,70,0,
  1,0,63,40,10,10,175,31,76,84,
  0,1,0,73,50,10,10,175,31,66,
  0,1,0,83,40,10,10,175,31,66,
  84,0,130,2,4,8,33,11,147,129,
  0,6,12,20,3,31,83,111,105,108,
  32,77,111,105,115,116,117,114,101,58,
  0,67,1,25,11,11,5,31,31,31,
  130,2,58,8,18,11,147,129,0,60,
  11,15,5,31,87,97,116,101,114,58,
  0,65,1,79,8,8,8,65,4,89,
  8,8,8,129,0,79,17,8,3,178,
  72,73,71,72,0,129,0,90,17,7,
  3,1,76,79,87,0,1,1,35,41,
  21,7,177,31,80,85,77,80,0,65,
  1,40,4,6,6,65,2,40,11,6,
  6,65,4,40,18,6,6,129,0,48,
  6,5,2,178,72,73,71,72,0,129,
  0,47,13,8,2,133,78,79,82,77,
  65,76,0,129,0,48,20,4,2,1,
  76,79,87,0,4,32,10,31,5,27,
  177,31,4,32,21,31,5,27,177,31 };

  // this structure defines all the variables and events of your control interface
  struct {

    // input variables
    // uint8_t button_ft; // =1 if button pressed, else = 0
    // uint8_t button_lt; // =1 if button pressed, else = 0
    // uint8_t button_bt; // =1 if button pressed, else = 0 
    // uint8_t button_rt; // =1 if button pressed, else = 0
    uint8_t button_pump; //=1 if button pressed, else = 0
    int8_t slider_2; // = -100..100 slider postion
    int8_t slider_1; // = -100..100 slider postion

    // output variables
    char SM_level[31]; //string UTF8 end zero
    uint8_t led_blue_b; // = 0..255 LED Blue brightness
    uint8_t led_red_r; // = 0..255 LED Red brightness
    uint8_t led_high_b; // = 0..255 LED Green brightness
    uint8_t led_normal_g; // = 0..255 LED Blue brightness
    uint8_t led_low_r; // = 0..255 LED Red brightness

    // other variable 
    uint8_t connect_flag; // = 1 if wire connected, else = 0
} RemoteXY;
#pragma pack(pop)

// Declare Motor used in L293D
AF_DCMotor motor3(3,MOTOR34_1KHZ);
Servo myservo1;
Servo myservo2;

// Declare variables used in Soil Moisture Sensor
int sensorPin = A0; // Soil moisture sensor placed on A0 pin
int sensorValue = 0;
int percentValue = 0;
// Declare variables used in Ultrasonic Sensor
const int trigPin = 11;
const int echoPin = 12;
long duration;
int distance;
// Declare variables used in IR Proximity Sensor
int irSensorL = A1; //Left IR Sensor placed on A1 pin
int sensorLeftRead = 0;

void setup()
{
  
  // TODO you setup code
  myservo1.attach(10);
  myservo1.write(80);
  myservo2.attach(9);
  myservo2.write(100);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop()
{
  RemoteXY_Handler ();
  sensorLeftRead = analogRead(irSensorL); // read Left IR Sensor
  if(RemoteXY.button_pump) // if button pump is pressed
  Pump();
  else
  Stop();

  servo1(); //void servo1() is called
  servo2(); // void servo2() is called
  soilMoisture_Setup(); // perform the reading of soil mositure in loop
  ultrasonic(); // perform the water level 
}

void Stop()
{
  motor3.setSpeed(0); //Definne minimum velocity
  motor3.run(RELEASE); // stop the motor when release the button
}
void Pump()
{
  motor3.setSpeed(250); //Definne minimum velocity
  motor3.run(FORWARD); // pump water
}

void soilMoisture_Setup()
{
  sensorValue = analogRead(sensorPin); // read the soil moisture
  // map the sesnorValue and store it to percent value
  percentValue = map(sensorValue, 1023, 200, 0, 100);
  itoa (percentValue, RemoteXY.SM_level, 10); // display the text string to the app
  Serial.println(percentValue);

  if (percentValue <= 60 && percentValue >= 20) {
    RemoteXY.led_low_r = 0;
    RemoteXY.led_normal_g = 255; // greeen LED in app will ON
    RemoteXY.led_high_b = 0;
  }
  else if(percentValue < 20) {
    RemoteXY.led_low_r = 255; // red LED in app will ON
    RemoteXY.led_normal_g = 0;
    RemoteXY.led_high_b = 0;
  }
   else if(percentValue > 60) {
    RemoteXY.led_low_r = 0;
    RemoteXY.led_normal_g = 0;
    RemoteXY.led_high_b = 255; // blue LED in app will ON
  }
}

void servo1() { // to control servo 1
  int angle1 = RemoteXY.slider_1 * 0.8+40; // equation for min. and max. angle is 105-180
  myservo1.write(angle1);
}
void servo2() { // to control servo 2
  int angle2 = RemoteXY.slider_1 + 50; // equation for min. and max. angle is 105-180
  myservo2.write(angle2);
}

void ultrasonic() {
  long duration, distance;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2)/29.1;
  delay(10);
  if (distance <= 7) {
    RemoteXY.led_blue_b = 255;
    RemoteXY.led_red_r = 0;
  }
  else if (distance > 7) {
    RemoteXY.led_blue_b = 0;
    RemoteXY.led_red_r = 255;  
  }
}

#include <AFMotor.h>
#include <SoftwareSerial.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

AF_DCMotor motorlu(4);
AF_DCMotor motorld(1);
AF_DCMotor motorrd(2);
AF_DCMotor motorru(3);
SoftwareSerial mySerial(53, 52);
MPU6050 mpu;
#define OUTPUT_READABLE_YAWPITCHROLL
#define INTERRUPT_PIN 2
bool cur = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init w as successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };

int trig = 51, echo = 50;
unsigned int duration, distance;
// Values for joistic
  int forward_int = 1;
  int back_int = -1;
  int stop_int = 0;
  int right_int = 2;
  int left_int = -2;
  int current_side = 0;
  int rht, lft;
  
// Values for accelerometer
// angles
  float yaw_0;
  float dyaw_0 = 0.0;
  

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
volatile int mpuOverflows = 0;
void dmpDataReady() {
  if(mpuInterrupt)
    ++mpuOverflows;
  mpuInterrupt = true;
}

float get_angle();
int check_side();

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
  Serial.begin(9600);
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); 
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);

    // turn on motor
    motorlu.setSpeed(200);
    motorld.setSpeed(200);
    motorru.setSpeed(200);
    motorrd.setSpeed(200);
    motorlu.run(RELEASE);
    motorld.run(RELEASE);
    motorru.run(RELEASE);
    motorrd.run(RELEASE);
    
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); 

    if (devStatus == 0) {
        mpu.setDMPEnabled(true);
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
    }
    
    // wait for 30 seconds
  
    delay(30000);
    Serial.println("Come on and slam");
    
    yaw_0 = get_angle();

    mySerial.begin(9600);
    pinMode (trig, OUTPUT);
    pinMode (echo, INPUT);
}

int check_side(){
  
}


float get_angle() {

    while (!mpuInterrupt && fifoCount < packetSize) {
    }

    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    fifoCount = mpu.getFIFOCount();

    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        mpu.resetFIFO();

    } else if (mpuIntStatus & 0x02) {
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        fifoCount -= packetSize;

        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        #endif
    }
    
    return ypr[0] * 180/M_PI;
}


void turn_right_90(float an_pre, float rotan){
  float an, dan = 0.0;
  right();  
  while (1 == 1){
    an = get_angle();
    dan += an - an_pre;
    if ((dan >= rotan) || (dan <= (-1 * rotan))){
      break;
    }
    an_pre = an;
  }
  forward();
}


void turn_left_90(float an_pre, float rotan){
  float an, dan = 0.0;
  left();  
  while (1 == 1){
    an = get_angle();
    dan += an - an_pre;
    if ((dan > rotan) || (dan < (-1 * rotan))){
      break;
    }
    an_pre = an;
  }
  forward();
}


void forward(){
  motorlu.setSpeed(200);
  motorld.setSpeed(200);
  motorru.setSpeed(200);
  motorrd.setSpeed(200);
  motorlu.run(FORWARD);
  motorld.run(FORWARD);
  motorrd.run(FORWARD);
  motorru.run(FORWARD);
}

void right() {
  motorlu.setSpeed(200);
  motorld.setSpeed(200);
  motorru.setSpeed(200);
  motorrd.setSpeed(200);
  motorlu.run(FORWARD);
  motorld.run(FORWARD);
  motorrd.run(BACKWARD);
  motorru.run(BACKWARD);
}

void left() {
  motorlu.setSpeed(200);
  motorld.setSpeed(200);
  motorru.setSpeed(200);
  motorrd.setSpeed(200);
  motorlu.run(BACKWARD);
  motorld.run(BACKWARD);
  motorrd.run(FORWARD);
  motorru.run(FORWARD);
}
 
 void stap() {
  motorlu.setSpeed(200);
  motorld.setSpeed(200);
  motorru.setSpeed(200);
  motorrd.setSpeed(200);
  motorlu.run(RELEASE);
  motorld.run(RELEASE);
  motorru.run(RELEASE);
  motorrd.run(RELEASE);
}

void moving() {
  yaw_0 = get_angle();
  digitalWrite(trig, HIGH);
  delayMicroseconds (10);
  digitalWrite(trig, LOW);
  delayMicroseconds (1);
  duration = pulseIn(echo, HIGH);
  distance = duration * 0.034/2;
  if (distance > 10){
    forward();
    } else {
    rht = analogRead(A0);
    lft = analogRead(A1);
    if (lft < rht) {
      return turn_left_90(yaw_0, 90.0);
    } else {
      return turn_right_90(yaw_0, 90.0);
    }
  }
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop(){
  char c;
  if (mySerial.available()) {
    c = mySerial.read();
    Serial.println(c);
    if (c == '1')
    {
      moving();
    }
    else if (c == '0') {
      stap(); 
    }
  }
}

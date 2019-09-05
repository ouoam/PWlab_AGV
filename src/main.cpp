#include <Arduino.h>

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char auth[] = "4305ebb3ece54cd7bb96f69eeee95f29";
const char ssid[] = "MikroTik-066735";
const char pass[] = "1234567890";

const int magSensor[] = {23, 19, 21, 22, -1};

#define MOTER_L_SPEED 33
#define MOTER_L_REVERSE 27
#define MOTER_L_LOCK 13
#define MOTER_R_SPEED 32
#define MOTER_R_REVERSE 26
#define MOTER_R_LOCK 12

void go(int l, int r)
{
  ledcWrite(0, l > 0 ? l : -l);
  digitalWrite(MOTER_L_REVERSE, l > 0);
  ledcWrite(1, r > 0 ? r : -r);
  digitalWrite(MOTER_R_REVERSE, r < 0);
}

int lNow = 0;
int rNow = 0;
int lTarget;
int rTarget;

unsigned long nextSetTimeL = 0;
unsigned long nextSetTimeR = 0;

void setGo(int l, int r)
{
  if (l != lTarget)
  {
    lTarget = l;
  }

  if (r != rTarget)
  {
    rTarget = r;
  }
}

void loopGo()
{
  if (millis() > nextSetTimeL)
  {
    if (lTarget != lNow)
    {
      if (lTarget > lNow)
        lNow += 3;
      else
        lNow -= 3;
    }
    nextSetTimeL += 1;
  }

  if (millis() > nextSetTimeR)
  {
    if (rTarget != rNow)
    {
      if (rTarget > rNow)
        rNow += 3;
      else
        rNow -= 3;
    }
    nextSetTimeR += 1;
  }
  go(lNow, rNow);
}

bool lBack = false;
bool rBack = false;

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt();
  setGo(lBack ? -pinValue : pinValue, rTarget);
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  setGo(lTarget, rBack ? -pinValue : pinValue);
}

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt();
  lBack = pinValue == 1;
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
  rBack = pinValue == 1;
}

BLYNK_WRITE(V9)
{
  int x = param[0].asInt();
  int y = param[1].asInt();

  setGo(x, y);
}

int speed = 0;

BLYNK_WRITE(V10)
{
  int Value = param.asInt();
  speed = Value;
}

BLYNK_WRITE(V11)
{
  int Value = param.asInt();
  if (Value == 1)
  {
    setGo(speed, speed);
  }
  else
  {
    setGo(0, 0);
  }
}

BLYNK_WRITE(V12)
{
  int Value = param.asInt();
  if (Value == 1)
  {
    setGo(-speed, -speed);
  }
  else
  {
    setGo(0, 0);
  }
}

BLYNK_WRITE(V13)
{
  int Value = param.asInt();
  if (Value == 1)
  {
    setGo(0, speed);
  }
  else
  {
    setGo(0, 0);
  }
}

BLYNK_WRITE(V14)
{
  int Value = param.asInt();
  if (Value == 1)
  {
    setGo(speed, 0);
  }
  else
  {
    setGo(0, 0);
  }
}

unsigned long nextRun = 0;

bool folowLine = false;
BLYNK_WRITE(V20)
{
  int Value = param.asInt();
  if (Value == 1)
    folowLine = true;
  Serial.println("IN");
  nextRun = millis();
  setGo(520, 520);
}

BLYNK_WRITE(V21)
{
  int Value = param.asInt();
  if (Value == 1)
    folowLine = false;
  setGo(0, 0);
}

void setup()
{
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass, IPAddress(161, 246, 6, 1), 8012);
  Blynk.syncAll();

  pinMode(MOTER_L_REVERSE, OUTPUT);
  pinMode(MOTER_L_LOCK, OUTPUT);
  pinMode(MOTER_R_REVERSE, OUTPUT);
  pinMode(MOTER_R_LOCK, OUTPUT);

  ledcSetup(0, 50000, 10);
  ledcAttachPin(MOTER_L_SPEED, 0);

  ledcSetup(1, 50000, 10);
  ledcAttachPin(MOTER_R_SPEED, 1);

  for (int i = 0; magSensor[i] != -1; i++)
  {
    pinMode(magSensor[i], INPUT_PULLUP);
  }
}

bool isFound(int port)
{
  return !digitalRead(port);
}

float Kp = 4;
float Kd = 2;
float Ki = 0;

BLYNK_WRITE(V31)
{
  float Value = param.asFloat();
  Kp = Value;
}

BLYNK_WRITE(V32)
{
  float Value = param.asFloat();
  Ki = Value;
}

BLYNK_WRITE(V33)
{
  float Value = param.asFloat();
  Kd = Value;
}

//***********************************************************************

//ขับเคลื่อน
int motorSpeed;
int baseSpeed = 475;
int speedB;
int speedA;
int maxSpeed = 600;
int sum_error = 0;

// PID
int error = 0;
int pre_error = 0;

bool sss1;
bool sss2;
bool sss3;
bool sss4;

// ********************************************************

void loop()
{
  Blynk.run();
  loopGo();

  // for (int i = 0; i < 4; i++)
  // {
  //   Serial.print(isFound(magSensor[i]));
  //   Serial.print(" ");
  // }
  // Serial.println();

  if (folowLine)
  {
    sss1 = isFound(magSensor[0]);
    sss2 = isFound(magSensor[1]);
    sss3 = isFound(magSensor[2]);
    sss4 = isFound(magSensor[3]);

    //PID
    /*    int Ka =analogRead(pin_bA);
    int Kb =analogRead(pin_bB);
    int Kc =analogRead(pin_bC);*/

    if (sss2 && sss3 && sss4)
    {
      error = 3;
    }
    else if (sss2 && sss3 && !sss4)
    {
      error = 2;
    }
    else if (sss2 && !sss3)
    {
      error = 1;
    }

    else if (sss1)
    {
      error = -2;
    }
    else
    // (sss4 && sss3 && sss2 && sss1)
    {
      error = 0;
    }

    /// check black black

    /* else if( (sss1 > 500) && (sss2> 500) && (sss3 > 500 ) && (sss4 > 500) &&(sss5 > 500) && (sss6 > 500) && (sss7 > 500) && (sss8 > 500) && (sss9 > 500) )
    {
        error = pre_error;
    }
*/
    motorSpeed = Kp * error + Kd * (error - pre_error) + Ki * (sum_error);
    speedA = baseSpeed + motorSpeed;
    speedB = baseSpeed - motorSpeed;

    if (speedA > maxSpeed)
      speedA = maxSpeed;
    if (speedB > maxSpeed)
      speedB = maxSpeed;
    if (speedA < 0)
      speedA = 0;
    if (speedB < 0)
      speedB = 0;

    pre_error = error;
    sum_error += error;
    Serial.print("speed A");
    Serial.println(speedA);
    Serial.print("speed B");
    Serial.println(speedB);

    setGo(speedA, speedB);
    delay(10);
  }
}
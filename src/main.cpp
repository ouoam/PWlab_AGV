#include <Arduino.h>

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char auth[] = "4305ebb3ece54cd7bb96f69eeee95f29";
const char ssid[] = "KMITL-WIFI";
const char pass[] = "";

const int magSensor[] = {4, 16, 17, 5, 18, 19, -1};

#define MOTER_L_SPEED 33
#define MOTER_R_SPEED 32

void go(int l, int r)
{
  ledcWrite(0, l);
  ledcWrite(1, r);
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
        lNow += 7;
      else
        lNow -= 7;
    }
    nextSetTimeL += 1;
  }

  if (millis() > nextSetTimeR)
  {
    if (rTarget != rNow)
    {
      if (rTarget > rNow)
        rNow += 7;
      else
        rNow -= 7;
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
  {
    folowLine = true;
    nextRun = millis();
    Serial.println("IN");
    setGo(520, 520);
  }
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

  pinMode(22, INPUT_PULLUP);

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
int baseSpeed = 700;
int speedB;
int speedA;
int maxSpeed = 1000;
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

  // for (int i = 0; magSensor[i] != -1; i++)
  // {
  //   Serial.print(isFound(magSensor[i]));
  //   Serial.print(" ");
  // }
  // Serial.println();

  // if (analogRead(35) < 200)
  // {
  //   setGo(0, 0);
  //   lNow = 0;
  //   rNow = 0;
  //   go(lNow, rNow);
  //   Serial.println(analogRead(35));
  //   delay(1000);
  // }

  // if (digitalRead(22) == 0)
  // {
  //   folowLine = true;
  //   Serial.println("IN");
  //   nextRun = millis();
  //   setGo(520, 520);
  // }

  if (folowLine)
  {
    int sen = 0;
    for (int i = 0; magSensor[i] != -1; i++)
    {
      sen |= isFound(magSensor[i]) << (5 - i);
    }

    if (sen == 0)
    {
      folowLine = false;
      setGo(0, 0);
      Serial.println("OUT");
    }
    else
    {
      int error = 0;

      switch (sen)
      {
      case 0b100000:
        error = -7;
        break;
      case 0b110000:
        error = -4;
        break;
      case 0b010000:
      case 0b111000:
        error = -3;
        break;
      case 0b011000:
        error = -2;
        break;
      case 0b001000:
      case 0b011100:
        error = -1;
        break;
      case 0b001100:
        error = 0;
        break;
      case 0b001110:
      case 0b000100:
        error = 1;
        break;
      case 0b000110:
        error = 2;
        break;
      case 0b000111:
      case 0b000010:
        error = 3;
        break;
      case 0b000011:
        error = 4;
        break;
      case 0b000001:
        error = 7;
        break;
      }

      //PID
      /*    int Ka =analogRead(pin_bA);
      int Kb =analogRead(pin_bB);
      int Kc =analogRead(pin_bC);*/

      /// check black black

      /* else if( (sss1 > 500) && (sss2> 500) && (sss3 > 500 ) && (sss4 > 500) &&(sss5 > 500) && (sss6 > 500) && (sss7 > 500) && (sss8 > 500) && (sss9 > 500) )
      {
          error = pre_error;
      }
  */
      motorSpeed = Kp * error + Kd * (error - pre_error) + Ki * (sum_error);
      speedA = baseSpeed + motorSpeed;
      speedB = baseSpeed - motorSpeed;

      // if (error == 0)
      // {
      //   speedA += 50;
      //   speedB += 50;
      // }

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

      setGo(speedA, speedB);
    }
  }
}
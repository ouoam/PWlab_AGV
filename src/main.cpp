#include <Arduino.h>

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char auth[] = "4305ebb3ece54cd7bb96f69eeee95f29";
const char ssid[] = "MikroTik-066735";
const char pass[] = "1234567890";

const int magnaticSensor[] = {22, 21, 19, 23, -1};

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
  lTarget = l;
  rTarget = r;
  if (-100 < l && l < 100)
  {
    digitalWrite(MOTER_L_LOCK, HIGH);
    lNow = l;
  }
  else
  {
    digitalWrite(MOTER_L_LOCK, LOW);
    nextSetTimeL = millis() + 150;
  }

  if (-100 < r && r < 100)
  {
    digitalWrite(MOTER_R_LOCK, HIGH);
    rNow = r;
  }
  else
  {
    digitalWrite(MOTER_R_LOCK, LOW);
    nextSetTimeR = millis() + 150;
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
    setGo(-speed, speed);
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
    setGo(speed, -speed);
  }
  else
  {
    setGo(0, 0);
  }
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

  ledcSetup(0, 500, 10);
  ledcAttachPin(MOTER_L_SPEED, 0);

  ledcSetup(1, 500, 10);
  ledcAttachPin(MOTER_R_SPEED, 1);

  for (int i = 0; magnaticSensor[i] != -1; i++)
  {
    pinMode(magnaticSensor[i], INPUT_PULLUP);
  }
}

unsigned long nextRun = 0;

void loop()
{
  Blynk.run();
  loopGo();

  // if (millis() >= nextRun)
  // {
  //   for (int i = 0; magnaticSensor[i] != -1; i++)
  //   {
  //     Serial.print(digitalRead(magnaticSensor[i]));
  //     Serial.print(" ");
  //   }
  //   Serial.println();
  //   nextRun += 100;
  // }
}
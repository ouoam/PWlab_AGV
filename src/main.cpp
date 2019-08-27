#include <Arduino.h>

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "T0PuHhtzvWlxXDoH7NDTbmUmF5XYEiSR";
char ssid[] = "AGV_Network";
char pass[] = "111222333444";

void go(int r, int l)
{
  ledcWrite(0, r > 0 ? r : -r);
  digitalWrite(26, r < 0);
  ledcWrite(1, l > 0 ? l : -l);
  digitalWrite(27, l > 0);
}

int rNow = 0;
int lNow = 0;
int rTarget;
int lTarget;

unsigned long nextSetTime = 0;

void setGo(int r, int l)
{
  rTarget = r;
  lTarget = l;
}

void loopGo()
{
  if (millis() > nextSetTime)
  {
    if (rTarget != rNow)
    {
      if (rTarget > rNow)
        rNow += 2;
      else
        rNow -= 2;
    }

    if (lTarget != lNow)
    {
      if (lTarget > lNow)
        lNow += 2;
      else
        lNow -= 2;
    }

    go(rNow, lNow);

    nextSetTime += 1;
  }
}

bool lBack = false;
bool rBack = false;

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt();
  setGo(rBack ? -pinValue : pinValue, lTarget);
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  setGo(rTarget, lBack ? -pinValue : pinValue);
}

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt();
  rBack = pinValue == 1;
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
  lBack = pinValue == 1;
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

void setup()
{
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass, IPAddress(10, 10, 0, 236), 8080);
  Blynk.syncAll();

  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);

  ledcSetup(0, 500, 10);
  ledcAttachPin(32, 0);

  ledcSetup(1, 500, 10);
  ledcAttachPin(33, 1);
}

void loop()
{
  Blynk.run();
  loopGo();
}
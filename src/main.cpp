#include <Arduino.h>

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char auth[] = "4305ebb3ece54cd7bb96f69eeee95f29";
const char ssid[] = "MikroTik-066735";
const char pass[] = "1234567890";

const int magnaticSensor[] = {0, 4, 16, 17, -1};

#define MOTER_L_SPEED 33
#define MOTER_L_REVERSE 27
#define MOTER_R_SPEED 32
#define MOTER_R_REVERSE 26

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

unsigned long nextSetTime = 0;

void setGo(int l, int r)
{
  lTarget = l;
  rTarget = r;
}

void loopGo()
{
  if (millis() > nextSetTime)
  {
    if (lTarget != lNow)
    {
      if (lTarget > lNow)
        lNow += 3;
      else
        lNow -= 3;
    }

    if (rTarget != rNow)
    {
      if (rTarget > rNow)
        rNow += 3;
      else
        rNow -= 3;
    }

    go(lNow, rNow);

    nextSetTime += 1;
  }
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
  pinMode(MOTER_R_REVERSE, OUTPUT);

  ledcSetup(0, 500, 10);
  ledcAttachPin(MOTER_L_SPEED, 0);

  ledcSetup(1, 500, 10);
  ledcAttachPin(MOTER_R_SPEED, 1);

  for (int i = 0; magnaticSensor[i] != -1; i++)
  {
    pinMode(magnaticSensor[i], INPUT);
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
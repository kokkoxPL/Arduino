#include "SMS_SS.h"
#include "HX711.h"

unsigned long czas;
unsigned long czas_numer2;
unsigned long czas_alarm;
unsigned long licznik;
bool dziecko = true;
bool dorosly = false;
bool ok = false;
char number[] = "+48xxxxxxxxx";
char message[] = "Hello World!";

void setup()
{
  Serial.begin(57600);
  reset_timers();
}

void loop()
{
  delay(500);
  licznik = millis() - licznik;

  if (dziecko && !dorosly)
  {
    czas += licznik;
  }
  else
  {
    reset_timers();
  }

  if (czas > 10000)
  {
    if (ok)
    {
      reset_timers();
      Serial.println("reset");
    }

    if (czas_alarm > 10000)
    {
      Serial.println("alarm");
      ok = true;
    }
    else if (czas_numer2 > 10000)
    {
      Serial.println("sms2");
      czas_alarm += licznik;
    }
    else if (czas > 10000)
    {
      Serial.println("sms1");
      czas_numer2 += licznik;
    }
  }

  Serial.print("licznik: ");
  Serial.print(licznik);
  Serial.print(" czas: ");
  Serial.print(czas);
  Serial.print(" czas_numer2: ");
  Serial.print(czas_numer2);
  Serial.print(" czas_alarm: ");
  Serial.println(czas_alarm);
  
  licznik = millis();
}

void reset_timers()
{
  licznik = 0;
  czas = 0;
  czas_numer2 = 0;
  czas_alarm = 0;
}

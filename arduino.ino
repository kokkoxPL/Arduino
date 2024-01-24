#include "SMS_SS.h"
#include "HX711.h"

const unsigned int REED = 10;
const unsigned int ALARM = 13;
const unsigned int time_between_loop = 500;
const unsigned int time_max = 180000;
unsigned long timer = 0;
unsigned long timer_number2 = 0;
unsigned long timer_alarm = 0;
bool sendingSMS1 = false;
bool sendingSMS2 = false;
char number1[] = "+48xxxxxxxxx";
char number2[] = "+48xxxxxxxxx";
char message[] = "W aucie zostalo dziecko!!! Wroc do auta lub zadzwon na ten numer aby oznac ze wszystko jest dobrze.";

void setup()
{
  pinMode(REED, INPUT_PULLUP);
  pinMode(ALARM, OUTPUT);

  Serial.begin(57600);

  setupHX711();
  setupSMS();
}

void loop()
{
  delay(time_between_loop);

  if (checkLoadCells())
  {
    timer += time_between_loop;
  }
  else
  {
    reset_timers();
  }

  if (timer > time_max || sendingSMS1 || sendingSMS2)
  {
    if (getcallStatus() == 4)
    {
      hangup();
      reset_timers();
      sendingSMS1 = false;
      sendingSMS2 = false;
      Serial.println("reset");
    }

    if (timer_alarm > time_max)
    {
      Serial.println("alarm");
      tone(ALARM, 1000);
      delay(500);
      noTone(ALARM);
      delay(100);
    }
    else if (timer_number2 > time_max)
    {
      if (!sendingSMS2)
      {
        Serial.println("sms2");
        sendMessage(number2, message);
        sendingSMS2 = true;
      }

      timer_alarm += time_between_loop;
    }
    else if (timer > 300000)
    {
      if (!sendingSMS1)
      {
        Serial.println("sms1");
        sendMessage(number1, message);
        sendingSMS1 = true;
      }

      timer_number2 += time_between_loop;
    }
  }

  Serial.print("licznik: ");
  Serial.print(time_between_loop);
  Serial.print(" czas: ");
  Serial.print(timer);
  Serial.print(" czas_numer2: ");
  Serial.print(timer_number2);
  Serial.print(" czas_alarm: ");
  Serial.println(timer_alarm);
}

void reset_timers()
{
  timer = 0;
  timer_number2 = 0;
  timer_alarm = 0;
}
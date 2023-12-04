void setupSMS()
{
  agsmSerial.begin(9600);
  clearagsmSerial();
  clearSerial();
  delay(10);

  modemHWSetup(); // configure Arduino IN and OUT to be used with modem

  Serial.flush();
  agsmSerial.flush();
  delay(1000);
  Serial.println(F("a-gsmII/b-gsmgnss SMS send/read/list SMS example"));
  Serial.flush();

  Serial.println(F("seat back and relax until your a-gsmII/b-gsmgnss is ready"));
  delay(100);

  powerOnModem();

  clearBUFFD();
  while (strlen(buffd) < 1)
  {
    getIMEI();
    delay(500);
  }

  ready4SMS = 0;
  ready4Voice = 0;

  Serial.println(F("a-gsmII/b-gsmgnss ready.. let's run the example"));
  Serial.print(F("a-gsmII/b-gsmgnss IMEI: "));
  Serial.flush();
  Serial.println(buffd);
  Serial.flush();
  // setAUDIOchannel(20);
  delay(1000);
}

void sendMessage(char destinationNumber[], char message[])
{
  int SMSindex = 1;
  char *pch0;
  char *pch1;
  int res;
  int i;
  int smslen = 0;
  bool loop = true;

  while(loop) {
    switch (state)
    {
    case 0: // check modem status
      if (!getModemState())
        restartMODEM();
      else
        state++;
      i = 0;
      res = 0;
      while (res != 1)
      {
        res = sendATcommand("", "OK", "ERROR", 2);
        if (res != 1)
        {
          if (i++ >= 10)
          {
            printDebugLN(F("AT err...restarting"));
            restartMODEM();
          }
        }
        delay(500);
      }
      sendATcommand("+IPR=0;&w", "OK", "ERROR", 2);
      delay(2000);
      break;

    case 1:
      clearBUFFD();
      // next some init strings...
      aGsmCMD("AT+QIMODE=0", 200);
      aGsmCMD("AT+QINDI=0", 200);
      aGsmCMD("AT+QIMUX=0", 200);
      aGsmCMD("AT+QIDNSIP=0", 200);
      offsetTime = 0;
      clearBUFFD();
      state++;
      break;

    case 2:
      printDebugLN(F("try CPIN..."));
      if (!offsetTime)
        offsetTime = millis();
      if ((millis() - offsetTime) > 20000)
        restartMODEM();
      if (sendATcommand("+CPIN?", "READY") == 1)
      {
        offsetTime = 0;
        state++;
        printDebugLN(F("READY"));
      }
      else
      {
      }
      clearagsmSerial();
      delay(100);
      offsetTime = millis();
      break;

    case 3:
      if (!offsetTime)
        offsetTime = millis();
      if ((millis() - offsetTime) > 30000)
        restartMODEM();

      printDebugLN(F("Query GSM registration?"));
      res = registration(GSM);
      if (res == 1)
      {
        offsetTime = 0;
        state++;
        printDebugLN(F("READY, HOME NETWORK"));
      }
      else if (res == 5)
      {
        offsetTime = 0;
        state++;
        printDebugLN(F("READY, ROAMING"));
      }
      else
      {
        Serial.print(F("."));
      }
      offsetTime = millis();
      break;

    case 4: // init SIM/MODEM
      printDebugLN(F("Query State of Initialization"));
      if (sendATcommand("+QINISTAT", "3") == 1)
      {
        offsetTime = 0;
        state++;
        printDebugLN(F("READY"));
      }
      else
      {
        Serial.print(F("."));
        delay(100);
      }
      clearagsmSerial();
      delay(100);
      offsetTime = millis();
      break;

    case 5: // let's prepare modem for SMS usage
      if (!offsetTime)
        offsetTime = millis();
      if ((millis() - offsetTime) > 5000)
        restartMODEM();
      clearBUFFD();
      clearagsmSerial();

      printDebugLN(F("Prepare the modem for SMS usage!"));
      setupMODEMforSMSusage();
      delay(10000);
      offsetTime = millis();
      state++;
      break;

    case 6: // let's send one SMS to the destination receipment
      if (!offsetTime)
        offsetTime = millis();
      if ((millis() - offsetTime) > 5000)
        restartMODEM();

      printDebugLN(F("Let's send one SMS..."));

      // sendSMS(destinationNumber, message);

      // if(sendSMS(destinationNumber,message,"147")==1) printDebugLN("test SMS has been send with succees!");

      // in real app you would like to use it like bellow
      if(sendSMS(destinationNumber,message) == 1) {
        printDebugLN("SMS SUCCESS");
      }
      else {
        printDebugLN("SMS FAILURE");
      }

      clearBUFFD();
      clearagsmSerial();

      delay(10000);
      offsetTime = millis();

      state++;
      break;

    default:
      loop = false;
      // restartMODEM();
      printDebugLN(F("That's all folks!"));
      delay(10000);
      // state=0;
      break;
    }
  }
}

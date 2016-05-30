#include <Arduino.h>
#include <SoftwareSerial.h>
#include <AFMotor.h>
#include "esp8266udp.h"
#include "net.h"
#include "camera.h"
#include "cdroid_core.h"

// (Arduino_RX, Arduino_TX)
SoftwareSerial espSerial(A9, A8);

Esp8266Udp esp(&espSerial, 0, &Serial);

#define CDROID_AP_SSID "CDROID"
#define CDROID_AP_PASS "12345678"
#define CDROID_UDP_PORT 1234

void setup()
{
  Serial.begin(9600);
  espSerial.begin(115200);
  espSerial.println("AT+UART_CUR=19200,8,1,0,0");
  delay(100);
  while (espSerial.available() > 0)
  {
    espSerial.read();
  }
  espSerial.begin(19200);
  
  esp.begin();
  netInit(&esp, &Serial);
  
  bool test = false;
  test = esp.CheckAt();
  if (test)
  {
    Serial.println("CheckAt OK");
  }
  else
  {
    Serial.println("CheckAt FAIL");
  }
  
  test = esp.SetAp(CDROID_AP_SSID, CDROID_AP_PASS, 5, ENC_WPA2_PSK);
  if (test)
  {
    Serial.println("SetAp OK");
  }
  else
  {
    Serial.println("SetAp FAIL");
  }
  
  test = esp.OpenUdp(CDROID_UDP_PORT);
  if (test)
  {
    Serial.println("OpenUdp OK");
  }
  else
  {
    Serial.println("OpenUdp FAIL");
  }
  
  if (test == false)
  {
    test = esp.CloseUdp();
    if (test)
    {
      Serial.println("CloseUdp OK");
    }
    else
    {
      Serial.println("CloseUdp FAIL");
    }
  }
  
  test = esp.OpenUdp(CDROID_UDP_PORT);
  if (test)
  {
    Serial.println("OpenUdp OK");
  }
  else
  {
    Serial.println("OpenUdp FAIL");
  }
  
  Serial.println("Setup done");
  
  cameraInit(&Serial);
  
  cdroidInit(255);
  /*
  // debug
  CDroidInput testInput;
  testInput.Forward = true;
  testInput.Speed = 255;
  cdroidProcessInput(&testInput);*/
}

void loop()
{
  netLoop();
  cdroidLoop();
}

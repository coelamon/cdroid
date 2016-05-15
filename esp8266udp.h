#ifndef ESP8266UDP_H
#define ESP8266UDP_H

#include <Arduino.h>
#include <Stream.h>
#include <IPAddress.h>
#include "NetPacket.h"

#define ATRESPONSE_OK "OK"
#define ATRESPONSE_ERROR "ERROR"
#define ATRESPONSE_FAIL "FAIL"
#define ATRESPONSE_SENDOK "SEND OK"
#define ATRESPONSE_SENDFAIL "SEND FAIL"
#define ATRESPONSE_ALREADYCONNECT "ALREADY CONNECT"

enum EncriptionMode
{
  ENC_OPEN         = 0,
  ENC_WEP          = 1,
  ENC_WPA_PSK      = 2,
  ENC_WPA2_PSK     = 3,
  ENC_WPA_WPA2_PSK = 4
};

class Esp8266Udp;

typedef void (*EspReceiveCallback)(Esp8266Udp *espUdp, int length, IPAddress *remoteIp, unsigned int remotePort);

class Esp8266Udp {
public:
  Stream *EspStream;
  Esp8266Udp(Stream *espStream, EspReceiveCallback recvCallback, Stream *debugStream);
  void begin();
  // returns true if no error
  bool SendPacket(NetPacket *packet, IPAddress *remoteIp, unsigned int remotePort);
  // returns true if no error
  bool SetAp(char *ssid, char *pass, int chn, int enc);
  // returns true if no error
  bool OpenUdp(int port);
  // returns true if no error
  bool CloseUdp();
  // returns true if no error
  bool CheckAt();

  void ReadAllAvailable(int timeout);
  
  void SetReceiveCallback(EspReceiveCallback recvCallback);
private:
  Stream *dbgStream;
  EspReceiveCallback recvCallback;
  
  void debugMessage(char* msg);

  // returns 0 if no problem
  // returns 1 if timeout
  // returns 2 if overflow
  // returns 3 if argument exception
  int readUntil(int *outlen, char *outstopchar, char *buf, int buflen, int timeout, const char *stopchars, int stopcharslen);
  
  // returns true if ok
  bool readIpdParameters(int *datalen, IPAddress *remoteip, unsigned int *remoteport);
  
  // it will read full line and trash overflow
  // returns 0 if no problem
  // returns 1 if timeout
  // returns 2 if overflow
  // returns 3 if argument exception
  // buflen must be at least 40
  int readLine(char *buf, int buflen, int timeout, bool skipEmptyLines);
  void sendCommand(char *cmd);
  // returns true if no error
  bool execute(char *cmd);
  // returns +1 if ok
  // returns 0 if unknown
  // returns -1 if error
  int getResponseStatus(char *response);
  
  // returns true if no error
  bool echoOn();
  // returns true if no error
  bool echoOff();
  
  // wifi
  // returns true if no error
  bool setWifiMode(int mode);
};

#endif

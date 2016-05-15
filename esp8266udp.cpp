#include "esp8266udp.h"

Esp8266Udp::Esp8266Udp(Stream *stream, EspReceiveCallback recvCb, Stream *debugStream)
{
  recvCallback = recvCb;
  EspStream = stream;
  dbgStream = debugStream;
}

void Esp8266Udp::SetReceiveCallback(EspReceiveCallback recvCb)
{
  recvCallback = recvCb;
}

void Esp8266Udp::begin()
{
  echoOff();
}

void Esp8266Udp::debugMessage(char* msg)
{
  if (dbgStream == 0)
    return;
  dbgStream->print("[debug] ");
  dbgStream->print(msg);
  dbgStream->println();
}

int Esp8266Udp::readUntil(int *outlen, char *outstopchar, char *buf, int buflen, int timeout, const char *stopchars, int stopcharslen)
{
  if (buf == 0)
  {
    return 3; // argument exception
  }
  if (stopchars == 0)
  {
    return 3; // argument exception
  }
  if (stopcharslen < 1)
  {
    return 3; // argument exception
  }
  int bufidx = 0;
  unsigned long start_time = millis();
  while ((EspStream->available() > 0) || (millis() < (start_time + timeout)))
  {
    if (EspStream->available())
    {
      start_time = millis();
      char c = EspStream->read();
      if (bufidx < buflen)
      {
        buf[bufidx] = c;
        bufidx++;
      }
      else
      {
        bufidx = buflen + 1;
      }
      bool isstopchar = false;
      for (int i = 0; i < stopcharslen; i++)
      {
        if (c == stopchars[i])
        {
          isstopchar = true;
          break;
        }
      }
      if (isstopchar == true)
      {
        if (outstopchar != 0)
        {
          *outstopchar = c;
        }
        if (bufidx > buflen)
        {
          if (outlen != 0)
          {
            *outlen = buflen - 1;
          }
          buf[buflen-1] = 0;
          return 2; // overflow
        }
        buf[bufidx-1] = 0;
        return 0; // ok
      }
    }
    else
    {
      delay(1);
    }
  }
  return 1; // timeout
}

// source: https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/IPAddress.cpp
// but modified
bool ipaddressFromString(IPAddress *outaddr, const char *address)
{
  if (outaddr == 0) {
    return false;
  }
  uint16_t acc = 0; // Accumulator
  uint8_t dots = 0;
  while (*address)
  {
    char c = *address++;
    if (c >= '0' && c <= '9')
    {
      acc = acc * 10 + (c - '0');
      if (acc > 255) {
        // Value out of [0..255] range
        return false;
      }
    }
    else if (c == '.')
    {
      if (dots == 3) {
        // Too much dots (there must be 3 dots)
        return false;
      }
      (*outaddr)[dots++] = acc;
      acc = 0;
    }
    else
    {
      // Invalid char
      return false;
    }
  }
  if (dots != 3) {
    // Too few dots (there must be 3 dots)
    return false;
  }
  (*outaddr)[3] = acc;
  return true;
}

bool Esp8266Udp::readIpdParameters(int *datalen, IPAddress *remoteip, unsigned int *remoteport)
{
  char ipdbuf[20];
  int read_ret = 0;
  char stopchar = 0;
  int readlen = 0;
  read_ret = readUntil(&readlen, &stopchar, ipdbuf, 20, 10, ",:", 2);
  if (read_ret != 0)
  {
    return false;
  }
  int local_datalen = atoi(ipdbuf);
  if (datalen != 0)
  {
    *datalen = local_datalen;
  }
  read_ret = readUntil(&readlen, &stopchar, ipdbuf, 20, 10, ",:", 2);
  if (read_ret != 0)
  {
    return false;
  }
  IPAddress local_remoteip;
  if (ipaddressFromString(&local_remoteip, ipdbuf) == false)
  {
    while (EspStream->available() > 0)
    {
      EspStream->read();
    }
    return 1; // timeout
  }
  if (remoteip != 0)
  {
    *remoteip = local_remoteip;
  }
  read_ret = readUntil(&readlen, &stopchar, ipdbuf, 20, 10, ",:", 2);
  if (read_ret != 0)
  {
    return false;
  }
  unsigned int local_remoteport = atoi(ipdbuf);
  if (remoteport != 0)
  {
    *remoteport = local_remoteport;
  }
  if (stopchar != ':')
  {
    return false;
  }
  return true;
}

int Esp8266Udp::readLine(char *buf, int buflen, int timeout, bool skipEmptyLines)
{
  if (buflen < 40)
  {
    return 3; // argument exception
  }
  int bufidx = 0;
  unsigned long start_time = millis();
  while ((EspStream->available() > 0) || (millis() < (start_time + timeout)))
  {
    if (EspStream->available())
    {
      start_time = millis();
      char c = EspStream->read();
      if (bufidx < buflen)
      {
        buf[bufidx] = c;
        bufidx++;
      }
      else
      {
        bufidx = buflen + 1;
      }
      
      if (bufidx == 5 && strncmp(buf, "+IPD,", 5) == 0)
      {
        int datalen = 0;
        IPAddress remoteip = INADDR_NONE;
        unsigned int remoteport = 0;
        
        if (readIpdParameters(&datalen, &remoteip, &remoteport) == false)
        {
                                        debugMessage("readIpdParameters: fail");
          while (EspStream->available() > 0)
          {
            EspStream->read();
          }
          return 1; // timeout
        }
        
        if (EspStream->available() < datalen)
        {
          delay(timeout); // wait some time
          start_time = millis();
        }

        if (EspStream->available() >= datalen)
        {
          if (recvCallback != 0)
          {
            recvCallback(this, datalen, &remoteip, remoteport);
          }
          else
          {
            for (int i = 0; i < datalen; i++)
            {
              EspStream->read();
            }
          }
        }
        else
        {
          debugMessage("+IPD: not enough available data");
          while (EspStream->available() > 0)
          {
            EspStream->read();
          }
          return 1; // timeout
        }
        start_time = millis();
        bufidx = 0;
        continue;
      }
      
      if (c == 13 || c == 10)
      {
        if (bufidx == 1 && skipEmptyLines)
        {
          start_time = millis();
          bufidx = 0;
          continue;
        }
        if (bufidx > buflen)
        {
          buf[buflen-1] = 0;
          debugMessage("readLine: overflow");
          return 2; // overflow
        }
        buf[bufidx-1] = 0;
        return 0; // ok
      }
    }
    else
    {
      delay(1);
    }
  }
  return 1; // timeout
}

int Esp8266Udp::getResponseStatus(char *response)
{
  int ret = 0;
  if (strcmp(response, ATRESPONSE_OK) == 0)
    ret = +1;
  if (strcmp(response, ATRESPONSE_SENDOK) == 0)
    ret = +1;
  
  if (strcmp(response, ATRESPONSE_ERROR) == 0)
    ret = -1;
  if (strcmp(response, ATRESPONSE_FAIL) == 0)
    ret = -1;
  if (strcmp(response, ATRESPONSE_SENDFAIL) == 0)
    ret = -1;
  if (strcmp(response, ATRESPONSE_ALREADYCONNECT) == 0)
    ret = -1;
  
  if (ret != 0 && dbgStream != 0)
  {
    char buf[256];
    sprintf(buf, "getResponseStatus: %s => %d", response, ret);
    debugMessage(buf);
  }

  return ret;
}

void Esp8266Udp::sendCommand(char *cmd)
{
  char *c = cmd;
  while (*c != 0)
  {
    EspStream->write(*c);
    c++;
  }
  EspStream->write(13);
  EspStream->write(10);
}

void Esp8266Udp::ReadAllAvailable(int timeout)
{
  char buf[256];
  int buflen = 256;
  while (true)
  {
    int read_ret = readLine(buf, buflen, timeout, true);
    if (read_ret == 1 || read_ret == 3) // timeout, argument exception
    {
      return;
    }
  }
}

bool Esp8266Udp::execute(char *cmd)
{
  ReadAllAvailable(10);
  char buf[256];
  int buflen = 256;
  sendCommand(cmd);
  while (true)
  {
    int read_ret = readLine(buf, buflen, 1000, true);
    if (read_ret == 1 || read_ret == 3) // timeout, argument exception
    {
      return false;
    }
    int resp_status = getResponseStatus(buf);
    if (resp_status == +1)
    {
      return true;
    }
    if (resp_status == -1)
    {
      return false;
    }
  }
}

bool Esp8266Udp::echoOff()
{
  return execute("ATE0");
}

bool Esp8266Udp::echoOn()
{
  return execute("ATE1");
}

bool Esp8266Udp::setWifiMode(int mode)
{
  if (mode < 1)
    return false;
  if (mode > 3)
    return false;
  char buf[20];
  sprintf(buf, "AT+CWMODE=%d", mode);
  return execute(buf);
}

bool Esp8266Udp::CheckAt()
{
  return execute("AT");
}

bool Esp8266Udp::OpenUdp(int port)
{
  if (execute("AT+CIPDINFO=1") == false)
  {
    return false;
  }
  char buf[40];
  sprintf(buf, "AT+CIPSTART=\"UDP\",\"0.0.0.0\",0,%d,2", port);
  return execute(buf);
}

bool Esp8266Udp::CloseUdp()
{
  return execute("AT+CIPCLOSE");
}

bool Esp8266Udp::SetAp(char *ssid, char *pass, int chn, int enc)
{
  char buf[100];
  sprintf(buf, "AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d", ssid, pass, chn, enc);
  return execute(buf);
}

bool Esp8266Udp::SendPacket(NetPacket *packet, IPAddress *remoteIp, unsigned int remotePort)
{
  if (packet == 0)
  {
    return false;
  }
  char ipbuf[20];
  sprintf(ipbuf, "%d.%d.%d.%d", (*remoteIp)[0], (*remoteIp)[1], (*remoteIp)[2], (*remoteIp)[3]);
  char buf[100];
  int buflen = 100;
  sprintf(buf, "AT+CIPSEND=%d,\"%s\",%u", packet->size(), ipbuf, remotePort);
  bool cmd_send_result = execute(buf);
  if (cmd_send_result == false)
  {
    debugMessage(buf);
    return false;
  }
  
  int read_ret = readUntil(0, 0, buf, buflen, 10, ">", 1);
  if (read_ret != 0)
  {
    debugMessage("couldn't get '> '");
    return false;
  }
  EspStream->read(); // read ' '
  char *msgbuf = packet->getBuffer();
  int msglen = packet->size();
  for (int i = 0; i < msglen; i++)
  {
    EspStream->write(msgbuf[i]);
  }
  
  while (true)
  {
    read_ret = readLine(buf, buflen, 1000, true);
    if (read_ret == 1 || read_ret == 3) // timeout, argument exception
    {
      return false;
    }
    int resp_status = getResponseStatus(buf);
    if (resp_status == +1)
    {
      return true;
    }
    if (resp_status == -1)
    {
      return false;
    }
  }
}

#include "net.h"
#include "cdroid_core.h"

Esp8266Udp *netEspUdp;
Stream *netDebugStream;

bool clientConnected = false;
IPAddress clientIp;
unsigned int clientPort;
unsigned long lastClientMessageTime;

void netProcessMessage(Esp8266Udp *espUdp, IPAddress *remoteIp, unsigned int remotePort, unsigned int msgid, unsigned int datalen, char *data);

void netInit(Esp8266Udp *espUdp, Stream *dbgStream)
{
  netEspUdp = espUdp;
  netDebugStream = dbgStream;
}

bool checkSignature(char *sig)
{
  if (sig[0] == 'C' && sig[1] == 'D' && sig[2] == 'R' && sig[3] == 'D')
  {
    return true;
  }
  else
  {
    return false;
  }
}

void netReceiveCallback(Esp8266Udp *espUdp, int length, IPAddress *remoteIp, unsigned int remotePort)
{
  if (netDebugStream != 0)
  {
    netDebugStream->print("[debug] netReceiveCallback receives ");
    netDebugStream->print(length);
    netDebugStream->print(" bytes from ");
    char ip_str[20];
    sprintf(ip_str, "%d.%d.%d.%d", (*remoteIp)[0], (*remoteIp)[1], (*remoteIp)[2], (*remoteIp)[3]);
    netDebugStream->print(ip_str);
    netDebugStream->print(":");
    netDebugStream->print(remotePort);
    netDebugStream->println();
  } 
  if (clientConnected)
  {
    if (*remoteIp != clientIp)
    {
      for (int i = 0; i < length; i++)
      {
        espUdp->EspStream->read();
      }
      if (netDebugStream != 0)
      {
        netDebugStream->print("[debug] netReceiveCallback: wrong ip");
        netDebugStream->println();
      }
      return;
    }
  }
  if (length < 8 || length > 128)
  {
    for (int i = 0; i < length; i++)
    {
      espUdp->EspStream->read();
    }
    return;
  }
  char signature[4];
  unsigned int msgid;
  unsigned int msgdatalen;
  char data[128];
  for (int i = 0; i < 4; i++)
  {
    signature[i] = espUdp->EspStream->read();
  }
  if (!checkSignature(signature)) // error
  {
    for (int i = 0; i < length-4; i++)
    {
      espUdp->EspStream->read();
    }
    if (netDebugStream != 0)
    {
      netDebugStream->print("[debug] netReceiveCallback receives wrong signature");
      netDebugStream->println();
    }
    return;
  }
  msgid = (unsigned int)espUdp->EspStream->read();
  msgid |= (unsigned int)espUdp->EspStream->read() << 8;
  msgdatalen = (unsigned int)espUdp->EspStream->read();
  msgdatalen |= (unsigned int)espUdp->EspStream->read() << 8;
  if (msgdatalen != length-8) // error
  {
    for (int i = 0; i < length-8; i++)
    {
      espUdp->EspStream->read();
    }
    if (netDebugStream != 0)
    {
      netDebugStream->print("[debug] netReceiveCallback: msgdatalen != length-8");
      netDebugStream->println();
    }
    return;
  }
  for (int i = 0; i < msgdatalen; i++)
  {
	data[i] = espUdp->EspStream->read();
  }
  netProcessMessage(espUdp, remoteIp, remotePort, msgid, msgdatalen, data);
}

void netSendMessage(Esp8266Udp *espUdp, IPAddress *remoteIp, unsigned int remotePort, unsigned int msgid, unsigned int msgdatalen, char *msgdata)
{
  char buf[128];
  if (msgdatalen > 120)
  {
    return;
  }
  if (msgdatalen > 0 && msgdata == 0)
  {
    return;
  }
  NetPacket packet(buf, 128);
  packet.write('C');
  packet.write('D');
  packet.write('R');
  packet.write('D');
  packet.write(msgid & 0xFF);
  packet.write((msgid >> 8) & 0xFF);
  packet.write(msgdatalen & 0xFF);
  packet.write((msgdatalen >> 8) & 0xFF);
  for (int i = 0; i < msgdatalen; i++)
  {
    packet.write(msgdata[i]);
  }
  espUdp->SendPacket(&packet, remoteIp, remotePort);
}

void netProcessMessage(Esp8266Udp *espUdp, IPAddress *remoteIp, unsigned int remotePort, unsigned int msgid, unsigned int datalen, char *data)
{
  if (!clientConnected)
  {
    if (msgid == NETMSG_LOGIN)
    {
      if (datalen != 32)
      {
        netSendMessage(espUdp, remoteIp, remotePort, NETMSG_AUTH_FAIL, 0, 0);
        if (netDebugStream != 0)
        {
          netDebugStream->print("[debug] netProcessMessage: wrong data length");
          netDebugStream->println();
        }
        return;
      }
      if (strncmp(data, CDROID_PASSWORD, 32) == 0)
      {
	clientConnected = true;
	clientIp = *remoteIp;
	clientPort = remotePort;
	lastClientMessageTime = millis();
	netSendMessage(espUdp, &clientIp, clientPort, NETMSG_AUTH_OK, 0, 0);
        if (netDebugStream != 0)
        {
          netDebugStream->print("[debug] netProcessMessage receives right password");
          netDebugStream->println();
        }
        return;
      }
      else
      {
        netSendMessage(espUdp, remoteIp, remotePort, NETMSG_AUTH_FAIL, 0, 0);
        if (netDebugStream != 0)
        {
          netDebugStream->print("[debug] netProcessMessage receives wrong password: ");
          char buf[33];
          for (int i = 0; i < 32; i++)
          {
            buf[i] = data[i];
          }
          buf[32] = 0;
          netDebugStream->print(buf);
          netDebugStream->println();
        }
        return;
      }
    }
    else
    {
      if (netDebugStream != 0)
      {
        netDebugStream->print("[debug] netProcessMessage receives wrong message: ");
        netDebugStream->print(msgid);
        netDebugStream->println();
      }
    }
  }
  else
  {
    if (msgid == NETMSG_CLIENT_DISCONNECT)
	{
	  clientConnected = false;
	  return;
	}
    else if (msgid == NETMSG_CDROID_INPUT)
	{
	  if (datalen != 5)
	  {
	    return;
	  }
	  lastClientMessageTime = millis();
	  CDroidInput input;
	  input.Forward = data[0];
	  input.Backward = data[1];
	  input.Left = data[2];
	  input.Right = data[3];
	  input.Speed = data[4];
	  cdroidProcessInput(&input);
	  return;
	}
    else if (msgid == NETMSG_CAPTURE_SHOT)
	{
	  return;
	}
    else
    {
      if (netDebugStream != 0)
      {
        netDebugStream->print("[debug] netProcessMessage receives wrong message: ");
        netDebugStream->print(msgid);
        netDebugStream->println();
      }
    }
  }
}

void netLoop()
{
  netEspUdp->SetReceiveCallback(netReceiveCallback);
  netEspUdp->ReadAllAvailable(100);
  
  if (clientConnected)
  {
    if (lastClientMessageTime + CLIENT_CONNECTION_TIMEOUT < millis())
    {
      netSendMessage(netEspUdp, &clientIp, clientPort, NETMSG_CDROID_DISCONNECT, 0, 0);
      clientConnected = false;
    }
  }
}

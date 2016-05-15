#include "NetPacket.h"

NetPacket::NetPacket(char *initbuf, int buflen)
{
  msgbuf = initbuf;
  msgbuflen = buflen;
  pos = 0;
  msglen = 0;
  overflowflag = false;
}

char* NetPacket::getBuffer()
{
  return msgbuf;
}

int NetPacket::size()
{
  return msglen;
}

int NetPacket::tell()
{
  return pos;
}

void NetPacket::seek(int newpos)
{
  overflowflag = false;
  if (newpos < 0)
  {
    pos = 0;
	return;
  }
  if (newpos <= msglen)
  {
    pos = newpos;
  }
  else
  {
    pos = msglen;
  }
}

bool NetPacket::getOverflowFlag()
{
  return overflowflag;
}

void NetPacket::write(char c)
{
  if (msgbuf == 0)
  {
    overflowflag = true;
	return;
  }
  if (pos >= msgbuflen)
  {
    overflowflag = true;
	return;
  }
  msgbuf[pos] = c;
  pos++;
  if (pos > msglen)
  {
	msglen = pos;
  }
}

char NetPacket::read()
{
  if (msgbuf == 0)
  {
	return 0;
  }
  if (pos >= msglen)
  {
    return 0;
  }
  char ret = msgbuf[pos];
  pos++;
  return ret;
}

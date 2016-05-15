#ifndef NET_PACKET_H
#define NET_PACKET_H

class NetPacket
{
private:
  char *msgbuf;
  int msgbuflen;
  int pos;
  int msglen;
  bool overflowflag;
public:
  NetPacket(char *initbuf, int buflen);
  char* getBuffer();
  int size();
  int tell();
  void seek(int newpos);
  bool getOverflowFlag();
  void write(char c);
  char read();
};

#endif


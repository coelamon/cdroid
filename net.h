#ifndef NET_H
#define NET_H

#include "cdroid_config.h"
#include "esp8266udp.h"
#include "net_msg.h"
#include "camera.h"

#define CLIENT_CONNECTION_TIMEOUT 10000

void netInit(Esp8266Udp *espUdp, Stream *dbgStream);
void netLoop();
void netShotBegin();
void netShotEnd();
void netShotPiece(int x, int y, int pixelnum, int datalen, char *data);

#endif


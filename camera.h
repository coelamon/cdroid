#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include "ov7670.h"
#include "net.h"

void cameraInit(Stream *dbgStream);
void cameraCaptureImage();

#endif

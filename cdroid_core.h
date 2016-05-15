#ifndef CDROID_CORE_H
#define CDROID_CORE_H

#include <Arduino.h>
#include <AFMotor.h>
#include "CDroidInput.h"
#include "cdroid_config.h"

void cdroidInit(int speed);
void cdroidProcessInput(CDroidInput *input);
void cdroidBeginCapture();
void cdroidLoop();

#endif

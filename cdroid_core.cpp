#include "cdroid_core.h"

AF_DCMotor motorFrontLeft(CDOIRD_MOTOR_FRONTLEFT);
AF_DCMotor motorFrontRight(CDOIRD_MOTOR_FRONTRIGHT);
AF_DCMotor motorBackLeft(CDOIRD_MOTOR_BACKLEFT);
AF_DCMotor motorBackRight(CDOIRD_MOTOR_BACKRIGHT);

int currentSpeed = -1;
bool lastInputTimeIsSet = false;
unsigned long lastInputTime = 0;

void cdroidSetSpeed(int speed);
void cdroidStop();
void cdroidRunForward();
void cdroidRunBackward();
void cdroidTurnLeft();
void cdroidTurnRight();

void cdroidInit(int speed)
{
  cdroidSetSpeed(speed);
}

void cdroidSetSpeed(int speed)
{
  motorFrontLeft.setSpeed(speed);
  motorFrontRight.setSpeed(speed);
  motorBackLeft.setSpeed(speed);
  motorBackRight.setSpeed(speed);
  currentSpeed = speed;
}

void cdroidStop()
{
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(RELEASE);
  motorBackLeft.run(RELEASE);
  motorBackRight.run(RELEASE);
}

void cdroidRunForward()
{
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(FORWARD);
  motorBackLeft.run(FORWARD);
  motorBackRight.run(FORWARD);
}

void cdroidRunBackward()
{
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(BACKWARD);
  motorBackLeft.run(BACKWARD);
  motorBackRight.run(BACKWARD);
}

void cdroidTurnLeft()
{
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(FORWARD);
  motorBackLeft.run(BACKWARD);
  motorBackRight.run(FORWARD);
}

void cdroidTurnRight()
{
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(BACKWARD);
  motorBackLeft.run(FORWARD);
  motorBackRight.run(BACKWARD);
}

void cdroidProcessInput(CDroidInput *input)
{
  if (input == 0)
  {
    return;
  }
  lastInputTimeIsSet = true;
  lastInputTime = millis();
  if (input->Speed != currentSpeed)
  {
    cdroidSetSpeed(input->Speed);
  }
  if (input->Forward)
  {
    cdroidRunForward();
	return;
  }
  if (input->Backward)
  {
    cdroidRunBackward();
	return;
  }
  if (input->Left)
  {
    cdroidTurnLeft();
	return;
  }
  if (input->Right)
  {
    cdroidTurnRight();
	return;
  }
  cdroidStop();
}

void cdroidBeginCapture()
{
  // TODO: capture camera shot and send
}

void cdroidLoop()
{
  if (lastInputTimeIsSet)
  {
    if ((millis() - lastInputTime) > CDROID_MOTOR_AUTO_STOP_TIME)
	{
	  lastInputTimeIsSet = false;
	  cdroidStop();
	}
  }
}

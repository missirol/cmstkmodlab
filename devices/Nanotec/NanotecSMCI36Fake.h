#ifndef __NANOTECSMCI36FAKE_H
#define __NANOTECSMCI36FAKE_H

#include "VNanotecSMCI36.h"

class NanotecSMCI36Fake : public VNanotecSMCI36
{
 public:

  NanotecSMCI36Fake( const ioport_t );
  ~NanotecSMCI36Fake();

 public:

  bool DeviceAvailable() const { return true; }

  std::string GetFirmwareVersion() const;
  int GetStatus() const;

  void SetMotorType(int type);
  int GetMotorType() const;

  void SetPhaseCurrent(int current);
  int GetPhaseCurrent() const;

  void SetStandStillPhaseCurrent(int current);
  int GetStandStillPhaseCurrent() const;

  void SetStepMode(int mode);
  int GetStepMode() const;

  void SetMotorID(int ID);
  int GetMotorID() const;

  void SetErrorCorrectionMode(int mode);
  int GetErrorCorrectionMode() const;

  void SetEncoderDirection(int direction);
  int GetEncoderDirection() const;

  void SetSwingOutTime(int time);
  int GetSwingOutTime() const;

  void SetMaxEncoderDeviation(int deviation);
  int GetMaxEncoderDeviation() const;

  void SetInputPinFunction(int pin, int function);
  int GetInputPinFunction(int pin) const;

  void SetOutputPinFunction(int pin, int function);
  int GetOutputPinFunction(int pin) const;

  void SetReversePolarityMask(int mask);
  int GetReversePolarityMask() const;

 private:

  int status_;
  int motorType_;
  int phaseCurrent_;
  int stepMode_;
  int motorID_;
  int errorCorrectionMode_;
  int encoderDirection_;
  int swingOutTime_;
  int maxEncoderDeviation_;
  int standStillPhaseCurrent_;
  int inputPinFunction_[7];
  int outputPinFunction_[4];
  int reversePolarityMask_;
};

#endif

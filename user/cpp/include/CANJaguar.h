/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2009-2016. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <atomic>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "CAN/can_proto.h"
#include "CANSpeedController.h"
#include "ErrorBase.h"
#include "FRC_NetworkCommunication/CANSessionMux.h"
#include "HAL/cpp/priority_mutex.h"
#include "LiveWindow/LiveWindowSendable.h"
#include "MotorSafety.h"
#include "MotorSafetyHelper.h"
#include "PIDOutput.h"
#include "tables/ITableListener.h"

namespace frc {

/**
 * Luminary Micro / Vex Robotics Jaguar Speed Control
 */
class CANJaguar : public MotorSafety,
                  public CANSpeedController,
                  public ErrorBase,
                  public LiveWindowSendable,
                  public ITableListener {
 public:
  // The internal PID control loop in the Jaguar runs at 1kHz.
  static const int kControllerRate = 1000;
  static constexpr double kApproxBusVoltage = 12.0;

  // Control mode tags
  /** Sets an encoder as the speed reference only. <br> Passed as the "tag" when
   * setting the control mode.*/
  static const struct EncoderStruct {
  } Encoder;
  /** Sets a quadrature encoder as the position and speed reference. <br> Passed
   * as the "tag" when setting the control mode.*/
  static const struct QuadEncoderStruct {
  } QuadEncoder;
  /** Sets a potentiometer as the position reference only. <br> Passed as the
   * "tag" when setting the control mode. */
  static const struct PotentiometerStruct {
  } Potentiometer;

  explicit CANJaguar(int deviceNumber);
  virtual ~CANJaguar();

  int getDeviceNumber() const;
  int GetHardwareVersion() const;

  // PIDOutput interface
  void PIDWrite(double output) override;

  // Control mode methods
  void EnableControl(double encoderInitialPosition = 0.0);
  void DisableControl();

  void SetPercentMode();
  void SetPercentMode(EncoderStruct, uint16_t codesPerRev);
  void SetPercentMode(QuadEncoderStruct, uint16_t codesPerRev);
  void SetPercentMode(PotentiometerStruct);

  void SetCurrentMode(double p, double i, double d);
  void SetCurrentMode(EncoderStruct, uint16_t codesPerRev, double p, double i,
                      double d);
  void SetCurrentMode(QuadEncoderStruct, uint16_t codesPerRev, double p,
                      double i, double d);
  void SetCurrentMode(PotentiometerStruct, double p, double i, double d);

  void SetSpeedMode(EncoderStruct, uint16_t codesPerRev, double p, double i,
                    double d);
  void SetSpeedMode(QuadEncoderStruct, uint16_t codesPerRev, double p, double i,
                    double d);

  void SetPositionMode(QuadEncoderStruct, uint16_t codesPerRev, double p,
                       double i, double d);
  void SetPositionMode(PotentiometerStruct, double p, double i, double d);

  void SetVoltageMode();
  void SetVoltageMode(EncoderStruct, uint16_t codesPerRev);
  void SetVoltageMode(QuadEncoderStruct, uint16_t codesPerRev);
  void SetVoltageMode(PotentiometerStruct);

  void Set(double value, int syncGroup);

  // CANSpeedController interface
  double Get() const override;
  void Set(double value) override;
  void Disable() override;
  void SetP(double p) override;
  void SetI(double i) override;
  void SetD(double d) override;
  void SetPID(double p, double i, double d) override;
  double GetP() const override;
  double GetI() const override;
  double GetD() const override;
  bool IsModePID(CANSpeedController::ControlMode mode) const override;
  double GetBusVoltage() const override;
  double GetOutputVoltage() const override;
  double GetOutputCurrent() const override;
  double GetTemperature() const override;
  double GetPosition() const override;
  double GetSpeed() const override;
  bool GetForwardLimitOK() const override;
  bool GetReverseLimitOK() const override;
  uint16_t GetFaults() const override;
  void SetVoltageRampRate(double rampRate) override;
  int GetFirmwareVersion() const override;
  void ConfigNeutralMode(NeutralMode mode) override;
  void ConfigEncoderCodesPerRev(uint16_t codesPerRev) override;
  void ConfigPotentiometerTurns(uint16_t turns) override;
  void ConfigSoftPositionLimits(double forwardLimitPosition,
                                double reverseLimitPosition) override;
  void DisableSoftPositionLimits() override;
  void ConfigLimitMode(LimitMode mode) override;
  void ConfigForwardLimit(double forwardLimitPosition) override;
  void ConfigReverseLimit(double reverseLimitPosition) override;
  void ConfigMaxOutputVoltage(double voltage) override;
  void ConfigFaultTime(double faultTime) override;
  virtual void SetControlMode(ControlMode mode);
  virtual ControlMode GetControlMode() const;

  static void UpdateSyncGroup(uint8_t syncGroup);

  void SetExpiration(double timeout) override;
  double GetExpiration() const override;
  bool IsAlive() const override;
  void StopMotor() override;
  bool IsSafetyEnabled() const override;
  void SetSafetyEnabled(bool enabled) override;
  void GetDescription(std::ostringstream& desc) const override;
  int GetDeviceID() const;

  // SpeedController overrides
  void SetInverted(bool isInverted) override;
  bool GetInverted() const override;

 protected:
  // Control mode helpers
  void SetSpeedReference(int reference);
  int GetSpeedReference() const;

  void SetPositionReference(int reference);
  int GetPositionReference() const;

  int packPercentage(uint8_t* buffer, double value);
  int packFXP8_8(uint8_t* buffer, double value);
  int packFXP16_16(uint8_t* buffer, double value);
  int packInt16(uint8_t* buffer, int16_t value);
  int packInt32(uint8_t* buffer, int32_t value);
  double unpackPercentage(uint8_t* buffer) const;
  double unpackFXP8_8(uint8_t* buffer) const;
  double unpackFXP16_16(uint8_t* buffer) const;
  int16_t unpackInt16(uint8_t* buffer) const;
  int32_t unpackInt32(uint8_t* buffer) const;

  void sendMessage(int messageID, const uint8_t* data, uint8_t dataSize,
                   int period = CAN_SEND_PERIOD_NO_REPEAT);
  void requestMessage(int messageID, int period = CAN_SEND_PERIOD_NO_REPEAT);
  bool getMessage(int messageID, uint32_t mask, uint8_t* data,
                  uint8_t* dataSize) const;

  void setupPeriodicStatus();
  void updatePeriodicStatus() const;

  mutable priority_recursive_mutex m_mutex;

  int m_deviceNumber;
  double m_value = 0.0f;

  // Parameters/configuration
  ControlMode m_controlMode = kPercentVbus;
  int m_speedReference = LM_REF_NONE;
  int m_positionReference = LM_REF_NONE;
  double m_p = 0.0;
  double m_i = 0.0;
  double m_d = 0.0;
  NeutralMode m_neutralMode = kNeutralMode_Jumper;
  uint16_t m_encoderCodesPerRev = 0;
  uint16_t m_potentiometerTurns = 0;
  LimitMode m_limitMode = kLimitMode_SwitchInputsOnly;
  double m_forwardLimit = 0.0;
  double m_reverseLimit = 0.0;
  double m_maxOutputVoltage = 30.0;
  double m_voltageRampRate = 0.0;
  float m_faultTime = 0.0f;

  // Which parameters have been verified since they were last set?
  bool m_controlModeVerified =
      false;  // Needs to be verified because it's set in the constructor
  bool m_speedRefVerified = true;
  bool m_posRefVerified = true;
  bool m_pVerified = true;
  bool m_iVerified = true;
  bool m_dVerified = true;
  bool m_neutralModeVerified = true;
  bool m_encoderCodesPerRevVerified = true;
  bool m_potentiometerTurnsVerified = true;
  bool m_forwardLimitVerified = true;
  bool m_reverseLimitVerified = true;
  bool m_limitModeVerified = true;
  bool m_maxOutputVoltageVerified = true;
  bool m_voltageRampRateVerified = true;
  bool m_faultTimeVerified = true;

  // Status data
  mutable float m_busVoltage = 0.0f;
  mutable float m_outputVoltage = 0.0f;
  mutable float m_outputCurrent = 0.0f;
  mutable float m_temperature = 0.0f;
  mutable double m_position = 0.0;
  mutable double m_speed = 0.0;
  mutable int m_limits = 0x00;
  mutable uint16_t m_faults = 0x0000;
  int m_firmwareVersion = 0;
  int m_hardwareVersion = 0;

  // Which periodic status messages have we received at least once?
  mutable std::atomic<bool> m_receivedStatusMessage0{false};
  mutable std::atomic<bool> m_receivedStatusMessage1{false};
  mutable std::atomic<bool> m_receivedStatusMessage2{false};

  bool m_controlEnabled = false;
  bool m_stopped = false;

  void verify();

  std::unique_ptr<MotorSafetyHelper> m_safetyHelper;

  void ValueChanged(ITable* source, llvm::StringRef key,
                    std::shared_ptr<nt::Value> value, bool isNew) override;
  void UpdateTable() override;
  void StartLiveWindowMode() override;
  void StopLiveWindowMode() override;
  std::string GetSmartDashboardType() const override;
  void InitTable(std::shared_ptr<ITable> subTable) override;
  std::shared_ptr<ITable> GetTable() const override;

  std::shared_ptr<ITable> m_table;

 private:
  void InitCANJaguar();
  bool m_isInverted = false;
};
}  // namespace frc

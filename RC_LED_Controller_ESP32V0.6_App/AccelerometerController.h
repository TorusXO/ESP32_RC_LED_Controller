#pragma once

#include <Arduino.h>
#include <Wire.h>

enum class EAccelerometerAxis : uint8_t
{
    X,
    Y,
    Z
};

struct FAccelerometerAxisConfiguration
{
    EAccelerometerAxis ForwardAxis = EAccelerometerAxis::X;
    bool bForwardAxisInverted = false;

    EAccelerometerAxis SideAxis = EAccelerometerAxis::Y;
    bool bSideAxisInverted = false;

    EAccelerometerAxis VerticalAxis = EAccelerometerAxis::Z;
    bool bVerticalAxisInverted = false;
};

struct FAccelerationState
{
    bool bHasValidSample = false;

    float ForwardAccelerationG = 0.0f;
    float FilteredForwardAccelerationG = 0.0f;
    float SideAccelerationG = 0.0f;
    float VerticalAccelerationG = 0.0f;
    float TotalDynamicAccelerationG = 0.0f;

    float GyroscopeXDps = 0.0f;
    float GyroscopeYDps = 0.0f;
    float GyroscopeZDps = 0.0f;
    float TotalAngularVelocityDps = 0.0f;

    uint32_t SampleTimeMs = 0;
};

class FAccelerometerController
{
public:
    FAccelerometerController();

    bool Begin(
        TwoWire& aWireRef,
        uint8_t aDeviceAddress = 0x68
    );

    bool SetAxisConfiguration(
        const FAccelerometerAxisConfiguration& aAxisConfigurationRef
    );

    void SetForwardToleranceG(float aToleranceG);

    bool CalibrateStationary(
        uint16_t aSampleCount = 200,
        uint16_t aSampleDelayMs = 5
    );

    bool Update(uint32_t aCurrentTimeMs);

    const FAccelerationState& GetState() const;

    bool IsConnected() const;
    bool IsCalibrated() const;
    uint8_t GetDeviceAddress() const;
    uint8_t GetWhoAmI() const;

    void ResetFilter();

private:
    bool ConfigureDevice();
    bool ReadRawMotion(
        int16_t& aRawAccelerationXRef,
        int16_t& aRawAccelerationYRef,
        int16_t& aRawAccelerationZRef,
        int16_t& aRawGyroscopeXRef,
        int16_t& aRawGyroscopeYRef,
        int16_t& aRawGyroscopeZRef
    );

    bool WriteRegister(
        uint8_t aRegisterAddress,
        uint8_t aValue
    );

    bool ReadRegister(
        uint8_t aRegisterAddress,
        uint8_t& aValueRef
    );

    float GetConfiguredAxisValue(
        EAccelerometerAxis aAxis,
        bool bAxisInverted,
        float aXG,
        float aYG,
        float aZG
    ) const;

    bool IsAxisConfigurationValid(
        const FAccelerometerAxisConfiguration& aAxisConfigurationRef
    ) const;

private:
    TwoWire* WirePtr = nullptr;
    uint8_t DeviceAddress = 0x68;
    uint8_t DeviceWhoAmI = 0;

    FAccelerometerAxisConfiguration AxisConfiguration;
    FAccelerationState AccelerationState;

    float StationaryOffsetXG = 0.0f;
    float StationaryOffsetYG = 0.0f;
    float StationaryOffsetZG = 0.0f;

    float StationaryGyroscopeOffsetXDps = 0.0f;
    float StationaryGyroscopeOffsetYDps = 0.0f;
    float StationaryGyroscopeOffsetZDps = 0.0f;
    float ForwardToleranceG = 0.02f;

    uint32_t LastSampleTimeMs = 0;

    bool bConnected = false;
    bool bCalibrated = false;
    bool bFilterInitialized = false;
};

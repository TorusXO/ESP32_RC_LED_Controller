#pragma once

#include <Arduino.h>

#include "ControllerConfiguration.h"

class BluetoothSerial;

class FControllerProtocol
{
public:
    FControllerProtocol();

    bool Begin(
        BluetoothSerial& aBluetoothSerialPortRef,
        FControllerConfiguration& aConfigurationRef
    );

    void Update();

    bool ConsumeConfigurationChanged();
    bool ConsumeExhaustTestRequested();

    void SendHello();
    void SendConfiguration();

    void SendTelemetry(
        uint32_t aCurrentTimeMs,
        bool aSteeringHasSignal,
        uint32_t aSteeringPulseUs,
        int16_t aSteeringPercent,
        bool aThrottleHasSignal,
        uint32_t aThrottlePulseUs,
        int16_t aThrottlePercent,
        float aForwardAccelerationG,
        float aFilteredForwardAccelerationG,
        float aSideAccelerationG,
        float aVerticalAccelerationG,
        float aGyroscopeXDps,
        float aGyroscopeYDps,
        float aGyroscopeZDps,
        const char* aTurnDirectionPtr,
        bool aBrakeActive,
        bool aExhaustPulseActive
    );


private:
    void HandleCommand(char* aCommandPtr);

    void HandleSetCommand(
        const char* aKeyPtr,
        const char* aValuePtr
    );

    void LoadConfiguration();
    bool SaveConfiguration();

    void SendAcknowledgement(
        const char* aKeyPtr,
        const char* aValuePtr
    );

    void SendError(
        const char* aErrorPtr
    );

private:
    static constexpr size_t COMMAND_BUFFER_SIZE = 192;

    BluetoothSerial* BluetoothSerialPortPtr = nullptr;
    FControllerConfiguration* ConfigurationPtr = nullptr;

    char CommandBuffer[COMMAND_BUFFER_SIZE] = {};
    size_t CommandLength = 0;

    bool bHadBluetoothClient = false;
    bool bConfigurationChanged = false;
    bool bExhaustTestRequested = false;
};

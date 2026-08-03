#pragma once

#include <Arduino.h>

#include "ControllerConfiguration.h"

struct FLightingInputState
{
    bool bSteeringHasSignal = false;
    int16_t SteeringPercent = 0;

    bool bThrottleHasSignal = false;
    int16_t ThrottlePercent = 0;

    bool bAccelerometerHasSample = false;
    float FilteredForwardAccelerationG = 0.0f;
};

enum class ETurnDirection : uint8_t
{
    None,
    Left,
    Right
};

class FLightingController
{
public:
    FLightingController();

    bool Begin();

    void ApplyConfiguration(
        const FControllerConfiguration& aConfigurationRef
    );

    void Update(
        const FLightingInputState& aInputStateRef,
        uint32_t aCurrentTimeMs
    );

    void TriggerExhaustTest(
        uint32_t aCurrentTimeMs
    );

    ETurnDirection GetActiveTurnDirection() const;
    bool IsBrakeOrReverseActive() const;
    bool IsExhaustPulseActive() const;

private:
    void UpdateTurnSignalState(
        const FLightingInputState& aInputStateRef,
        uint32_t aCurrentTimeMs
    );

    void UpdateRearLightState(
        const FLightingInputState& aInputStateRef
    );

    void UpdateExhaustState(
        const FLightingInputState& aInputStateRef,
        uint32_t aCurrentTimeMs
    );

    void TriggerExhaustPulse(
        uint32_t aCurrentTimeMs,
        uint32_t aDurationMs
    );

    void ApplyLightingOutputs(
        uint32_t aCurrentTimeMs
    );

    void ApplyPassiveOutputs();

    void ApplyExhaustOutputs(
        uint32_t aCurrentTimeMs
    );

    uint16_t MoveBrightnessTowardsTarget(
        uint16_t aCurrentBrightness,
        uint16_t aTargetBrightness,
        uint16_t aBrightnessRange,
        uint32_t aElapsedTimeMs,
        uint32_t aBrightenDurationMs,
        uint32_t aDimDurationMs
    ) const;

    uint16_t ConvertPercentToBrightness(
        uint8_t aBrightnessPercent
    ) const;

    void SetLedBrightness(
        uint8_t aChannel,
        uint16_t aBrightness
    );

    void SetRoleBrightness(
        uint8_t aRole,
        uint16_t aBrightness
    );

    void SetRoleServoPosition(
        uint16_t aPulse
    );

private:
    FControllerConfiguration Configuration;

    ETurnDirection ActiveTurnDirection =
        ETurnDirection::None;

    bool bBrakeOrReverseActive = false;
    bool bExhaustPulseActive = false;
    bool bManualExhaustPulseActive = false;

    int16_t PreviousThrottlePercent = 0;
    float PreviousFilteredForwardAccelerationG = 0.0f;

    uint16_t CurrentRearBrightness = 0;
    uint16_t CurrentLeftTurnBrightness = 0;
    uint16_t CurrentRightTurnBrightness = 0;

    uint16_t LastPassiveBrightness = 65535;

    uint32_t TurnBlinkStartTimeMs = 0;
    uint32_t LastLightingAnimationTimeMs = 0;
    uint32_t LastExhaustTriggerTimeMs = 0;
    uint32_t ExhaustPulseStartTimeMs = 0;
    uint32_t ExhaustPulseDurationMs = 0;
};

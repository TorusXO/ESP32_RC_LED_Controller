#include "LightingController.h"

#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>

namespace
{
    static constexpr uint8_t I2C_SDA_PIN = 21;
    static constexpr uint8_t I2C_SCL_PIN = 22;

    static constexpr uint8_t PCA9685_ADDRESS = 0x40;

    // 50 Hz supports both LED PWM and the optional headlight servo.
    static constexpr uint16_t PCA9685_PWM_FREQUENCY_HZ = 50;

    static constexpr uint16_t LED_OFF_BRIGHTNESS = 0;
    static constexpr uint16_t LED_FULL_BRIGHTNESS = 4095;

    static constexpr int16_t TURN_ACTIVATION_PERCENT = 25;
    static constexpr int16_t TURN_DEACTIVATION_PERCENT = 18;

    static constexpr int16_t BRAKE_REVERSE_ACTIVATION_PERCENT =
        -10;

    static constexpr int16_t BRAKE_REVERSE_DEACTIVATION_PERCENT =
        -5;

    static constexpr uint32_t TURN_BLINK_INTERVAL_MS = 450;

    static constexpr bool RESTART_BLINK_ON_DIRECTION_CHANGE =
        true;

    static constexpr uint32_t REAR_LIGHT_BRIGHTEN_DURATION_MS =
        70;

    static constexpr uint32_t REAR_LIGHT_DIM_DURATION_MS =
        180;

    static constexpr uint32_t TURN_LIGHT_BRIGHTEN_DURATION_MS =
        60;

    static constexpr uint32_t TURN_LIGHT_DIM_DURATION_MS =
        140;

    static constexpr uint32_t MAX_ANIMATION_ELAPSED_TIME_MS =
        100;

    static constexpr int16_t EXHAUST_MINIMUM_FORWARD_THROTTLE_PERCENT =
        10;

    static constexpr int16_t EXHAUST_THROTTLE_RISE_PERCENT =
        18;

    static constexpr uint32_t EXHAUST_THROTTLE_PULSE_DURATION_MS =
        70;

    static constexpr uint32_t EXHAUST_ACCELERATION_PULSE_DURATION_MS =
        110;

    static constexpr uint32_t EXHAUST_TEST_PULSE_DURATION_MS =
        180;

    static constexpr uint32_t EXHAUST_TRIGGER_COOLDOWN_MS =
        140;

    Adafruit_PWMServoDriver PwmController(
        PCA9685_ADDRESS
    );
}

FLightingController::FLightingController()
{
}

bool FLightingController::Begin()
{
    Wire.begin(
        I2C_SDA_PIN,
        I2C_SCL_PIN
    );

    Wire.setClock(
        100000
    );

    if (!PwmController.begin())
    {
        return false;
    }

    PwmController.setOutputMode(
        false
    );

    PwmController.setPWMFreq(
        PCA9685_PWM_FREQUENCY_HZ
    );

    delay(10);

    ActiveTurnDirection =
        ETurnDirection::None;

    bBrakeOrReverseActive =
        false;

    bExhaustPulseActive =
        false;

    bManualExhaustPulseActive =
        false;

    PreviousThrottlePercent = 0;
    PreviousFilteredForwardAccelerationG = 0.0f;

    CurrentRearBrightness =
        Configuration.bPassiveLightsEnabled
            ? ConvertPercentToBrightness(
                Configuration.DimBrightnessPercent
            )
            : LED_OFF_BRIGHTNESS;

    CurrentLeftTurnBrightness =
        LED_OFF_BRIGHTNESS;

    CurrentRightTurnBrightness =
        LED_OFF_BRIGHTNESS;

    SetRoleBrightness(1, LED_OFF_BRIGHTNESS);
    SetRoleBrightness(2, LED_OFF_BRIGHTNESS);
    SetRoleBrightness(4, CurrentRearBrightness);
    SetRoleBrightness(5, CurrentLeftTurnBrightness);
    SetRoleBrightness(6, CurrentRightTurnBrightness);
    SetRoleBrightness(
        8,
        Configuration.bFansEnabled
            ? ConvertPercentToBrightness(Configuration.FanSpeedPercent)
            : LED_OFF_BRIGHTNESS
    );

    LastPassiveBrightness = 65535;
    ApplyPassiveOutputs();

    const uint32_t CurrentTimeMs =
        millis();

    TurnBlinkStartTimeMs =
        CurrentTimeMs;

    LastLightingAnimationTimeMs =
        CurrentTimeMs;

    LastExhaustTriggerTimeMs =
        CurrentTimeMs -
        EXHAUST_TRIGGER_COOLDOWN_MS;

    return true;
}

void FLightingController::ApplyConfiguration(
    const FControllerConfiguration& aConfigurationRef
)
{
    Configuration =
        aConfigurationRef;

    LastPassiveBrightness = 65535;
    ApplyPassiveOutputs();
}

void FLightingController::Update(
    const FLightingInputState& aInputStateRef,
    uint32_t aCurrentTimeMs
)
{
    UpdateTurnSignalState(
        aInputStateRef,
        aCurrentTimeMs
    );

    UpdateRearLightState(
        aInputStateRef
    );

    UpdateExhaustState(
        aInputStateRef,
        aCurrentTimeMs
    );

    ApplyLightingOutputs(
        aCurrentTimeMs
    );
}

void FLightingController::TriggerExhaustTest(
    uint32_t aCurrentTimeMs
)
{
    bManualExhaustPulseActive =
        true;

    TriggerExhaustPulse(
        aCurrentTimeMs,
        EXHAUST_TEST_PULSE_DURATION_MS
    );
}

void FLightingController::UpdateTurnSignalState(
    const FLightingInputState& aInputStateRef,
    uint32_t aCurrentTimeMs
)
{
    ETurnDirection NewTurnDirection =
        ActiveTurnDirection;

    if (!aInputStateRef.bSteeringHasSignal)
    {
        NewTurnDirection =
            ETurnDirection::None;
    }
    else
    {
        switch (ActiveTurnDirection)
        {
        case ETurnDirection::None:
        {
            if (
                aInputStateRef.SteeringPercent <=
                -TURN_ACTIVATION_PERCENT
            )
            {
                NewTurnDirection =
                    ETurnDirection::Left;
            }
            else if (
                aInputStateRef.SteeringPercent >=
                TURN_ACTIVATION_PERCENT
            )
            {
                NewTurnDirection =
                    ETurnDirection::Right;
            }

            break;
        }

        case ETurnDirection::Left:
        {
            if (
                aInputStateRef.SteeringPercent >=
                TURN_ACTIVATION_PERCENT
            )
            {
                NewTurnDirection =
                    ETurnDirection::Right;
            }
            else if (
                aInputStateRef.SteeringPercent >
                -TURN_DEACTIVATION_PERCENT
            )
            {
                NewTurnDirection =
                    ETurnDirection::None;
            }

            break;
        }

        case ETurnDirection::Right:
        {
            if (
                aInputStateRef.SteeringPercent <=
                -TURN_ACTIVATION_PERCENT
            )
            {
                NewTurnDirection =
                    ETurnDirection::Left;
            }
            else if (
                aInputStateRef.SteeringPercent <
                TURN_DEACTIVATION_PERCENT
            )
            {
                NewTurnDirection =
                    ETurnDirection::None;
            }

            break;
        }
        }
    }

    if (NewTurnDirection == ActiveTurnDirection)
    {
        return;
    }

    ActiveTurnDirection =
        NewTurnDirection;

    if (RESTART_BLINK_ON_DIRECTION_CHANGE)
    {
        TurnBlinkStartTimeMs =
            aCurrentTimeMs;
    }
}

void FLightingController::UpdateRearLightState(
    const FLightingInputState& aInputStateRef
)
{
    if (
        !Configuration.bActiveLightsEnabled ||
        !aInputStateRef.bThrottleHasSignal
    )
    {
        bBrakeOrReverseActive =
            false;

        return;
    }

    if (!bBrakeOrReverseActive)
    {
        if (
            aInputStateRef.ThrottlePercent <=
            BRAKE_REVERSE_ACTIVATION_PERCENT
        )
        {
            bBrakeOrReverseActive =
                true;
        }

        return;
    }

    if (
        aInputStateRef.ThrottlePercent >=
        BRAKE_REVERSE_DEACTIVATION_PERCENT
    )
    {
        bBrakeOrReverseActive =
            false;
    }
}

void FLightingController::UpdateExhaustState(
    const FLightingInputState& aInputStateRef,
    uint32_t aCurrentTimeMs
)
{
    if (
        !Configuration.bExhaustEnabled &&
        !bManualExhaustPulseActive
    )
    {
        bExhaustPulseActive = false;
        ExhaustPulseDurationMs = 0;

        PreviousThrottlePercent =
            aInputStateRef.ThrottlePercent;

        PreviousFilteredForwardAccelerationG =
            aInputStateRef.FilteredForwardAccelerationG;

        return;
    }

    if (
        !aInputStateRef.bThrottleHasSignal ||
        aInputStateRef.ThrottlePercent <= 0
    )
    {
        PreviousThrottlePercent =
            aInputStateRef.ThrottlePercent;

        PreviousFilteredForwardAccelerationG =
            aInputStateRef.FilteredForwardAccelerationG;

        return;
    }

    const int16_t ThrottleRisePercent =
        aInputStateRef.ThrottlePercent -
        PreviousThrottlePercent;

    const bool bCooldownComplete =
        aCurrentTimeMs -
        LastExhaustTriggerTimeMs >=
        EXHAUST_TRIGGER_COOLDOWN_MS;

    const bool bRapidThrottleRise =
        aInputStateRef.ThrottlePercent >=
        EXHAUST_MINIMUM_FORWARD_THROTTLE_PERCENT &&
        ThrottleRisePercent >=
        EXHAUST_THROTTLE_RISE_PERCENT;

    const bool bAccelerationThresholdCrossed =
        Configuration.bAccelerometerExhaustEnabled &&
        aInputStateRef.bAccelerometerHasSample &&
        aInputStateRef.FilteredForwardAccelerationG >=
        Configuration.ExhaustTriggerThresholdG &&
        PreviousFilteredForwardAccelerationG <
        Configuration.ExhaustTriggerThresholdG;

    if (bCooldownComplete && bRapidThrottleRise)
    {
        TriggerExhaustPulse(
            aCurrentTimeMs,
            EXHAUST_THROTTLE_PULSE_DURATION_MS
        );
    }

    if (
        bCooldownComplete &&
        bAccelerationThresholdCrossed
    )
    {
        TriggerExhaustPulse(
            aCurrentTimeMs,
            EXHAUST_ACCELERATION_PULSE_DURATION_MS
        );
    }

    PreviousThrottlePercent =
        aInputStateRef.ThrottlePercent;

    PreviousFilteredForwardAccelerationG =
        aInputStateRef.FilteredForwardAccelerationG;
}

void FLightingController::TriggerExhaustPulse(
    uint32_t aCurrentTimeMs,
    uint32_t aDurationMs
)
{
    bExhaustPulseActive =
        true;

    ExhaustPulseStartTimeMs =
        aCurrentTimeMs;

    ExhaustPulseDurationMs =
        aDurationMs;

    LastExhaustTriggerTimeMs =
        aCurrentTimeMs;
}

void FLightingController::ApplyLightingOutputs(
    uint32_t aCurrentTimeMs
)
{
    uint32_t AnimationElapsedTimeMs =
        aCurrentTimeMs -
        LastLightingAnimationTimeMs;

    LastLightingAnimationTimeMs =
        aCurrentTimeMs;

    if (
        AnimationElapsedTimeMs >
        MAX_ANIMATION_ELAPSED_TIME_MS
    )
    {
        AnimationElapsedTimeMs =
            MAX_ANIMATION_ELAPSED_TIME_MS;
    }

    const uint16_t ActiveBrightness =
        ConvertPercentToBrightness(
            Configuration.ActiveBrightnessPercent
        );

    const uint16_t DimBrightness =
        ConvertPercentToBrightness(
            Configuration.DimBrightnessPercent
        );

    const uint32_t BlinkElapsedTimeMs =
        aCurrentTimeMs -
        TurnBlinkStartTimeMs;

    const bool bBlinkIsOn =
        (
            (
                BlinkElapsedTimeMs /
                TURN_BLINK_INTERVAL_MS
            ) %
            2
        ) == 0;

    const bool bLeftTurnLightShouldBeOn =
        Configuration.bActiveLightsEnabled &&
        ActiveTurnDirection ==
        ETurnDirection::Left &&
        bBlinkIsOn;

    const bool bRightTurnLightShouldBeOn =
        Configuration.bActiveLightsEnabled &&
        ActiveTurnDirection ==
        ETurnDirection::Right &&
        bBlinkIsOn;

    uint16_t RearTargetBrightness =
        Configuration.bPassiveLightsEnabled
            ? DimBrightness
            : LED_OFF_BRIGHTNESS;

    if (
        Configuration.bActiveLightsEnabled &&
        bBrakeOrReverseActive
    )
    {
        RearTargetBrightness =
            ActiveBrightness;
    }

    const uint16_t LeftTurnTargetBrightness =
        bLeftTurnLightShouldBeOn
            ? ActiveBrightness
            : LED_OFF_BRIGHTNESS;

    const uint16_t RightTurnTargetBrightness =
        bRightTurnLightShouldBeOn
            ? ActiveBrightness
            : LED_OFF_BRIGHTNESS;

    CurrentRearBrightness =
        MoveBrightnessTowardsTarget(
            CurrentRearBrightness,
            RearTargetBrightness,
            LED_FULL_BRIGHTNESS,
            AnimationElapsedTimeMs,
            REAR_LIGHT_BRIGHTEN_DURATION_MS,
            REAR_LIGHT_DIM_DURATION_MS
        );

    CurrentLeftTurnBrightness =
        MoveBrightnessTowardsTarget(
            CurrentLeftTurnBrightness,
            LeftTurnTargetBrightness,
            LED_FULL_BRIGHTNESS,
            AnimationElapsedTimeMs,
            TURN_LIGHT_BRIGHTEN_DURATION_MS,
            TURN_LIGHT_DIM_DURATION_MS
        );

    CurrentRightTurnBrightness =
        MoveBrightnessTowardsTarget(
            CurrentRightTurnBrightness,
            RightTurnTargetBrightness,
            LED_FULL_BRIGHTNESS,
            AnimationElapsedTimeMs,
            TURN_LIGHT_BRIGHTEN_DURATION_MS,
            TURN_LIGHT_DIM_DURATION_MS
        );

    SetRoleBrightness(4, CurrentRearBrightness);
    SetRoleBrightness(5, CurrentLeftTurnBrightness);
    SetRoleBrightness(6, CurrentRightTurnBrightness);
    SetRoleBrightness(
        8,
        Configuration.bFansEnabled
            ? ConvertPercentToBrightness(Configuration.FanSpeedPercent)
            : LED_OFF_BRIGHTNESS
    );

    ApplyPassiveOutputs();

    ApplyExhaustOutputs(
        aCurrentTimeMs
    );

    // At 50 Hz the PCA9685 can also drive the headlight servo. The exact
    // endpoints are intentionally conservative for small RC servos.
    SetRoleServoPosition(
        Configuration.bHeadlightsOpen
            ? Configuration.ServoOpenPulseUs
            : Configuration.ServoClosedPulseUs
    );
}

void FLightingController::ApplyPassiveOutputs()
{
    const uint16_t TargetBrightness =
        Configuration.bPassiveLightsEnabled
            ? ConvertPercentToBrightness(
                Configuration.DimBrightnessPercent
            )
            : LED_OFF_BRIGHTNESS;

    if (
        LastPassiveBrightness ==
        TargetBrightness
    )
    {
        return;
    }

    SetRoleBrightness(3, TargetBrightness);

    LastPassiveBrightness =
        TargetBrightness;
}

void FLightingController::ApplyExhaustOutputs(
    uint32_t aCurrentTimeMs
)
{
    if (!bExhaustPulseActive)
    {
        SetRoleBrightness(1, LED_OFF_BRIGHTNESS);
        SetRoleBrightness(2, LED_OFF_BRIGHTNESS);

        return;
    }

    const uint32_t PulseElapsedTimeMs =
        aCurrentTimeMs -
        ExhaustPulseStartTimeMs;

    if (
        ExhaustPulseDurationMs == 0 ||
        PulseElapsedTimeMs >=
        ExhaustPulseDurationMs
    )
    {
        bExhaustPulseActive = false;
        bManualExhaustPulseActive = false;
        ExhaustPulseDurationMs = 0;

        SetRoleBrightness(1, LED_OFF_BRIGHTNESS);
        SetRoleBrightness(2, LED_OFF_BRIGHTNESS);

        return;
    }

    const uint16_t PeakBrightness =
        ConvertPercentToBrightness(
            Configuration.ActiveBrightnessPercent
        );

    const uint32_t RemainingTimeMs =
        ExhaustPulseDurationMs -
        PulseElapsedTimeMs;

    const uint16_t YellowBrightness =
        static_cast<uint16_t>(
            (
                static_cast<uint32_t>(
                    PeakBrightness
                ) *
                RemainingTimeMs
            ) /
            ExhaustPulseDurationMs
        );

    const uint32_t BluePhaseDurationMs =
        max(
            static_cast<uint32_t>(20),
            ExhaustPulseDurationMs / 3
        );

    uint16_t BlueBrightness =
        LED_OFF_BRIGHTNESS;

    if (
        PulseElapsedTimeMs <
        BluePhaseDurationMs
    )
    {
        BlueBrightness =
            static_cast<uint16_t>(
                (
                    static_cast<uint32_t>(
                        PeakBrightness
                    ) *
                    (
                        BluePhaseDurationMs -
                        PulseElapsedTimeMs
                    )
                ) /
                BluePhaseDurationMs
            );
    }

    SetRoleBrightness(1, YellowBrightness);
    SetRoleBrightness(2, BlueBrightness);
}

uint16_t FLightingController::MoveBrightnessTowardsTarget(
    uint16_t aCurrentBrightness,
    uint16_t aTargetBrightness,
    uint16_t aBrightnessRange,
    uint32_t aElapsedTimeMs,
    uint32_t aBrightenDurationMs,
    uint32_t aDimDurationMs
) const
{
    if (aCurrentBrightness == aTargetBrightness)
    {
        return aCurrentBrightness;
    }

    const bool bBrightnessIncreasing =
        aTargetBrightness >
        aCurrentBrightness;

    const uint32_t TransitionDurationMs =
        bBrightnessIncreasing
            ? aBrightenDurationMs
            : aDimDurationMs;

    if (
        TransitionDurationMs == 0 ||
        aBrightnessRange == 0
    )
    {
        return aTargetBrightness;
    }

    uint32_t BrightnessStep =
        (
            static_cast<uint32_t>(
                aBrightnessRange
            ) *
            aElapsedTimeMs +
            TransitionDurationMs -
            1
        ) /
        TransitionDurationMs;

    if (BrightnessStep == 0)
    {
        BrightnessStep = 1;
    }

    if (bBrightnessIncreasing)
    {
        const uint32_t RemainingBrightness =
            static_cast<uint32_t>(
                aTargetBrightness
            ) -
            static_cast<uint32_t>(
                aCurrentBrightness
            );

        if (
            BrightnessStep >=
            RemainingBrightness
        )
        {
            return aTargetBrightness;
        }

        return static_cast<uint16_t>(
            static_cast<uint32_t>(
                aCurrentBrightness
            ) +
            BrightnessStep
        );
    }

    const uint32_t RemainingBrightness =
        static_cast<uint32_t>(
            aCurrentBrightness
        ) -
        static_cast<uint32_t>(
            aTargetBrightness
        );

    if (
        BrightnessStep >=
        RemainingBrightness
    )
    {
        return aTargetBrightness;
    }

    return static_cast<uint16_t>(
        static_cast<uint32_t>(
            aCurrentBrightness
        ) -
        BrightnessStep
    );
}

uint16_t FLightingController::ConvertPercentToBrightness(
    uint8_t aBrightnessPercent
) const
{
    return static_cast<uint16_t>(
        (
            static_cast<uint32_t>(
                constrain(
                    aBrightnessPercent,
                    static_cast<uint8_t>(0),
                    static_cast<uint8_t>(100)
                )
            ) *
            LED_FULL_BRIGHTNESS
        ) /
        100U
    );
}

void FLightingController::SetLedBrightness(
    uint8_t aChannel,
    uint16_t aBrightness
)
{
    if (aChannel >= 16)
    {
        return;
    }

    aBrightness = constrain(
        aBrightness,
        LED_OFF_BRIGHTNESS,
        LED_FULL_BRIGHTNESS
    );

    PwmController.setPin(
        aChannel,
        aBrightness,
        true
    );
}

void FLightingController::SetRoleBrightness(
    uint8_t aRole,
    uint16_t aBrightness
)
{
    for (uint8_t Channel = 0; Channel < 16; ++Channel)
    {
        if (Configuration.ChannelRoles[Channel] == aRole)
        {
            SetLedBrightness(Channel, aBrightness);
        }
    }
}

void FLightingController::SetRoleServoPosition(
    uint16_t aPulse
)
{
    for (uint8_t Channel = 0; Channel < 16; ++Channel)
    {
        if (Configuration.ChannelRoles[Channel] == 7)
        {
            PwmController.setPWM(Channel, 0, aPulse);
        }
    }
}

ETurnDirection
FLightingController::GetActiveTurnDirection() const
{
    return ActiveTurnDirection;
}

bool FLightingController::IsBrakeOrReverseActive() const
{
    return bBrakeOrReverseActive;
}

bool FLightingController::IsExhaustPulseActive() const
{
    return bExhaustPulseActive;
}

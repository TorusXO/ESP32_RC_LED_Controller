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
        35;

    static constexpr int16_t EXHAUST_THROTTLE_RISE_PERCENT =
        25;

    static constexpr uint32_t EXHAUST_THROTTLE_PULSE_DURATION_MS =
        70;

    static constexpr uint32_t EXHAUST_ACCELERATION_PULSE_DURATION_MS =
        110;

    static constexpr uint32_t EXHAUST_TEST_PULSE_DURATION_MS =
        180;

    static constexpr uint32_t EXHAUST_TRIGGER_COOLDOWN_MS =
        240;

    static constexpr uint32_t EXHAUST_COLOR_TEST_DURATION_MS = 2000;
    static constexpr uint32_t EXHAUST_THROTTLE_SAMPLE_MS = 100;
    static constexpr int16_t EXHAUST_HIGH_THROTTLE_PERCENT = 65;
    static constexpr int16_t EXHAUST_HIGH_THROTTLE_RELEASE_PERCENT = 55;

    Adafruit_PWMServoDriver PwmController(
        PCA9685_ADDRESS
    );
}

FLightingController::FLightingController()
{
}

bool FLightingController::Begin()
{
    bConnected = false;
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

    bConnected = true;

    PwmController.setOutputMode(
        true // Totem-pole outputs drive the common-ground RGB anodes high.
    );

    PwmController.setPWMFreq(
        PCA9685_PWM_FREQUENCY_HZ
    );

    delay(10);

    ActiveTurnDirection =
        ETurnDirection::None;

    bBrakeOrReverseActive =
        false;

    bReverseActive =
        false;

    bReverseArmed =
        false;

    bReverseModeEnabled =
        false;

    bExhaustPulseActive =
        false;

    bManualExhaustPulseActive =
        false;

    PreviousThrottlePercent = 0;
    bExhaustInputValid = false;
    bHighThrottleActive = false;
    bAccelerationTriggerArmed = true;
    ExhaustTestRole = 0;
    ExhaustBurstSequence = 0;
    ExhaustBurstIntervalMs = 500;

    // Tail lights keep their dim running-light level independently of the
    // passive-light toggle. Role 3 is the only role controlled by that toggle.
    CurrentRearBrightness =
        ConvertPercentToBrightness(
            Configuration.DimBrightnessPercent
        );

    CurrentLeftTurnBrightness =
        LED_OFF_BRIGHTNESS;

    CurrentRightTurnBrightness =
        LED_OFF_BRIGHTNESS;

    SetExhaustRoleBrightness(1, LED_OFF_BRIGHTNESS);
    SetExhaustRoleBrightness(2, LED_OFF_BRIGHTNESS);
    SetExhaustRoleBrightness(10, LED_OFF_BRIGHTNESS);
    SetRoleBrightness(4, CurrentRearBrightness);
    SetRoleBrightness(5, CurrentLeftTurnBrightness);
    SetRoleBrightness(6, CurrentRightTurnBrightness);
    SetRoleBrightness(
        9,
        bReverseActive
            ? LED_FULL_BRIGHTNESS
            : LED_OFF_BRIGHTNESS
    );
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

bool FLightingController::IsConnected() const
{
    return bConnected;
}

uint8_t FLightingController::GetDeviceAddress() const
{
    return PCA9685_ADDRESS;
}

uint8_t FLightingController::ReadMode1Register() const
{
    if (!bConnected)
    {
        return 0;
    }

    Wire.beginTransmission(PCA9685_ADDRESS);
    Wire.write(0x00);
    if (Wire.endTransmission(false) != 0 || Wire.requestFrom(PCA9685_ADDRESS, static_cast<uint8_t>(1)) != 1)
    {
        return 0;
    }

    return Wire.read();
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
    uint32_t aCurrentTimeMs,
    uint8_t aRole
)
{
    if (aRole != 0 && aRole != 1 && aRole != 2 && aRole != 10 && aRole != 255)
    {
        return;
    }
    ExhaustTestRole = aRole;
    if (aRole == 255)
    {
        bExhaustPulseActive = false;
        bManualExhaustPulseActive = false;
        ExhaustPulseDurationMs = 0;
        LastExhaustTriggerTimeMs = aCurrentTimeMs;
        ApplyExhaustOutputs(aCurrentTimeMs);
        return;
    }
    bManualExhaustPulseActive =
        true;

    TriggerExhaustPulse(
        aCurrentTimeMs,
        aRole == 0 ? EXHAUST_TEST_PULSE_DURATION_MS : EXHAUST_COLOR_TEST_DURATION_MS
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

        bReverseActive =
            false;

        bReverseArmed =
            false;

        bReverseModeEnabled =
            false;

        return;
    }

    if (
        aInputStateRef.ThrottlePercent <=
        BRAKE_REVERSE_ACTIVATION_PERCENT
    )
    {
        if (
            bReverseArmed ||
            bReverseModeEnabled
        )
        {
            bReverseActive =
                true;

            bReverseModeEnabled =
                true;

            bReverseArmed =
                false;
        }

        // The first reverse-direction command is always treated as braking.
        // Reverse becomes available only after the stick returns to neutral.
        bBrakeOrReverseActive =
            true;

        return;
    }

    if (
        aInputStateRef.ThrottlePercent >=
        BRAKE_REVERSE_DEACTIVATION_PERCENT
    )
    {
        if (
            bBrakeOrReverseActive &&
            !bReverseActive
        )
        {
            // Brake -> neutral arms the next reverse-direction command.
            bReverseArmed =
                true;
        }

        if (aInputStateRef.ThrottlePercent > 0)
        {
            // A forward command cancels the brake-to-reverse sequence and
            // exits reverse mode so the next reverse command brakes first.
            bReverseArmed =
                false;

            bReverseModeEnabled =
                false;
        }

        bBrakeOrReverseActive =
            false;

        bReverseActive =
            false;
    }
}

void FLightingController::UpdateExhaustState(
    const FLightingInputState& aInputStateRef,
    uint32_t aCurrentTimeMs
)
{
    const int16_t Throttle = aInputStateRef.ThrottlePercent;
    const float Acceleration = aInputStateRef.FilteredForwardAccelerationG;
    // Manual tests own all three exhaust outputs until their timeout.
    if (bManualExhaustPulseActive)
    {
        PreviousThrottlePercent = Throttle;
        ThrottleSampleTimeMs = aCurrentTimeMs;
        bExhaustInputValid = false;
        bHighThrottleActive = false;
        return;
    }

    if (!Configuration.bExhaustEnabled ||
        !aInputStateRef.bThrottleHasSignal || Throttle < 0)
    {
        bExhaustPulseActive = false;
        ExhaustPulseDurationMs = 0;
        bExhaustInputValid = false;
        bHighThrottleActive = false;
        bAccelerationTriggerArmed = true;
        return;
    }

    // Seed the sample on connection/reconnection: signal acquisition is not a blip.
    if (!bExhaustInputValid)
    {
        PreviousThrottlePercent = Throttle;
        ThrottleSampleTimeMs = aCurrentTimeMs;
        bExhaustInputValid = true;
        bAccelerationTriggerArmed = Acceleration < Configuration.ExhaustTriggerThresholdG;
    }

    const uint32_t SampleElapsed = aCurrentTimeMs - ThrottleSampleTimeMs;
    const int16_t ThrottleChange = Throttle - PreviousThrottlePercent;
    const bool bRecentSample = SampleElapsed <= 200;
    const bool bRapidRise = bRecentSample &&
        Throttle >= EXHAUST_MINIMUM_FORWARD_THROTTLE_PERCENT &&
        ThrottleChange >= EXHAUST_THROTTLE_RISE_PERCENT;
    const bool bLiftOff = bRecentSample &&
        PreviousThrottlePercent >= EXHAUST_HIGH_THROTTLE_PERCENT &&
        ThrottleChange <= -30;

    if (!bHighThrottleActive && Throttle >= EXHAUST_HIGH_THROTTLE_PERCENT)
    {
        bHighThrottleActive = true;
        HighThrottleStartTimeMs = aCurrentTimeMs;
    }
    else if (Throttle < EXHAUST_HIGH_THROTTLE_RELEASE_PERCENT)
    {
        bHighThrottleActive = false;
    }

    const bool bAccelerationAvailable = Configuration.bAccelerometerExhaustEnabled &&
        aInputStateRef.bAccelerometerHasSample;
    if (!bAccelerationAvailable || Acceleration < Configuration.ExhaustTriggerThresholdG * 0.6f)
    {
        bAccelerationTriggerArmed = true;
    }
    const bool bAccelerationSpike = bAccelerationAvailable &&
        bAccelerationTriggerArmed && Acceleration >= Configuration.ExhaustTriggerThresholdG;
    if (bAccelerationSpike)
    {
        bAccelerationTriggerArmed = false;
    }

    const bool bCooldownComplete =
        aCurrentTimeMs - LastExhaustTriggerTimeMs >= EXHAUST_TRIGGER_COOLDOWN_MS;
    const bool bHighThrottleBurst = bHighThrottleActive &&
        aCurrentTimeMs - HighThrottleStartTimeMs >= 250 &&
        aCurrentTimeMs - LastExhaustTriggerTimeMs >= ExhaustBurstIntervalMs;

    if (!bExhaustPulseActive && bCooldownComplete &&
        (bRapidRise || bLiftOff || (Throttle >= 40 && bAccelerationSpike) || bHighThrottleBurst))
    {
        ++ExhaustBurstSequence;
        // Vary the gaps and pulse lengths; leave a clear dark interval between pops.
        ExhaustBurstIntervalMs = 420 + (ExhaustBurstSequence * 137U) % 430U;
        uint32_t Duration = EXHAUST_THROTTLE_PULSE_DURATION_MS;
        if (bLiftOff || bAccelerationSpike)
            Duration = EXHAUST_ACCELERATION_PULSE_DURATION_MS;
        if (bHighThrottleBurst)
            Duration = 60 + (ExhaustBurstSequence * 23U) % 70U;
        TriggerExhaustPulse(aCurrentTimeMs, Duration);
    }

    // Compare over a short time window, rather than a single loop iteration.
    if (SampleElapsed >= EXHAUST_THROTTLE_SAMPLE_MS || bRapidRise || bLiftOff)
    {
        PreviousThrottlePercent = Throttle;
        ThrottleSampleTimeMs = aCurrentTimeMs;
    }
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
        ConvertPercentToBrightness(
            Configuration.DimBrightnessPercent
        );

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
        9,
        bReverseActive
            ? LED_FULL_BRIGHTNESS
            : LED_OFF_BRIGHTNESS
    );
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

    if (bServoTestOverrideActive)
    {
        PwmController.setPWM(
            ServoTestChannel,
            0,
            ServoTestPulse
        );
    }
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
        SetExhaustRoleBrightness(1, LED_OFF_BRIGHTNESS);
        SetExhaustRoleBrightness(2, LED_OFF_BRIGHTNESS);
        SetExhaustRoleBrightness(10, LED_OFF_BRIGHTNESS);

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

        SetExhaustRoleBrightness(1, LED_OFF_BRIGHTNESS);
        SetExhaustRoleBrightness(2, LED_OFF_BRIGHTNESS);
        SetExhaustRoleBrightness(10, LED_OFF_BRIGHTNESS);

        return;
    }

    const uint16_t PeakBrightness =
        ConvertPercentToBrightness(
            Configuration.ActiveBrightnessPercent
        );

    if (bManualExhaustPulseActive && ExhaustTestRole != 0)
    {
        SetExhaustRoleBrightness(1, ExhaustTestRole == 1 ? LED_FULL_BRIGHTNESS : 0);
        SetExhaustRoleBrightness(2, ExhaustTestRole == 2 ? LED_FULL_BRIGHTNESS : 0);
        SetExhaustRoleBrightness(10, ExhaustTestRole == 10 ? LED_FULL_BRIGHTNESS : 0);
        return;
    }

    const uint32_t RemainingTimeMs =
        ExhaustPulseDurationMs -
        PulseElapsedTimeMs;

    const uint16_t RedGreenBrightness =
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

    // Blue ignition, then a warm orange/red tail. Avoid an equal RGB white flash.
    const bool bIgnition = PulseElapsedTimeMs < BluePhaseDurationMs;
    SetExhaustRoleBrightness(1, bIgnition ? RedGreenBrightness / 5 : RedGreenBrightness);
    SetExhaustRoleBrightness(2, bIgnition ? 0 : RedGreenBrightness / 4);
    SetExhaustRoleBrightness(10, BlueBrightness);
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
    uint16_t aBrightness,
    bool aInvert
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
        aInvert
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

void FLightingController::SetExhaustRoleBrightness(
    uint8_t aRole,
    uint16_t aBrightness
)
{
    for (uint8_t Channel = 0; Channel < 16; ++Channel)
    {
        if (Configuration.ChannelRoles[Channel] == aRole)
        {
            // The RGB diode is common-cathode (common ground), so its anodes
            // are driven with non-inverted PWM.
            SetLedBrightness(Channel, aBrightness, false);
        }
    }
}

void FLightingController::SetRoleServoPosition(
    uint16_t aPulse
)
{
    bool bExplicitChannelWritten = false;

    if (Configuration.HeadlightServoChannel < 16)
    {
        PwmController.setPWM(
            Configuration.HeadlightServoChannel,
            0,
            aPulse
        );
        bExplicitChannelWritten = true;
    }

    for (uint8_t Channel = 0; Channel < 16; ++Channel)
    {
        if (
            Configuration.ChannelRoles[Channel] == 7 &&
            (!bExplicitChannelWritten ||
             Channel != Configuration.HeadlightServoChannel)
        )
        {
            PwmController.setPWM(Channel, 0, aPulse);
        }
    }
}

void FLightingController::SetServoTestPosition(
    uint8_t aChannel,
    uint16_t aPulse
)
{
    if (aChannel >= 16)
    {
        return;
    }

    ServoTestChannel = aChannel;
    ServoTestPulse = aPulse;
    bServoTestOverrideActive = true;

    PwmController.setPWM(
        ServoTestChannel,
        0,
        ServoTestPulse
    );
}

void FLightingController::ClearServoTestPosition()
{
    bServoTestOverrideActive = false;
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

bool FLightingController::IsReverseActive() const
{
    return bReverseActive;
}

bool FLightingController::IsExhaustPulseActive() const
{
    return bExhaustPulseActive;
}

#pragma once

#include <Arduino.h>

struct FControllerConfiguration
{
    bool bPassiveLightsEnabled = true;
    bool bActiveLightsEnabled = true;
    bool bExhaustEnabled = true;

    // Stored now and ready for future hardware assignments.
    bool bHeadlightsOpen = false;
    bool bFansEnabled = false;

    bool bAccelerometerExhaustEnabled = true;

    float ExhaustTriggerThresholdG = 0.06f;

    uint8_t ActiveBrightnessPercent = 100;
    uint8_t DimBrightnessPercent = 25;
    uint8_t FanSpeedPercent = 70;
    uint16_t ServoClosedPulseUs = 102;
    uint16_t ServoOpenPulseUs = 512;

    // PCA9685 output assignments. These are intentionally configurable so
    // the same firmware can be reused with different wiring harnesses.
    uint8_t ExhaustLight1Channel = 0;
    uint8_t ExhaustLight2Channel = 1;
    uint8_t PassiveLightsChannel = 2;
    uint8_t TailLightsChannel = 13;
    uint8_t LeftTurnLightsChannel = 14;
    uint8_t RightTurnLightsChannel = 15;
    uint8_t HeadlightServoChannel = 12;

    // Role assigned to each PCA9685 channel. Multiple channels may share
    // the same role; 0 means unused.
    uint8_t ChannelRoles[16] = {
        1, 2, 3, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 7, 4, 5, 6
    };

    void ResetToDefaults()
    {
        *this = FControllerConfiguration();
    }
};

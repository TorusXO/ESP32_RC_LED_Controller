#include "ControllerProtocol.h"

#include <BluetoothSerial.h>
#include <Preferences.h>

#include <stdlib.h>
#include <string.h>

namespace
{
    static constexpr char PREFERENCES_NAMESPACE[] =
        "rcled";

    static constexpr char DEVICE_PROTOCOL_VERSION[] =
        "1";

    static constexpr char DEVICE_NAME[] =
        "RC_LED_CONTROLLER";

    static constexpr char FIRMWARE_VERSION[] =
        "0.6";

    static constexpr float MINIMUM_TRIGGER_THRESHOLD_G =
        0.01f;

    static constexpr float MAXIMUM_TRIGGER_THRESHOLD_G =
        0.50f;


    uint8_t ParseChannel(const char* aValuePtr)
    {
        const int Value = atoi(aValuePtr);
        return Value == 255
            ? 255
            : static_cast<uint8_t>(constrain(Value, 0, 15));
    }
}

FControllerProtocol::FControllerProtocol()
{
}

bool FControllerProtocol::Begin(
    BluetoothSerial& aBluetoothSerialPortRef,
    FControllerConfiguration& aConfigurationRef
)
{
    BluetoothSerialPortPtr =
        &aBluetoothSerialPortRef;

    ConfigurationPtr =
        &aConfigurationRef;

    CommandLength = 0;
    bHadBluetoothClient = false;
    bConfigurationChanged = false;
    bExhaustTestRequested = false;

    LoadConfiguration();

    return true;
}

void FControllerProtocol::Update()
{
    if (
        BluetoothSerialPortPtr == nullptr ||
        ConfigurationPtr == nullptr
    )
    {
        return;
    }

    const bool bHasBluetoothClient =
        BluetoothSerialPortPtr->hasClient();

    if (
        bHasBluetoothClient &&
        !bHadBluetoothClient
    )
    {
        SendHello();
        SendConfiguration();
    }

    bHadBluetoothClient =
        bHasBluetoothClient;

    while (
        BluetoothSerialPortPtr->available() > 0
    )
    {
        const int ReadValue =
            BluetoothSerialPortPtr->read();

        if (ReadValue < 0)
        {
            break;
        }

        const char ReadCharacter =
            static_cast<char>(ReadValue);

        if (ReadCharacter == '\r')
        {
            continue;
        }

        if (ReadCharacter == '\n')
        {
            if (CommandLength > 0)
            {
                CommandBuffer[CommandLength] =
                    '\0';

                HandleCommand(CommandBuffer);
                CommandLength = 0;
            }

            continue;
        }

        if (
            CommandLength + 1 >=
            COMMAND_BUFFER_SIZE
        )
        {
            CommandLength = 0;
            SendError("LINE_TOO_LONG");
            continue;
        }

        CommandBuffer[CommandLength] =
            ReadCharacter;

        ++CommandLength;
    }
}

bool FControllerProtocol::ConsumeConfigurationChanged()
{
    const bool bWasChanged =
        bConfigurationChanged;

    bConfigurationChanged = false;
    return bWasChanged;
}

bool FControllerProtocol::ConsumeExhaustTestRequested()
{
    const bool bWasRequested =
        bExhaustTestRequested;

    bExhaustTestRequested = false;
    return bWasRequested;
}

void FControllerProtocol::SendHello()
{
    if (
        BluetoothSerialPortPtr == nullptr ||
        !BluetoothSerialPortPtr->hasClient()
    )
    {
        return;
    }

    BluetoothSerialPortPtr->printf(
        "HELLO,%s,%s,%s\n",
        DEVICE_PROTOCOL_VERSION,
        DEVICE_NAME,
        FIRMWARE_VERSION
    );
}

void FControllerProtocol::SendConfiguration()
{
    if (
        BluetoothSerialPortPtr == nullptr ||
        ConfigurationPtr == nullptr ||
        !BluetoothSerialPortPtr->hasClient()
    )
    {
        return;
    }

    BluetoothSerialPortPtr->printf(
        "CFG,%u,%u,%u,%u,%u,%u,%.3f,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,"
        "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
        ConfigurationPtr->bPassiveLightsEnabled
            ? 1U
            : 0U,
        ConfigurationPtr->bActiveLightsEnabled
            ? 1U
            : 0U,
        ConfigurationPtr->bExhaustEnabled
            ? 1U
            : 0U,
        ConfigurationPtr->bHeadlightsOpen
            ? 1U
            : 0U,
        ConfigurationPtr->bFansEnabled
            ? 1U
            : 0U,
        ConfigurationPtr->bAccelerometerExhaustEnabled
            ? 1U
            : 0U,
        ConfigurationPtr->ExhaustTriggerThresholdG,
        ConfigurationPtr->ActiveBrightnessPercent,
        ConfigurationPtr->DimBrightnessPercent,
        ConfigurationPtr->FanSpeedPercent,
        ConfigurationPtr->ExhaustLight1Channel,
        ConfigurationPtr->ExhaustLight2Channel,
        ConfigurationPtr->PassiveLightsChannel,
        ConfigurationPtr->TailLightsChannel,
        ConfigurationPtr->LeftTurnLightsChannel,
        ConfigurationPtr->RightTurnLightsChannel,
        ConfigurationPtr->HeadlightServoChannel,
        ConfigurationPtr->ChannelRoles[0],
        ConfigurationPtr->ChannelRoles[1],
        ConfigurationPtr->ChannelRoles[2],
        ConfigurationPtr->ChannelRoles[3],
        ConfigurationPtr->ChannelRoles[4],
        ConfigurationPtr->ChannelRoles[5],
        ConfigurationPtr->ChannelRoles[6],
        ConfigurationPtr->ChannelRoles[7],
        ConfigurationPtr->ChannelRoles[8],
        ConfigurationPtr->ChannelRoles[9],
        ConfigurationPtr->ChannelRoles[10],
        ConfigurationPtr->ChannelRoles[11],
        ConfigurationPtr->ChannelRoles[12],
        ConfigurationPtr->ChannelRoles[13],
        ConfigurationPtr->ChannelRoles[14],
        ConfigurationPtr->ChannelRoles[15],
        ConfigurationPtr->ServoClosedPulseUs,
        ConfigurationPtr->ServoOpenPulseUs
    );
}

void FControllerProtocol::SendTelemetry(
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
)
{
    if (
        BluetoothSerialPortPtr == nullptr ||
        !BluetoothSerialPortPtr->hasClient()
    )
    {
        return;
    }

    BluetoothSerialPortPtr->printf(
        "TEL,%lu,%u,%lu,%d,%u,%lu,%d,"
        "%.3f,%.3f,%.3f,%.3f,"
        "%.1f,%.1f,%.1f,%s,%u,%u\n",
        static_cast<unsigned long>(
            aCurrentTimeMs
        ),
        aSteeringHasSignal ? 1U : 0U,
        static_cast<unsigned long>(
            aSteeringPulseUs
        ),
        aSteeringPercent,
        aThrottleHasSignal ? 1U : 0U,
        static_cast<unsigned long>(
            aThrottlePulseUs
        ),
        aThrottlePercent,
        aForwardAccelerationG,
        aFilteredForwardAccelerationG,
        aSideAccelerationG,
        aVerticalAccelerationG,
        aGyroscopeXDps,
        aGyroscopeYDps,
        aGyroscopeZDps,
        aTurnDirectionPtr,
        aBrakeActive ? 1U : 0U,
        aExhaustPulseActive ? 1U : 0U
    );
}

void FControllerProtocol::HandleCommand(
    char* aCommandPtr
)
{
    char* SaveStatePtr = nullptr;

    const char* CommandPtr =
        strtok_r(
            aCommandPtr,
            ",",
            &SaveStatePtr
        );

    if (CommandPtr == nullptr)
    {
        return;
    }

    if (strcmp(CommandPtr, "HELLO") == 0)
    {
        SendHello();
        return;
    }

    if (strcmp(CommandPtr, "GET") == 0)
    {
        const char* KeyPtr =
            strtok_r(
                nullptr,
                ",",
                &SaveStatePtr
            );

        if (
            KeyPtr != nullptr &&
            strcmp(KeyPtr, "CONFIG") == 0
        )
        {
            SendConfiguration();
            return;
        }

        SendError("UNKNOWN_GET");
        return;
    }

    if (strcmp(CommandPtr, "SET") == 0)
    {
        const char* KeyPtr =
            strtok_r(
                nullptr,
                ",",
                &SaveStatePtr
            );

        const char* ValuePtr =
            strtok_r(
                nullptr,
                ",",
                &SaveStatePtr
            );

        if (
            KeyPtr == nullptr ||
            ValuePtr == nullptr
        )
        {
            SendError("INVALID_SET");
            return;
        }

        HandleSetCommand(
            KeyPtr,
            ValuePtr
        );

        return;
    }

    if (strcmp(CommandPtr, "TEST") == 0)
    {
        const char* KeyPtr =
            strtok_r(
                nullptr,
                ",",
                &SaveStatePtr
            );

        if (
            KeyPtr != nullptr &&
            strcmp(KeyPtr, "EXHAUST") == 0
        )
        {
            bExhaustTestRequested = true;
            SendAcknowledgement(
                "TEST_EXHAUST",
                "1"
            );

            return;
        }

        SendError("UNKNOWN_TEST");
        return;
    }

    if (strcmp(CommandPtr, "SAVE") == 0)
    {
        if (SaveConfiguration())
        {
            SendAcknowledgement("SAVE", "1");
            SendConfiguration();
        }

        return;
    }

    if (strcmp(CommandPtr, "RESET") == 0)
    {
        ConfigurationPtr->ResetToDefaults();
        bConfigurationChanged = true;

        if (SaveConfiguration())
        {
            SendAcknowledgement("RESET", "1");
            SendConfiguration();
        }

        return;
    }

    SendError("UNKNOWN_COMMAND");
}

void FControllerProtocol::HandleSetCommand(
    const char* aKeyPtr,
    const char* aValuePtr
)
{
    const bool bBooleanValue =
        atoi(aValuePtr) != 0;

    if (strcmp(aKeyPtr, "PASSIVE_LIGHTS") == 0)
    {
        ConfigurationPtr->bPassiveLightsEnabled =
            bBooleanValue;
    }
    else if (
        strcmp(aKeyPtr, "ACTIVE_LIGHTS") == 0
    )
    {
        ConfigurationPtr->bActiveLightsEnabled =
            bBooleanValue;
    }
    else if (
        strcmp(aKeyPtr, "EXHAUST_ENABLED") == 0
    )
    {
        ConfigurationPtr->bExhaustEnabled =
            bBooleanValue;
    }
    else if (
        strcmp(aKeyPtr, "HEADLIGHT_OPEN") == 0
    )
    {
        ConfigurationPtr->bHeadlightsOpen =
            bBooleanValue;
    }
    else if (
        strcmp(aKeyPtr, "FANS_ENABLED") == 0
    )
    {
        ConfigurationPtr->bFansEnabled =
            bBooleanValue;
    }
    else if (
        strcmp(
            aKeyPtr,
            "ACCELEROMETER_ENABLED"
        ) == 0
    )
    {
        ConfigurationPtr->bAccelerometerExhaustEnabled =
            bBooleanValue;
    }
    else if (
        strcmp(
            aKeyPtr,
            "EXHAUST_TRIGGER_G"
        ) == 0
    )
    {
        ConfigurationPtr->ExhaustTriggerThresholdG =
            constrain(
                static_cast<float>(
                    atof(aValuePtr)
                ),
                MINIMUM_TRIGGER_THRESHOLD_G,
                MAXIMUM_TRIGGER_THRESHOLD_G
            );
    }
    else if (
        strcmp(
            aKeyPtr,
            "ACTIVE_BRIGHTNESS"
        ) == 0
    )
    {
        ConfigurationPtr->ActiveBrightnessPercent =
            static_cast<uint8_t>(
                constrain(
                    atoi(aValuePtr),
                    0,
                    100
                )
            );
    }
    else if (
        strcmp(
            aKeyPtr,
            "DIM_BRIGHTNESS"
        ) == 0
    )
    {
        ConfigurationPtr->DimBrightnessPercent =
            static_cast<uint8_t>(
                constrain(
                    atoi(aValuePtr),
                    0,
                    100
                )
            );
    }
    else if (
        strcmp(aKeyPtr, "FAN_SPEED") == 0
    )
    {
        ConfigurationPtr->FanSpeedPercent =
            static_cast<uint8_t>(
                constrain(
                    atoi(aValuePtr),
                    0,
                    100
                )
            );
    }
    else if (strcmp(aKeyPtr, "SERVO_CLOSED_PULSE") == 0)
    {
        ConfigurationPtr->ServoClosedPulseUs = static_cast<uint16_t>(
            constrain(atoi(aValuePtr), 0, 4095)
        );
    }
    else if (strcmp(aKeyPtr, "SERVO_OPEN_PULSE") == 0)
    {
        ConfigurationPtr->ServoOpenPulseUs = static_cast<uint16_t>(
            constrain(atoi(aValuePtr), 0, 4095)
        );
    }
    else if (strcmp(aKeyPtr, "SERVO_ZERO") == 0)
    {
        ConfigurationPtr->bHeadlightsOpen = false;
    }
    else if (strcmp(aKeyPtr, "EXHAUST_LIGHT_1_CHANNEL") == 0)
    {
        ConfigurationPtr->ExhaustLight1Channel = ParseChannel(aValuePtr);
    }
    else if (strcmp(aKeyPtr, "EXHAUST_LIGHT_2_CHANNEL") == 0)
    {
        ConfigurationPtr->ExhaustLight2Channel = ParseChannel(aValuePtr);
    }
    else if (strcmp(aKeyPtr, "PASSIVE_LIGHTS_CHANNEL") == 0)
    {
        ConfigurationPtr->PassiveLightsChannel = ParseChannel(aValuePtr);
    }
    else if (strcmp(aKeyPtr, "TAIL_LIGHTS_CHANNEL") == 0)
    {
        ConfigurationPtr->TailLightsChannel = ParseChannel(aValuePtr);
    }
    else if (strcmp(aKeyPtr, "LEFT_TURN_LIGHTS_CHANNEL") == 0)
    {
        ConfigurationPtr->LeftTurnLightsChannel = ParseChannel(aValuePtr);
    }
    else if (strcmp(aKeyPtr, "RIGHT_TURN_LIGHTS_CHANNEL") == 0)
    {
        ConfigurationPtr->RightTurnLightsChannel = ParseChannel(aValuePtr);
    }
    else if (strcmp(aKeyPtr, "HEADLIGHT_SERVO_CHANNEL") == 0)
    {
        ConfigurationPtr->HeadlightServoChannel = ParseChannel(aValuePtr);
    }
    else if (strncmp(aKeyPtr, "CHANNEL_ROLE_", 13) == 0)
    {
        const int Channel = atoi(aKeyPtr + 13);
        if (Channel < 0 || Channel >= 16)
        {
            SendError("INVALID_CHANNEL");
            return;
        }

        ConfigurationPtr->ChannelRoles[Channel] = static_cast<uint8_t>(
            constrain(atoi(aValuePtr), 0, 8)
        );
    }
    else
    {
        SendError("UNKNOWN_SETTING");
        return;
    }

    bConfigurationChanged = true;

    SendAcknowledgement(
        aKeyPtr,
        aValuePtr
    );
}

void FControllerProtocol::LoadConfiguration()
{
    Preferences ControllerPreferences;

    if (
        !ControllerPreferences.begin(
            PREFERENCES_NAMESPACE,
            true
        )
    )
    {
        return;
    }

    ConfigurationPtr->bPassiveLightsEnabled =
        ControllerPreferences.getBool(
            "passive",
            ConfigurationPtr->bPassiveLightsEnabled
        );

    ConfigurationPtr->bActiveLightsEnabled =
        ControllerPreferences.getBool(
            "active",
            ConfigurationPtr->bActiveLightsEnabled
        );

    ConfigurationPtr->bExhaustEnabled =
        ControllerPreferences.getBool(
            "exhaust",
            ConfigurationPtr->bExhaustEnabled
        );

    ConfigurationPtr->bHeadlightsOpen =
        ControllerPreferences.getBool(
            "headopen",
            ConfigurationPtr->bHeadlightsOpen
        );

    ConfigurationPtr->bFansEnabled =
        ControllerPreferences.getBool(
            "fans",
            ConfigurationPtr->bFansEnabled
        );

    ConfigurationPtr->bAccelerometerExhaustEnabled =
        ControllerPreferences.getBool(
            "accelen",
            ConfigurationPtr->
                bAccelerometerExhaustEnabled
        );

    ConfigurationPtr->ExhaustTriggerThresholdG =
        ControllerPreferences.getFloat(
            "trig",
            ConfigurationPtr->ExhaustTriggerThresholdG
        );

    ConfigurationPtr->ActiveBrightnessPercent =
        ControllerPreferences.getUChar(
            "bright",
            ConfigurationPtr->ActiveBrightnessPercent
        );

    ConfigurationPtr->DimBrightnessPercent =
        ControllerPreferences.getUChar(
            "dim",
            ConfigurationPtr->DimBrightnessPercent
        );

    ConfigurationPtr->FanSpeedPercent =
        ControllerPreferences.getUChar(
            "fanspeed",
            ConfigurationPtr->FanSpeedPercent
        );

    ConfigurationPtr->ExhaustLight1Channel = ControllerPreferences.getUChar("exhaust1", ConfigurationPtr->ExhaustLight1Channel);
    ConfigurationPtr->ExhaustLight2Channel = ControllerPreferences.getUChar("exhaust2", ConfigurationPtr->ExhaustLight2Channel);
    ConfigurationPtr->PassiveLightsChannel = ControllerPreferences.getUChar("passivech", ConfigurationPtr->PassiveLightsChannel);
    ConfigurationPtr->TailLightsChannel = ControllerPreferences.getUChar("tailch", ConfigurationPtr->TailLightsChannel);
    ConfigurationPtr->LeftTurnLightsChannel = ControllerPreferences.getUChar("leftch", ConfigurationPtr->LeftTurnLightsChannel);
    ConfigurationPtr->RightTurnLightsChannel = ControllerPreferences.getUChar("rightch", ConfigurationPtr->RightTurnLightsChannel);
    ConfigurationPtr->HeadlightServoChannel = ControllerPreferences.getUChar("servoch", ConfigurationPtr->HeadlightServoChannel);
    ConfigurationPtr->ServoClosedPulseUs = ControllerPreferences.getUShort(
        "servoclose",
        ConfigurationPtr->ServoClosedPulseUs
    );
    ConfigurationPtr->ServoOpenPulseUs = ControllerPreferences.getUShort(
        "servoopen",
        ConfigurationPtr->ServoOpenPulseUs
    );

    for (int Channel = 0; Channel < 16; ++Channel)
    {
        char Key[8] = {};
        snprintf(Key, sizeof(Key), "role%d", Channel);
        ConfigurationPtr->ChannelRoles[Channel] =
            ControllerPreferences.getUChar(
                Key,
                ConfigurationPtr->ChannelRoles[Channel]
            );
    }

    ControllerPreferences.end();

    ConfigurationPtr->ExhaustTriggerThresholdG =
        constrain(
            ConfigurationPtr->ExhaustTriggerThresholdG,
            MINIMUM_TRIGGER_THRESHOLD_G,
            MAXIMUM_TRIGGER_THRESHOLD_G
        );

    ConfigurationPtr->ActiveBrightnessPercent =
        constrain(
            ConfigurationPtr->ActiveBrightnessPercent,
            static_cast<uint8_t>(0),
            static_cast<uint8_t>(100)
        );

    ConfigurationPtr->DimBrightnessPercent =
        constrain(
            ConfigurationPtr->DimBrightnessPercent,
            static_cast<uint8_t>(0),
            static_cast<uint8_t>(100)
        );

    ConfigurationPtr->FanSpeedPercent =
        constrain(
            ConfigurationPtr->FanSpeedPercent,
            static_cast<uint8_t>(0),
            static_cast<uint8_t>(100)
        );
}

bool FControllerProtocol::SaveConfiguration()
{
    Preferences ControllerPreferences;

    if (
        !ControllerPreferences.begin(
            PREFERENCES_NAMESPACE,
            false
        )
    )
    {
        SendError("SAVE_FAILED");
        return false;
    }

    ControllerPreferences.putBool(
        "passive",
        ConfigurationPtr->bPassiveLightsEnabled
    );

    ControllerPreferences.putBool(
        "active",
        ConfigurationPtr->bActiveLightsEnabled
    );

    ControllerPreferences.putBool(
        "exhaust",
        ConfigurationPtr->bExhaustEnabled
    );

    ControllerPreferences.putBool(
        "headopen",
        ConfigurationPtr->bHeadlightsOpen
    );

    ControllerPreferences.putBool(
        "fans",
        ConfigurationPtr->bFansEnabled
    );

    ControllerPreferences.putBool(
        "accelen",
        ConfigurationPtr->
            bAccelerometerExhaustEnabled
    );

    ControllerPreferences.putFloat(
        "trig",
        ConfigurationPtr->ExhaustTriggerThresholdG
    );

    ControllerPreferences.putUChar(
        "bright",
        ConfigurationPtr->ActiveBrightnessPercent
    );

    ControllerPreferences.putUChar(
        "dim",
        ConfigurationPtr->DimBrightnessPercent
    );

    ControllerPreferences.putUChar(
        "fanspeed",
        ConfigurationPtr->FanSpeedPercent
    );

    ControllerPreferences.putUChar("exhaust1", ConfigurationPtr->ExhaustLight1Channel);
    ControllerPreferences.putUChar("exhaust2", ConfigurationPtr->ExhaustLight2Channel);
    ControllerPreferences.putUChar("passivech", ConfigurationPtr->PassiveLightsChannel);
    ControllerPreferences.putUChar("tailch", ConfigurationPtr->TailLightsChannel);
    ControllerPreferences.putUChar("leftch", ConfigurationPtr->LeftTurnLightsChannel);
    ControllerPreferences.putUChar("rightch", ConfigurationPtr->RightTurnLightsChannel);
    ControllerPreferences.putUChar("servoch", ConfigurationPtr->HeadlightServoChannel);
    ControllerPreferences.putUShort("servoclose", ConfigurationPtr->ServoClosedPulseUs);
    ControllerPreferences.putUShort("servoopen", ConfigurationPtr->ServoOpenPulseUs);

    for (int Channel = 0; Channel < 16; ++Channel)
    {
        char Key[8] = {};
        snprintf(Key, sizeof(Key), "role%d", Channel);
        ControllerPreferences.putUChar(
            Key,
            ConfigurationPtr->ChannelRoles[Channel]
        );
    }

    ControllerPreferences.end();
    return true;
}

void FControllerProtocol::SendAcknowledgement(
    const char* aKeyPtr,
    const char* aValuePtr
)
{
    if (
        BluetoothSerialPortPtr == nullptr ||
        !BluetoothSerialPortPtr->hasClient()
    )
    {
        return;
    }

    BluetoothSerialPortPtr->printf(
        "ACK,%s,%s\n",
        aKeyPtr,
        aValuePtr
    );
}

void FControllerProtocol::SendError(
    const char* aErrorPtr
)
{
    if (
        BluetoothSerialPortPtr == nullptr ||
        !BluetoothSerialPortPtr->hasClient()
    )
    {
        return;
    }

    BluetoothSerialPortPtr->printf(
        "ERR,%s\n",
        aErrorPtr
    );
}

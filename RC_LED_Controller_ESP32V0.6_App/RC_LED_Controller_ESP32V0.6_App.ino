#include <Arduino.h>
#include <BluetoothSerial.h>

#include "AccelerometerController.h"
#include "ControllerConfiguration.h"
#include "ControllerProtocol.h"
#include "LightingController.h"

// =============================================================================
// MPU6050 CONFIGURATION
// =============================================================================

static constexpr uint8_t MPU6050_ADDRESS = 0x68;

// Change these values to match how the MPU6050 is mounted in the car.
//
// Default orientation:
// X axis -> front of the car
// Y axis -> side of the car
// Z axis -> top of the car
static constexpr EAccelerometerAxis ACCELEROMETER_FORWARD_AXIS =
EAccelerometerAxis::X;

static constexpr EAccelerometerAxis ACCELEROMETER_SIDE_AXIS =
EAccelerometerAxis::Y;

static constexpr EAccelerometerAxis ACCELEROMETER_VERTICAL_AXIS =
EAccelerometerAxis::Z;

// Change this to true if forward movement produces a negative filtered value.
static constexpr bool ACCELEROMETER_FORWARD_AXIS_INVERTED = false;

// =============================================================================
// ESP32 RC INPUT PINS
// =============================================================================

static constexpr uint8_t RC_STEERING_PIN = 35;
static constexpr uint8_t RC_THROTTLE_PIN = 25;

// =============================================================================
// RC SIGNAL VALIDATION
// =============================================================================

static constexpr uint16_t RC_VALID_MIN_US = 750;
static constexpr uint16_t RC_VALID_MAX_US = 2250;

static constexpr uint32_t RC_SIGNAL_LOST_TIMEOUT_US =
100000;

// =============================================================================
// CH1 STEERING CALIBRATION
// =============================================================================

// Full physical right: approximately 1254 us
// Center:              approximately 1520 us
// Full physical left:  approximately 1786 us

static constexpr uint16_t RC_STEERING_MIN_US = 1254;
static constexpr uint16_t RC_STEERING_CENTER_US = 1520;
static constexpr uint16_t RC_STEERING_MAX_US = 1786;

static constexpr uint16_t RC_STEERING_DEADZONE_US = 20;

// Lower pulse widths represent physical right.
static constexpr bool STEERING_DIRECTION_REVERSED = true;

// =============================================================================
// CH2 THROTTLE CALIBRATION
// =============================================================================

// Full reverse/brake: approximately 1070 us
// Neutral:            approximately 1495 us
// Full forward:       approximately 1834 us

static constexpr uint16_t RC_THROTTLE_MIN_US = 1070;
static constexpr uint16_t RC_THROTTLE_CENTER_US = 1495;
static constexpr uint16_t RC_THROTTLE_MAX_US = 1834;

static constexpr uint16_t RC_THROTTLE_DEADZONE_US = 25;

// =============================================================================
// UPDATE TIMING
// =============================================================================

static constexpr uint32_t OUTPUT_UPDATE_INTERVAL_MS = 10;
static constexpr uint32_t SERIAL_PRINT_INTERVAL_MS = 250;
static constexpr uint32_t BLUETOOTH_TELEMETRY_INTERVAL_MS = 100;

// =============================================================================
// RC CHANNEL DATA
// =============================================================================

struct FRcChannelCapture
{
    volatile uint32_t RiseTimeUs = 0;
    volatile uint32_t PulseWidthUs = 0;
    volatile uint32_t LastValidPulseTimeUs = 0;
};

struct FRcChannelSnapshot
{
    uint32_t PulseWidthUs = 0;
    uint32_t LastValidPulseTimeUs = 0;

    bool bHasSignal = false;

    int16_t InputPercent = 0;
};

FRcChannelCapture SteeringCapture;
FRcChannelCapture ThrottleCapture;

// =============================================================================
// CONTROLLERS
// =============================================================================

FLightingController LightingController;
FAccelerometerController AccelerometerController;
FControllerConfiguration ControllerConfiguration;
FControllerProtocol ControllerProtocol;

BluetoothSerial BluetoothSerialPort;

// =============================================================================
// TIMING STATE
// =============================================================================

uint32_t LastOutputUpdateTimeMs = 0;
uint32_t LastSerialPrintTimeMs = 0;
uint32_t LastBluetoothTelemetryTimeMs = 0;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

void IRAM_ATTR HandleSteeringSignal();
void IRAM_ATTR HandleThrottleSignal();


FRcChannelSnapshot ReadRcChannelSnapshot(
    FRcChannelCapture& aCaptureRef,
    uint16_t aMinimumPulseUs,
    uint16_t aCenterPulseUs,
    uint16_t aMaximumPulseUs,
    uint16_t aDeadzoneUs,
    bool aReverseDirection
);

int16_t ConvertPulseToPercent(
    uint32_t aPulseWidthUs,
    uint16_t aMinimumPulseUs,
    uint16_t aCenterPulseUs,
    uint16_t aMaximumPulseUs,
    uint16_t aDeadzoneUs
);

void PrintDiagnostics(
    const FRcChannelSnapshot& aSteeringSnapshotRef,
    const FRcChannelSnapshot& aThrottleSnapshotRef,
    const FAccelerationState& aAccelerationStateRef,
    uint32_t aCurrentTimeMs
);

void SendBluetoothTelemetry(
    const FRcChannelSnapshot& aSteeringSnapshotRef,
    const FRcChannelSnapshot& aThrottleSnapshotRef,
    const FAccelerationState& aAccelerationStateRef,
    uint32_t aCurrentTimeMs
);

const char* GetTurnDirectionText();

// =============================================================================
// SETUP
// =============================================================================

void setup()
{
    Serial.begin(
        115200
    );

    delay(
        1000
    );

    Serial.println();
    Serial.println(
        "Starting ESP32 RC controller..."
    );

    BluetoothSerialPort.begin(
        "RC-Light-Controller"
    );

    ControllerProtocol.Begin(
        BluetoothSerialPort,
        ControllerConfiguration
    );

    Serial.println(
        "Bluetooth device: RC-Light-Controller"
    );

    // The receiver actively drives these signal lines.
    pinMode(
        RC_STEERING_PIN,
        INPUT
    );

    pinMode(
        RC_THROTTLE_PIN,
        INPUT
    );

    attachInterrupt(
        digitalPinToInterrupt(
            RC_STEERING_PIN
        ),
        HandleSteeringSignal,
        CHANGE
    );

    attachInterrupt(
        digitalPinToInterrupt(
            RC_THROTTLE_PIN
        ),
        HandleThrottleSignal,
        CHANGE
    );

    if (!LightingController.Begin())
    {
        Serial.println(
            "ERROR: PCA9685 initialization failed."
        );

        BluetoothSerialPort.println(
            "ERROR: PCA9685 initialization failed."
        );
    }
    else
    {
        Serial.println(
            "PCA9685 lighting controller initialized."
        );

        LightingController.ApplyConfiguration(
            ControllerConfiguration
        );
    }

    FAccelerometerAxisConfiguration AxisConfiguration;

    AxisConfiguration.ForwardAxis =
        ACCELEROMETER_FORWARD_AXIS;

    AxisConfiguration.bForwardAxisInverted =
        ACCELEROMETER_FORWARD_AXIS_INVERTED;

    AxisConfiguration.SideAxis =
        ACCELEROMETER_SIDE_AXIS;

    AxisConfiguration.VerticalAxis =
        ACCELEROMETER_VERTICAL_AXIS;

    if (
        !AccelerometerController.SetAxisConfiguration(
            AxisConfiguration
        )
        )
    {
        Serial.println(
            "ERROR: Invalid MPU6050 axis configuration."
        );
    }
    else if (
        !AccelerometerController.Begin(
            Wire,
            MPU6050_ADDRESS
        )
        )
    {
        Serial.println(
            "ERROR: MPU6050 initialization failed."
        );

        BluetoothSerialPort.println(
            "ERROR: MPU6050 initialization failed."
        );
    }
    else
    {
        Serial.println(
            "Keep the car stationary: calibrating MPU6050..."
        );

        BluetoothSerialPort.println(
            "Keep the car stationary: calibrating MPU6050..."
        );

        delay(
            500
        );

        if (
            AccelerometerController.CalibrateStationary()
            )
        {
            Serial.println(
                "MPU6050 accelerometer initialized and calibrated."
            );

            BluetoothSerialPort.println(
                "MPU6050 accelerometer initialized and calibrated."
            );
        }
        else
        {
            Serial.println(
                "ERROR: MPU6050 calibration failed."
            );

            BluetoothSerialPort.println(
                "ERROR: MPU6050 calibration failed."
            );
        }
    }

    Serial.println(
        "CH1 steering signal -> GPIO35"
    );

    Serial.println(
        "CH2 throttle signal -> GPIO25"
    );

    BluetoothSerialPort.println();
    BluetoothSerialPort.println(
        "ESP32 RC controller connected."
    );

    BluetoothSerialPort.println(
        "CH1 steering signal -> GPIO35"
    );

    BluetoothSerialPort.println(
        "CH2 throttle signal -> GPIO25"
    );

    BluetoothSerialPort.println(
        "Waiting for RC signals..."
    );
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop()
{
    const uint32_t CurrentTimeMs =
        millis();

    ControllerProtocol.Update();

    if (
        ControllerProtocol.ConsumeConfigurationChanged()
    )
    {
        LightingController.ApplyConfiguration(
            ControllerConfiguration
        );
    }

    if (
        ControllerProtocol.ConsumeExhaustTestRequested()
    )
    {
        LightingController.TriggerExhaustTest(
            CurrentTimeMs
        );
    }

    if (
        CurrentTimeMs -
        LastOutputUpdateTimeMs <
        OUTPUT_UPDATE_INTERVAL_MS
        )
    {
        return;
    }

    LastOutputUpdateTimeMs =
        CurrentTimeMs;

    const FRcChannelSnapshot SteeringSnapshot =
        ReadRcChannelSnapshot(
            SteeringCapture,
            RC_STEERING_MIN_US,
            RC_STEERING_CENTER_US,
            RC_STEERING_MAX_US,
            RC_STEERING_DEADZONE_US,
            STEERING_DIRECTION_REVERSED
        );

    const FRcChannelSnapshot ThrottleSnapshot =
        ReadRcChannelSnapshot(
            ThrottleCapture,
            RC_THROTTLE_MIN_US,
            RC_THROTTLE_CENTER_US,
            RC_THROTTLE_MAX_US,
            RC_THROTTLE_DEADZONE_US,
            false
        );

    if (ControllerProtocol.ConsumeDiagnosticsRequested())
    {
        ControllerProtocol.SendDiagnostics(
            LightingController.IsConnected(),
            LightingController.GetDeviceAddress(),
            LightingController.ReadMode1Register(),
            AccelerometerController.IsConnected(),
            AccelerometerController.IsCalibrated(),
            AccelerometerController.GetDeviceAddress(),
            AccelerometerController.GetWhoAmI(),
            SteeringSnapshot.bHasSignal,
            ThrottleSnapshot.bHasSignal
        );
    }

    AccelerometerController.Update(
        CurrentTimeMs
    );

    const FAccelerationState& AccelerationStateRef =
        AccelerometerController.GetState();

    FLightingInputState LightingInputState;

    LightingInputState.bSteeringHasSignal =
        SteeringSnapshot.bHasSignal;

    LightingInputState.SteeringPercent =
        SteeringSnapshot.InputPercent;

    LightingInputState.bThrottleHasSignal =
        ThrottleSnapshot.bHasSignal;

    LightingInputState.ThrottlePercent =
        ThrottleSnapshot.InputPercent;

    LightingInputState.bAccelerometerHasSample =
        AccelerationStateRef.bHasValidSample;

    LightingInputState.FilteredForwardAccelerationG =
        AccelerationStateRef.FilteredForwardAccelerationG;

    LightingController.Update(
        LightingInputState,
        CurrentTimeMs
    );

    PrintDiagnostics(
        SteeringSnapshot,
        ThrottleSnapshot,
        AccelerationStateRef,
        CurrentTimeMs
    );

    SendBluetoothTelemetry(
        SteeringSnapshot,
        ThrottleSnapshot,
        AccelerationStateRef,
        CurrentTimeMs
    );
}

// =============================================================================
// RC INTERRUPTS
// =============================================================================

void IRAM_ATTR HandleSteeringSignal()
{
    const uint32_t CurrentTimeUs =
        micros();

    if (
        digitalRead(
            RC_STEERING_PIN
        ) == HIGH
        )
    {
        SteeringCapture.RiseTimeUs =
            CurrentTimeUs;

        return;
    }

    const uint32_t MeasuredPulseWidthUs =
        CurrentTimeUs -
        SteeringCapture.RiseTimeUs;

    if (
        MeasuredPulseWidthUs >= RC_VALID_MIN_US &&
        MeasuredPulseWidthUs <= RC_VALID_MAX_US
        )
    {
        SteeringCapture.PulseWidthUs =
            MeasuredPulseWidthUs;

        SteeringCapture.LastValidPulseTimeUs =
            CurrentTimeUs;
    }
}

void IRAM_ATTR HandleThrottleSignal()
{
    const uint32_t CurrentTimeUs =
        micros();

    if (
        digitalRead(
            RC_THROTTLE_PIN
        ) == HIGH
        )
    {
        ThrottleCapture.RiseTimeUs =
            CurrentTimeUs;

        return;
    }

    const uint32_t MeasuredPulseWidthUs =
        CurrentTimeUs -
        ThrottleCapture.RiseTimeUs;

    if (
        MeasuredPulseWidthUs >= RC_VALID_MIN_US &&
        MeasuredPulseWidthUs <= RC_VALID_MAX_US
        )
    {
        ThrottleCapture.PulseWidthUs =
            MeasuredPulseWidthUs;

        ThrottleCapture.LastValidPulseTimeUs =
            CurrentTimeUs;
    }
}

// =============================================================================
// RC CHANNEL READING
// =============================================================================

FRcChannelSnapshot ReadRcChannelSnapshot(
    FRcChannelCapture& aCaptureRef,
    uint16_t aMinimumPulseUs,
    uint16_t aCenterPulseUs,
    uint16_t aMaximumPulseUs,
    uint16_t aDeadzoneUs,
    bool aReverseDirection
)
{
    FRcChannelSnapshot Snapshot;

    noInterrupts();

    Snapshot.PulseWidthUs =
        aCaptureRef.PulseWidthUs;

    Snapshot.LastValidPulseTimeUs =
        aCaptureRef.LastValidPulseTimeUs;

    interrupts();

    const uint32_t CurrentTimeUs =
        micros();

    Snapshot.bHasSignal =
        Snapshot.PulseWidthUs >= RC_VALID_MIN_US &&
        Snapshot.PulseWidthUs <= RC_VALID_MAX_US &&
        Snapshot.LastValidPulseTimeUs != 0 &&
        CurrentTimeUs -
        Snapshot.LastValidPulseTimeUs <=
        RC_SIGNAL_LOST_TIMEOUT_US;

    if (!Snapshot.bHasSignal)
    {
        Snapshot.InputPercent = 0;
        return Snapshot;
    }

    Snapshot.InputPercent =
        ConvertPulseToPercent(
            Snapshot.PulseWidthUs,
            aMinimumPulseUs,
            aCenterPulseUs,
            aMaximumPulseUs,
            aDeadzoneUs
        );

    if (aReverseDirection)
    {
        Snapshot.InputPercent =
            -Snapshot.InputPercent;
    }

    return Snapshot;
}

// =============================================================================
// RC PULSE CONVERSION
// =============================================================================

int16_t ConvertPulseToPercent(
    uint32_t aPulseWidthUs,
    uint16_t aMinimumPulseUs,
    uint16_t aCenterPulseUs,
    uint16_t aMaximumPulseUs,
    uint16_t aDeadzoneUs
)
{
    const int32_t DistanceFromCenterUs =
        static_cast<int32_t>(
            aPulseWidthUs
            ) -
        static_cast<int32_t>(
            aCenterPulseUs
            );

    if (
        abs(DistanceFromCenterUs) <=
        aDeadzoneUs
        )
    {
        return 0;
    }

    if (aPulseWidthUs < aCenterPulseUs)
    {
        const uint32_t ClampedPulseWidthUs =
            constrain(
                aPulseWidthUs,
                static_cast<uint32_t>(
                    aMinimumPulseUs
                    ),
                static_cast<uint32_t>(
                    aCenterPulseUs
                    )
            );

        return static_cast<int16_t>(
            map(
                ClampedPulseWidthUs,
                aMinimumPulseUs,
                aCenterPulseUs,
                -100,
                0
            )
            );
    }

    const uint32_t ClampedPulseWidthUs =
        constrain(
            aPulseWidthUs,
            static_cast<uint32_t>(
                aCenterPulseUs
                ),
            static_cast<uint32_t>(
                aMaximumPulseUs
                )
        );

    return static_cast<int16_t>(
        map(
            ClampedPulseWidthUs,
            aCenterPulseUs,
            aMaximumPulseUs,
            0,
            100
        )
        );
}

// =============================================================================
// DIAGNOSTICS
// =============================================================================

void PrintDiagnostics(
    const FRcChannelSnapshot& aSteeringSnapshotRef,
    const FRcChannelSnapshot& aThrottleSnapshotRef,
    const FAccelerationState& aAccelerationStateRef,
    uint32_t aCurrentTimeMs
)
{
    if (
        aCurrentTimeMs -
        LastSerialPrintTimeMs <
        SERIAL_PRINT_INTERVAL_MS
        )
    {
        return;
    }

    LastSerialPrintTimeMs =
        aCurrentTimeMs;

    char SteeringText[80];
    char ThrottleText[80];
    char MotionText[200];
    char CompleteDiagnosticText[400];

    if (aSteeringSnapshotRef.bHasSignal)
    {
        const char* TurnStateText =
            "TURN OFF";

        switch (
            LightingController.GetActiveTurnDirection()
            )
        {
        case ETurnDirection::Left:
        {
            TurnStateText =
                "LEFT TURN";

            break;
        }

        case ETurnDirection::Right:
        {
            TurnStateText =
                "RIGHT TURN";

            break;
        }

        default:
        {
            TurnStateText =
                "TURN OFF";

            break;
        }
        }

        snprintf(
            SteeringText,
            sizeof(SteeringText),
            "CH1: %4lu us | %4d%% | %s",
            static_cast<unsigned long>(
                aSteeringSnapshotRef.PulseWidthUs
                ),
            aSteeringSnapshotRef.InputPercent,
            TurnStateText
        );
    }
    else
    {
        snprintf(
            SteeringText,
            sizeof(SteeringText),
            "CH1: NO SIGNAL | TURN OFF"
        );
    }

    if (aThrottleSnapshotRef.bHasSignal)
    {
        snprintf(
            ThrottleText,
            sizeof(ThrottleText),
            "CH2: %4lu us | %4d%% | %s",
            static_cast<unsigned long>(
                aThrottleSnapshotRef.PulseWidthUs
                ),
            aThrottleSnapshotRef.InputPercent,
            LightingController.IsBrakeOrReverseActive()
            ? "REAR BRIGHT"
            : "REAR DIM"
        );
    }
    else
    {
        snprintf(
            ThrottleText,
            sizeof(ThrottleText),
            "CH2: NO SIGNAL | REAR DIM"
        );
    }

    if (aAccelerationStateRef.bHasValidSample)
    {
        snprintf(
            MotionText,
            sizeof(MotionText),
            "MPU ACC: FWD %+0.3f g | FILT %+0.3f g | SIDE %+0.3f g | VERT %+0.3f g | GYRO: X %+0.1f dps | Y %+0.1f dps | Z %+0.1f dps",
            aAccelerationStateRef.ForwardAccelerationG,
            aAccelerationStateRef.FilteredForwardAccelerationG,
            aAccelerationStateRef.SideAccelerationG,
            aAccelerationStateRef.VerticalAccelerationG,
            aAccelerationStateRef.GyroscopeXDps,
            aAccelerationStateRef.GyroscopeYDps,
            aAccelerationStateRef.GyroscopeZDps
        );
    }
    else if (
        AccelerometerController.IsConnected() &&
        AccelerometerController.IsCalibrated()
        )
    {
        snprintf(
            MotionText,
            sizeof(MotionText),
            "MPU: WAITING FOR SAMPLE"
        );
    }
    else
    {
        snprintf(
            MotionText,
            sizeof(MotionText),
            "MPU: OFFLINE"
        );
    }

    snprintf(
        CompleteDiagnosticText,
        sizeof(CompleteDiagnosticText),
        "%s    ||    %s    ||    %s",
        SteeringText,
        ThrottleText,
        MotionText
    );

    Serial.println(
        CompleteDiagnosticText
    );

}

// =============================================================================
// STRUCTURED BLUETOOTH TELEMETRY
// =============================================================================

void SendBluetoothTelemetry(
    const FRcChannelSnapshot& aSteeringSnapshotRef,
    const FRcChannelSnapshot& aThrottleSnapshotRef,
    const FAccelerationState& aAccelerationStateRef,
    uint32_t aCurrentTimeMs
)
{
    if (
        aCurrentTimeMs -
        LastBluetoothTelemetryTimeMs <
        BLUETOOTH_TELEMETRY_INTERVAL_MS
    )
    {
        return;
    }

    LastBluetoothTelemetryTimeMs =
        aCurrentTimeMs;

    ControllerProtocol.SendTelemetry(
        aCurrentTimeMs,
        aSteeringSnapshotRef.bHasSignal,
        aSteeringSnapshotRef.PulseWidthUs,
        aSteeringSnapshotRef.InputPercent,
        aThrottleSnapshotRef.bHasSignal,
        aThrottleSnapshotRef.PulseWidthUs,
        aThrottleSnapshotRef.InputPercent,
        aAccelerationStateRef.ForwardAccelerationG,
        aAccelerationStateRef.FilteredForwardAccelerationG,
        aAccelerationStateRef.SideAccelerationG,
        aAccelerationStateRef.VerticalAccelerationG,
        aAccelerationStateRef.GyroscopeXDps,
        aAccelerationStateRef.GyroscopeYDps,
        aAccelerationStateRef.GyroscopeZDps,
        GetTurnDirectionText(),
        LightingController.IsBrakeOrReverseActive(),
        LightingController.IsExhaustPulseActive()
    );
}

const char* GetTurnDirectionText()
{
    switch (
        LightingController.GetActiveTurnDirection()
    )
    {
    case ETurnDirection::Left:
    {
        return "LEFT";
    }

    case ETurnDirection::Right:
    {
        return "RIGHT";
    }

    default:
    {
        return "NONE";
    }
    }
}

#include "AccelerometerController.h"

#include <math.h>

namespace
{
    static constexpr uint8_t MPU6050_WHO_AM_I_REGISTER = 0x75;
    static constexpr uint8_t MPU6050_POWER_MANAGEMENT_1_REGISTER = 0x6B;
    static constexpr uint8_t MPU6050_SAMPLE_RATE_DIVIDER_REGISTER = 0x19;
    static constexpr uint8_t MPU6050_CONFIGURATION_REGISTER = 0x1A;
    static constexpr uint8_t MPU6050_GYROSCOPE_CONFIGURATION_REGISTER = 0x1B;
    static constexpr uint8_t MPU6050_ACCELEROMETER_CONFIGURATION_REGISTER = 0x1C;
    static constexpr uint8_t MPU6050_INTERRUPT_ENABLE_REGISTER = 0x38;
    static constexpr uint8_t MPU6050_MOTION_OUTPUT_START_REGISTER = 0x3B;

    static constexpr uint8_t MPU6050_EXPECTED_DEVICE_ID = 0x68;
    static constexpr uint8_t MPU6500_EXPECTED_DEVICE_ID = 0x70;
    static constexpr uint8_t MPU9250_EXPECTED_DEVICE_ID = 0x71;

    // With the MPU6050 digital low-pass filter enabled, the internal sample
    // rate is 1 kHz. Divider 9 produces a 100 Hz accelerometer update rate.
    static constexpr uint8_t MPU6050_SAMPLE_RATE_DIVIDER = 9;

    // DLPF configuration 3 gives approximately 44 Hz accelerometer bandwidth.
    static constexpr uint8_t MPU6050_DLPF_CONFIGURATION = 3;

    // AFS_SEL = 1 selects the +/-4 g accelerometer range.
    static constexpr uint8_t MPU6050_ACCELEROMETER_RANGE_4G = 0x08;

    static constexpr float MPU6050_ACCELEROMETER_SCALE_LSB_PER_G = 8192.0f;

    // FS_SEL = 0 selects the +/-250 degrees-per-second gyroscope range.
    static constexpr uint8_t MPU6050_GYROSCOPE_RANGE_250_DPS = 0x00;
    static constexpr float MPU6050_GYROSCOPE_SCALE_LSB_PER_DPS = 131.0f;

    static constexpr uint32_t ACCELEROMETER_SAMPLE_INTERVAL_MS = 10;

    // New reading contributes 15%; retained history contributes 85%.
    static constexpr float FORWARD_ACCELERATION_FILTER_ALPHA = 0.15f;
}

FAccelerometerController::FAccelerometerController()
{
}

bool FAccelerometerController::Begin(
    TwoWire& aWireRef,
    uint8_t aDeviceAddress
)
{
    WirePtr = &aWireRef;
    DeviceAddress = aDeviceAddress;
    DeviceWhoAmI = 0;

    bConnected = false;
    bCalibrated = false;
    LastSampleTimeMs = 0;

    AccelerationState = FAccelerationState();
    ResetFilter();

    if (!IsAxisConfigurationValid(AxisConfiguration))
    {
        return false;
    }

    uint8_t DeviceId = 0;

    if (
        !ReadRegister(
            MPU6050_WHO_AM_I_REGISTER,
            DeviceId
        )
    )
    {
        return false;
    }

    Serial.printf(
        "MPU WHO_AM_I = 0x%02X\n",
        DeviceId
    );

    DeviceWhoAmI = DeviceId;

    const bool bIsMPU6050 =
        (DeviceId & 0x7E) ==
        MPU6050_EXPECTED_DEVICE_ID;

    const bool bIsMPU6500 =
        DeviceId ==
        MPU6500_EXPECTED_DEVICE_ID;

    const bool bIsMPU9250 =
        DeviceId ==
        MPU9250_EXPECTED_DEVICE_ID;

    if (!bIsMPU6050 && !bIsMPU6500 && !bIsMPU9250)
    {
        return false;
    }

    if (!ConfigureDevice())
    {
        return false;
    }

    bConnected = true;
    return true;
}

bool FAccelerometerController::SetAxisConfiguration(
    const FAccelerometerAxisConfiguration& aAxisConfigurationRef
)
{
    if (!IsAxisConfigurationValid(aAxisConfigurationRef))
    {
        return false;
    }

    AxisConfiguration = aAxisConfigurationRef;
    ResetFilter();

    return true;
}

bool FAccelerometerController::CalibrateStationary(
    uint16_t aSampleCount,
    uint16_t aSampleDelayMs
)
{
    if (
        !bConnected ||
        WirePtr == nullptr ||
        aSampleCount == 0
    )
    {
        return false;
    }

    int64_t AccumulatedRawX = 0;
    int64_t AccumulatedRawY = 0;
    int64_t AccumulatedRawZ = 0;

    int64_t AccumulatedRawGyroscopeX = 0;
    int64_t AccumulatedRawGyroscopeY = 0;
    int64_t AccumulatedRawGyroscopeZ = 0;

    uint16_t ValidSampleCount = 0;

    for (
        uint16_t SampleIndex = 0;
        SampleIndex < aSampleCount;
        ++SampleIndex
    )
    {
        int16_t RawX = 0;
        int16_t RawY = 0;
        int16_t RawZ = 0;

        int16_t RawGyroscopeX = 0;
        int16_t RawGyroscopeY = 0;
        int16_t RawGyroscopeZ = 0;

        if (
            ReadRawMotion(
                RawX,
                RawY,
                RawZ,
                RawGyroscopeX,
                RawGyroscopeY,
                RawGyroscopeZ
            )
            )
        {
            AccumulatedRawX += RawX;
            AccumulatedRawY += RawY;
            AccumulatedRawZ += RawZ;

            AccumulatedRawGyroscopeX +=
                RawGyroscopeX;

            AccumulatedRawGyroscopeY +=
                RawGyroscopeY;

            AccumulatedRawGyroscopeZ +=
                RawGyroscopeZ;

            ++ValidSampleCount;
        }

        if (aSampleDelayMs > 0)
        {
            delay(aSampleDelayMs);
        }
    }

    if (ValidSampleCount == 0)
    {
        bCalibrated = false;
        return false;
    }

    StationaryOffsetXG =
        (
            static_cast<float>(AccumulatedRawX) /
            static_cast<float>(ValidSampleCount)
        ) /
        MPU6050_ACCELEROMETER_SCALE_LSB_PER_G;

    StationaryOffsetYG =
        (
            static_cast<float>(AccumulatedRawY) /
            static_cast<float>(ValidSampleCount)
        ) /
        MPU6050_ACCELEROMETER_SCALE_LSB_PER_G;

    StationaryOffsetZG =
        (
            static_cast<float>(AccumulatedRawZ) /
            static_cast<float>(ValidSampleCount)
        ) /
        MPU6050_ACCELEROMETER_SCALE_LSB_PER_G;

    StationaryGyroscopeOffsetXDps =
        (
            static_cast<float>(
                AccumulatedRawGyroscopeX
            ) /
            static_cast<float>(ValidSampleCount)
        ) /
        MPU6050_GYROSCOPE_SCALE_LSB_PER_DPS;

    StationaryGyroscopeOffsetYDps =
        (
            static_cast<float>(
                AccumulatedRawGyroscopeY
            ) /
            static_cast<float>(ValidSampleCount)
        ) /
        MPU6050_GYROSCOPE_SCALE_LSB_PER_DPS;

    StationaryGyroscopeOffsetZDps =
        (
            static_cast<float>(
                AccumulatedRawGyroscopeZ
            ) /
            static_cast<float>(ValidSampleCount)
        ) /
        MPU6050_GYROSCOPE_SCALE_LSB_PER_DPS;

    bCalibrated = true;
    AccelerationState = FAccelerationState();
    LastSampleTimeMs = 0;
    ResetFilter();

    return true;
}

bool FAccelerometerController::Update(uint32_t aCurrentTimeMs)
{
    if (
        !bConnected ||
        !bCalibrated ||
        WirePtr == nullptr
    )
    {
        return false;
    }

    if (
        AccelerationState.bHasValidSample &&
        aCurrentTimeMs - LastSampleTimeMs <
        ACCELEROMETER_SAMPLE_INTERVAL_MS
    )
    {
        return false;
    }

    int16_t RawX = 0;
    int16_t RawY = 0;
    int16_t RawZ = 0;

    int16_t RawGyroscopeX = 0;
    int16_t RawGyroscopeY = 0;
    int16_t RawGyroscopeZ = 0;

    if (
        !ReadRawMotion(
            RawX,
            RawY,
            RawZ,
            RawGyroscopeX,
            RawGyroscopeY,
            RawGyroscopeZ
        )
        )
    {
        AccelerationState.bHasValidSample = false;
        return false;
    }

    const float XG =
        (
            static_cast<float>(RawX) /
            MPU6050_ACCELEROMETER_SCALE_LSB_PER_G
        ) -
        StationaryOffsetXG;

    const float YG =
        (
            static_cast<float>(RawY) /
            MPU6050_ACCELEROMETER_SCALE_LSB_PER_G
        ) -
        StationaryOffsetYG;

    const float ZG =
        (
            static_cast<float>(RawZ) /
            MPU6050_ACCELEROMETER_SCALE_LSB_PER_G
        ) -
        StationaryOffsetZG;

    const float GyroscopeXDps =
        (
            static_cast<float>(RawGyroscopeX) /
            MPU6050_GYROSCOPE_SCALE_LSB_PER_DPS
        ) -
        StationaryGyroscopeOffsetXDps;

    const float GyroscopeYDps =
        (
            static_cast<float>(RawGyroscopeY) /
            MPU6050_GYROSCOPE_SCALE_LSB_PER_DPS
        ) -
        StationaryGyroscopeOffsetYDps;

    const float GyroscopeZDps =
        (
            static_cast<float>(RawGyroscopeZ) /
            MPU6050_GYROSCOPE_SCALE_LSB_PER_DPS
        ) -
        StationaryGyroscopeOffsetZDps;

    const float ForwardAccelerationG =
        GetConfiguredAxisValue(
            AxisConfiguration.ForwardAxis,
            AxisConfiguration.bForwardAxisInverted,
            XG,
            YG,
            ZG
        );

    const float SideAccelerationG =
        GetConfiguredAxisValue(
            AxisConfiguration.SideAxis,
            AxisConfiguration.bSideAxisInverted,
            XG,
            YG,
            ZG
        );

    const float VerticalAccelerationG =
        GetConfiguredAxisValue(
            AxisConfiguration.VerticalAxis,
            AxisConfiguration.bVerticalAxisInverted,
            XG,
            YG,
            ZG
        );

    if (!bFilterInitialized)
    {
        AccelerationState.FilteredForwardAccelerationG =
            ForwardAccelerationG;

        bFilterInitialized = true;
    }
    else
    {
        AccelerationState.FilteredForwardAccelerationG =
            (
                AccelerationState.FilteredForwardAccelerationG *
                (1.0f - FORWARD_ACCELERATION_FILTER_ALPHA)
            ) +
            (
                ForwardAccelerationG *
                FORWARD_ACCELERATION_FILTER_ALPHA
            );
    }

    AccelerationState.ForwardAccelerationG =
        ForwardAccelerationG;

    AccelerationState.SideAccelerationG =
        SideAccelerationG;

    AccelerationState.VerticalAccelerationG =
        VerticalAccelerationG;

    AccelerationState.TotalDynamicAccelerationG =
        sqrtf(
            XG * XG +
            YG * YG +
            ZG * ZG
        );

    AccelerationState.GyroscopeXDps =
        GyroscopeXDps;

    AccelerationState.GyroscopeYDps =
        GyroscopeYDps;

    AccelerationState.GyroscopeZDps =
        GyroscopeZDps;

    AccelerationState.TotalAngularVelocityDps =
        sqrtf(
            GyroscopeXDps * GyroscopeXDps +
            GyroscopeYDps * GyroscopeYDps +
            GyroscopeZDps * GyroscopeZDps
        );

    AccelerationState.SampleTimeMs =
        aCurrentTimeMs;

    AccelerationState.bHasValidSample =
        true;

    LastSampleTimeMs =
        aCurrentTimeMs;

    return true;
}

const FAccelerationState&
FAccelerometerController::GetState() const
{
    return AccelerationState;
}

bool FAccelerometerController::IsConnected() const
{
    return bConnected;
}

bool FAccelerometerController::IsCalibrated() const
{
    return bCalibrated;
}

uint8_t FAccelerometerController::GetDeviceAddress() const
{
    return DeviceAddress;
}

uint8_t FAccelerometerController::GetWhoAmI() const
{
    return DeviceWhoAmI;
}

void FAccelerometerController::ResetFilter()
{
    bFilterInitialized = false;
    AccelerationState.FilteredForwardAccelerationG = 0.0f;
}

bool FAccelerometerController::ConfigureDevice()
{
    if (
        !WriteRegister(
            MPU6050_POWER_MANAGEMENT_1_REGISTER,
            0x80
        )
    )
    {
        return false;
    }

    delay(100);

    // Wake the MPU6050 and select the X gyroscope PLL as its clock.
    if (
        !WriteRegister(
            MPU6050_POWER_MANAGEMENT_1_REGISTER,
            0x01
        )
    )
    {
        return false;
    }

    delay(10);

    return
        WriteRegister(
            MPU6050_SAMPLE_RATE_DIVIDER_REGISTER,
            MPU6050_SAMPLE_RATE_DIVIDER
        ) &&
        WriteRegister(
            MPU6050_CONFIGURATION_REGISTER,
            MPU6050_DLPF_CONFIGURATION
        ) &&
        WriteRegister(
            MPU6050_GYROSCOPE_CONFIGURATION_REGISTER,
            MPU6050_GYROSCOPE_RANGE_250_DPS
        ) &&
        WriteRegister(
            MPU6050_ACCELEROMETER_CONFIGURATION_REGISTER,
            MPU6050_ACCELEROMETER_RANGE_4G
        ) &&
        WriteRegister(
            MPU6050_INTERRUPT_ENABLE_REGISTER,
            0x00
        );
}

bool FAccelerometerController::ReadRawMotion(
    int16_t& aRawAccelerationXRef,
    int16_t& aRawAccelerationYRef,
    int16_t& aRawAccelerationZRef,
    int16_t& aRawGyroscopeXRef,
    int16_t& aRawGyroscopeYRef,
    int16_t& aRawGyroscopeZRef
)
{
    if (WirePtr == nullptr)
    {
        return false;
    }

    WirePtr->beginTransmission(DeviceAddress);
    WirePtr->write(
        MPU6050_MOTION_OUTPUT_START_REGISTER
    );

    if (WirePtr->endTransmission(false) != 0)
    {
        return false;
    }

    // Acceleration XYZ (6 bytes), temperature (2 bytes),
    // and gyroscope XYZ (6 bytes).
    const size_t RequestedByteCount = 14;
    const size_t ReceivedByteCount =
        WirePtr->requestFrom(
            DeviceAddress,
            RequestedByteCount,
            true
        );

    if (ReceivedByteCount != RequestedByteCount)
    {
        return false;
    }

    aRawAccelerationXRef =
        static_cast<int16_t>(
            (
                static_cast<uint16_t>(WirePtr->read()) <<
                8
            ) |
            static_cast<uint16_t>(WirePtr->read())
        );

    aRawAccelerationYRef =
        static_cast<int16_t>(
            (
                static_cast<uint16_t>(WirePtr->read()) <<
                8
            ) |
            static_cast<uint16_t>(WirePtr->read())
        );

    aRawAccelerationZRef =
        static_cast<int16_t>(
            (
                static_cast<uint16_t>(WirePtr->read()) <<
                8
            ) |
            static_cast<uint16_t>(WirePtr->read())
        );

    // The temperature value is not required by the lighting controller.
    WirePtr->read();
    WirePtr->read();

    aRawGyroscopeXRef =
        static_cast<int16_t>(
            (
                static_cast<uint16_t>(WirePtr->read()) <<
                8
            ) |
            static_cast<uint16_t>(WirePtr->read())
        );

    aRawGyroscopeYRef =
        static_cast<int16_t>(
            (
                static_cast<uint16_t>(WirePtr->read()) <<
                8
            ) |
            static_cast<uint16_t>(WirePtr->read())
        );

    aRawGyroscopeZRef =
        static_cast<int16_t>(
            (
                static_cast<uint16_t>(WirePtr->read()) <<
                8
            ) |
            static_cast<uint16_t>(WirePtr->read())
        );

    return true;
}

bool FAccelerometerController::WriteRegister(
    uint8_t aRegisterAddress,
    uint8_t aValue
)
{
    if (WirePtr == nullptr)
    {
        return false;
    }

    WirePtr->beginTransmission(DeviceAddress);
    WirePtr->write(aRegisterAddress);
    WirePtr->write(aValue);

    return WirePtr->endTransmission(true) == 0;
}

bool FAccelerometerController::ReadRegister(
    uint8_t aRegisterAddress,
    uint8_t& aValueRef
)
{
    if (WirePtr == nullptr)
    {
        return false;
    }

    WirePtr->beginTransmission(DeviceAddress);
    WirePtr->write(aRegisterAddress);

    if (WirePtr->endTransmission(false) != 0)
    {
        return false;
    }

    const size_t ReceivedByteCount =
        WirePtr->requestFrom(
            DeviceAddress,
            static_cast<size_t>(1),
            true
        );

    if (ReceivedByteCount != 1)
    {
        return false;
    }

    aValueRef =
        static_cast<uint8_t>(WirePtr->read());

    return true;
}

float FAccelerometerController::GetConfiguredAxisValue(
    EAccelerometerAxis aAxis,
    bool bAxisInverted,
    float aXG,
    float aYG,
    float aZG
) const
{
    float AxisValueG = 0.0f;

    switch (aAxis)
    {
        case EAccelerometerAxis::X:
        {
            AxisValueG = aXG;
            break;
        }

        case EAccelerometerAxis::Y:
        {
            AxisValueG = aYG;
            break;
        }

        case EAccelerometerAxis::Z:
        {
            AxisValueG = aZG;
            break;
        }
    }

    return bAxisInverted
        ? -AxisValueG
        : AxisValueG;
}

bool FAccelerometerController::IsAxisConfigurationValid(
    const FAccelerometerAxisConfiguration& aAxisConfigurationRef
) const
{
    return
        aAxisConfigurationRef.ForwardAxis !=
            aAxisConfigurationRef.SideAxis &&
        aAxisConfigurationRef.ForwardAxis !=
            aAxisConfigurationRef.VerticalAxis &&
        aAxisConfigurationRef.SideAxis !=
            aAxisConfigurationRef.VerticalAxis;
}

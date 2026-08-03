#include "DeviceController.h"

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothPermission>
#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>
#include <QBluetoothUuid>
#include <QCoreApplication>
#include <QMetaEnum>
#include <QPermission>
#include <QList>
#include <QPair>
#include <QSettings>
#include <QTimer>

namespace
{
    const QString TargetDeviceName =
        QStringLiteral("RC-Light-Controller");

    const QBluetoothUuid SerialPortServiceUuid(
        QBluetoothUuid::ServiceClassUuid::SerialPort
    );

    static constexpr qsizetype MaximumReceiveBufferSize =
        4096;
}

FDeviceController::FDeviceController(
    QObject* aParentPtr
)
    : QObject(aParentPtr)
    , DiscoveryAgentPtr(
        new QBluetoothDeviceDiscoveryAgent(this)
    )
    , BluetoothSocketPtr(
        new QBluetoothSocket(
            QBluetoothServiceInfo::RfcommProtocol,
            this
        )
    )
    , ReconnectTimerPtr(
        new QTimer(this)
    )
{
    ReconnectTimerPtr->setSingleShot(true);
    ReconnectTimerPtr->setInterval(2000);

    connect(
        ReconnectTimerPtr,
        &QTimer::timeout,
        this,
        &FDeviceController::StartScan
    );

    connect(
        DiscoveryAgentPtr,
        &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
        this,
        &FDeviceController::HandleDeviceDiscovered
    );

    connect(
        DiscoveryAgentPtr,
        &QBluetoothDeviceDiscoveryAgent::finished,
        this,
        &FDeviceController::HandleDiscoveryFinished
    );

    connect(
        DiscoveryAgentPtr,
        &QBluetoothDeviceDiscoveryAgent::canceled,
        this,
        &FDeviceController::HandleDiscoveryFinished
    );

    connect(
        DiscoveryAgentPtr,
        &QBluetoothDeviceDiscoveryAgent::errorOccurred,
        this,
        [this](
            QBluetoothDeviceDiscoveryAgent::Error aError
        )
        {
            Q_UNUSED(aError);

            SetConnectionState(
                false,
                false,
                DiscoveryAgentPtr->errorString().isEmpty()
                    ? QStringLiteral("Bluetooth scan failed")
                    : DiscoveryAgentPtr->errorString()
            );
        }
    );

    connect(
        BluetoothSocketPtr,
        &QBluetoothSocket::connected,
        this,
        &FDeviceController::HandleSocketConnected
    );

    connect(
        BluetoothSocketPtr,
        &QBluetoothSocket::disconnected,
        this,
        &FDeviceController::HandleSocketDisconnected
    );

    connect(
        BluetoothSocketPtr,
        &QBluetoothSocket::readyRead,
        this,
        &FDeviceController::HandleSocketReadyRead
    );

    connect(
        BluetoothSocketPtr,
        &QBluetoothSocket::errorOccurred,
        this,
        [this](QBluetoothSocket::SocketError)
        {
            HandleSocketError();
        }
    );

    LoadLocalSettings();
}

FDeviceController::~FDeviceController() = default;

bool FDeviceController::IsConnected() const
{
    return bConnected;
}

bool FDeviceController::IsScanning() const
{
    return bScanning;
}

const QString& FDeviceController::GetConnectionStatus() const
{
    return ConnectionStatus;
}

const QString& FDeviceController::GetDeviceName() const
{
    return DeviceName;
}

const QString& FDeviceController::GetFirmwareVersion() const
{
    return FirmwareVersion;
}

bool FDeviceController::ArePassiveLightsEnabled() const
{
    return bPassiveLightsEnabled;
}

bool FDeviceController::AreActiveLightsEnabled() const
{
    return bActiveLightsEnabled;
}

bool FDeviceController::IsExhaustEnabled() const
{
    return bExhaustEnabled;
}

bool FDeviceController::AreHeadlightsOpen() const
{
    return bHeadlightsOpen;
}

bool FDeviceController::AreFansEnabled() const
{
    return bFansEnabled;
}

bool FDeviceController::IsAccelerometerEnabled() const
{
    return bAccelerometerEnabled;
}

double FDeviceController::GetTriggerThresholdG() const
{
    return TriggerThresholdG;
}

int FDeviceController::GetAccelerometerForwardAxis() const
{
    return AccelerometerForwardAxis;
}

bool FDeviceController::IsAccelerometerForwardInverted() const
{
    return bAccelerometerForwardInverted;
}

double FDeviceController::GetAccelerometerToleranceG() const
{
    return AccelerometerToleranceG;
}

int FDeviceController::GetActiveBrightnessPercent() const
{
    return ActiveBrightnessPercent;
}

int FDeviceController::GetDimBrightnessPercent() const
{
    return DimBrightnessPercent;
}

int FDeviceController::GetFanSpeedPercent() const
{
    return FanSpeedPercent;
}

int FDeviceController::GetExhaustLight1Channel() const { return ExhaustLight1Channel; }
int FDeviceController::GetExhaustLight2Channel() const { return ExhaustLight2Channel; }
int FDeviceController::GetPassiveLightsChannel() const { return PassiveLightsChannel; }
int FDeviceController::GetTailLightsChannel() const { return TailLightsChannel; }
int FDeviceController::GetLeftTurnLightsChannel() const { return LeftTurnLightsChannel; }
int FDeviceController::GetRightTurnLightsChannel() const { return RightTurnLightsChannel; }
int FDeviceController::GetHeadlightServoChannel() const { return HeadlightServoChannel; }
int FDeviceController::GetServoClosedPulseUs() const { return ServoClosedPulseUs; }
int FDeviceController::GetServoOpenPulseUs() const { return ServoOpenPulseUs; }
bool FDeviceController::IsServoZeroed() const { return bServoZeroed; }

int FDeviceController::GetChannelRole(int aChannel) const
{
    if (aChannel < 0 || aChannel > 15)
    {
        return 0;
    }

    return ChannelRoles.value(aChannel, 0);
}

bool FDeviceController::AreSettingsDirty() const
{
    return bSettingsDirty;
}

bool FDeviceController::IsSettingsUploadPending() const
{
    return bSettingsUploadPending;
}

int FDeviceController::GetSteeringPulseUs() const
{
    return SteeringPulseUs;
}

int FDeviceController::GetSteeringPercent() const
{
    return SteeringPercent;
}

int FDeviceController::GetThrottlePulseUs() const
{
    return ThrottlePulseUs;
}

int FDeviceController::GetThrottlePercent() const
{
    return ThrottlePercent;
}

double FDeviceController::GetForwardAccelerationG() const
{
    return ForwardAccelerationG;
}

double FDeviceController::GetFilteredForwardAccelerationG() const
{
    return FilteredForwardAccelerationG;
}

double FDeviceController::GetSideAccelerationG() const
{
    return SideAccelerationG;
}

double FDeviceController::GetVerticalAccelerationG() const
{
    return VerticalAccelerationG;
}

double FDeviceController::GetGyroscopeXDps() const
{
    return GyroscopeXDps;
}

double FDeviceController::GetGyroscopeYDps() const
{
    return GyroscopeYDps;
}

double FDeviceController::GetGyroscopeZDps() const
{
    return GyroscopeZDps;
}

const QString& FDeviceController::GetTurnDirection() const
{
    return TurnDirection;
}

bool FDeviceController::IsBrakeActive() const
{
    return bBrakeActive;
}

bool FDeviceController::IsExhaustPulseActive() const
{
    return bExhaustPulseActive;
}

bool FDeviceController::HasSteeringSignal() const
{
    return bSteeringSignalPresent;
}

bool FDeviceController::HasThrottleSignal() const
{
    return bThrottleSignalPresent;
}

bool FDeviceController::IsPcaConnected() const
{
    return bPcaConnected;
}

int FDeviceController::GetPcaAddress() const
{
    return PcaAddress;
}

int FDeviceController::GetPcaMode1() const
{
    return PcaMode1;
}

bool FDeviceController::IsAccelerometerConnected() const
{
    return bAccelerometerConnected;
}

bool FDeviceController::IsAccelerometerCalibrated() const
{
    return bAccelerometerCalibrated;
}

int FDeviceController::GetAccelerometerAddress() const
{
    return AccelerometerAddress;
}

int FDeviceController::GetAccelerometerWhoAmI() const
{
    return AccelerometerWhoAmI;
}

bool FDeviceController::AreDiagnosticsPending() const
{
    return bDiagnosticsPending;
}

const QString& FDeviceController::GetDiagnosticsSummary() const
{
    return DiagnosticsSummary;
}

QString FDeviceController::GetPcaStatusText() const
{
    return bPcaConnected
        ? QStringLiteral("OK at 0x%1 (MODE1 0x%2)")
            .arg(PcaAddress, 2, 16, QLatin1Char('0'))
            .arg(PcaMode1, 2, 16, QLatin1Char('0'))
        : QStringLiteral("NOT RESPONDING at 0x%1")
            .arg(PcaAddress, 2, 16, QLatin1Char('0'));
}

QString FDeviceController::GetAccelerometerStatusText() const
{
    return bAccelerometerConnected
        ? QStringLiteral("OK at 0x%1 (WHO_AM_I 0x%2)%3")
            .arg(AccelerometerAddress, 2, 16, QLatin1Char('0'))
            .arg(AccelerometerWhoAmI, 2, 16, QLatin1Char('0'))
            .arg(bAccelerometerCalibrated ? QStringLiteral(", calibrated") : QString())
        : QStringLiteral("NOT RESPONDING at 0x%1")
            .arg(AccelerometerAddress, 2, 16, QLatin1Char('0'));
}

QString FDeviceController::GetSteeringSignalStatus() const
{
    return bSteeringSignalPresent ? QStringLiteral("OK") : QStringLiteral("NO SIGNAL");
}

QString FDeviceController::GetThrottleSignalStatus() const
{
    return bThrottleSignalPresent ? QStringLiteral("OK") : QStringLiteral("NO SIGNAL");
}

QString FDeviceController::GetDeviceStatusSummary() const
{
    const QString Status =
        !bConnected
            ? ConnectionStatus
            : QStringLiteral("ESP32 OK  •  PCA %1  •  MPU %2  •  CH1 %3  •  CH2 %4")
                .arg(bPcaConnected ? QStringLiteral("OK") : QStringLiteral("OFFLINE"))
                .arg(bAccelerometerConnected ? QStringLiteral("OK") : QStringLiteral("OFFLINE"))
                .arg(GetSteeringSignalStatus())
                .arg(GetThrottleSignalStatus());

    return bSettingsDirty
        ? QStringLiteral("Unsaved changes  •  ") + Status
        : Status;
}

void FDeviceController::StartScan()
{
    if (
        bConnected ||
        bScanning
    )
    {
        return;
    }

    bManualDisconnectRequested = false;

    QBluetoothPermission Permission;
    Permission.setCommunicationModes(
        QBluetoothPermission::Access
    );

    switch (
        QCoreApplication::instance()->checkPermission(Permission)
    )
    {
    case Qt::PermissionStatus::Granted:
    {
        BeginBluetoothScan();
        break;
    }

    case Qt::PermissionStatus::Denied:
    {
        SetConnectionState(
            false,
            false,
            QStringLiteral("Bluetooth permission denied")
        );

        break;
    }

    case Qt::PermissionStatus::Undetermined:
    {
        QCoreApplication::instance()->requestPermission(
            Permission,
            this,
            [this](
                const QPermission& aPermissionRef
            )
            {
                if (
                    aPermissionRef.status() ==
                    Qt::PermissionStatus::Granted
                )
                {
                    BeginBluetoothScan();
                    return;
                }

                SetConnectionState(
                    false,
                    false,
                    QStringLiteral(
                        "Bluetooth permission denied"
                    )
                );
            }
        );

        break;
    }
    }
}

void FDeviceController::DisconnectFromDevice()
{
    bManualDisconnectRequested = true;
    ReconnectTimerPtr->stop();

    if (DiscoveryAgentPtr->isActive())
    {
        DiscoveryAgentPtr->stop();
    }

    BluetoothSocketPtr->disconnectFromService();
}

void FDeviceController::SetPassiveLightsEnabled(
    bool aEnabled
)
{
    bPassiveLightsEnabled = aEnabled;
    emit ConfigurationChanged();

    SendCommand(
        QByteArray("SET,PASSIVE_LIGHTS,") +
        (aEnabled ? "1" : "0")
    );
}

void FDeviceController::SetActiveLightsEnabled(
    bool aEnabled
)
{
    bActiveLightsEnabled = aEnabled;
    emit ConfigurationChanged();

    SendCommand(
        QByteArray("SET,ACTIVE_LIGHTS,") +
        (aEnabled ? "1" : "0")
    );
}

void FDeviceController::SetExhaustEnabled(
    bool aEnabled
)
{
    bExhaustEnabled = aEnabled;
    emit ConfigurationChanged();

    SendCommand(
        QByteArray("SET,EXHAUST_ENABLED,") +
        (aEnabled ? "1" : "0")
    );
}

void FDeviceController::SetHeadlightsOpen(
    bool aOpen
)
{
    bHeadlightsOpen = aOpen;
    emit ConfigurationChanged();

    SendCommand(
        QByteArray("SET,HEADLIGHT_OPEN,") +
        (aOpen ? "1" : "0")
    );
}

void FDeviceController::SetFansEnabled(
    bool aEnabled
)
{
    bFansEnabled = aEnabled;
    emit ConfigurationChanged();

    SendCommand(
        QByteArray("SET,FANS_ENABLED,") +
        (aEnabled ? "1" : "0")
    );
}

void FDeviceController::SetPendingAccelerometerEnabled(
    bool aEnabled
)
{
    if (bAccelerometerEnabled == aEnabled)
    {
        return;
    }

    bAccelerometerEnabled = aEnabled;
    MarkSettingsDirty();
}

void FDeviceController::SetPendingTriggerThresholdG(
    double aThresholdG
)
{
    const double ClampedThresholdG =
        qBound(0.01, aThresholdG, 0.50);

    if (
        qAbs(
            TriggerThresholdG -
            ClampedThresholdG
        ) < 0.0001
    )
    {
        return;
    }

    TriggerThresholdG = ClampedThresholdG;
    MarkSettingsDirty();
}

void FDeviceController::SetPendingAccelerometerForwardAxis(
    int aAxis
)
{
    const int ClampedAxis = qBound(0, aAxis, 2);

    if (AccelerometerForwardAxis == ClampedAxis)
    {
        return;
    }

    AccelerometerForwardAxis = ClampedAxis;
    MarkSettingsDirty();
}

void FDeviceController::SetPendingAccelerometerForwardInverted(
    bool aInverted
)
{
    if (bAccelerometerForwardInverted == aInverted)
    {
        return;
    }

    bAccelerometerForwardInverted = aInverted;
    MarkSettingsDirty();
}

void FDeviceController::SetPendingAccelerometerToleranceG(
    double aToleranceG
)
{
    const double ClampedToleranceG =
        qBound(0.0, aToleranceG, 0.20);

    if (
        qAbs(
            AccelerometerToleranceG -
            ClampedToleranceG
        ) < 0.0001
    )
    {
        return;
    }

    AccelerometerToleranceG = ClampedToleranceG;
    MarkSettingsDirty();
}

void FDeviceController::SetPendingActiveBrightnessPercent(
    int aBrightnessPercent
)
{
    const int ClampedBrightnessPercent =
        qBound(0, aBrightnessPercent, 100);

    if (
        ActiveBrightnessPercent ==
        ClampedBrightnessPercent
    )
    {
        return;
    }

    ActiveBrightnessPercent =
        ClampedBrightnessPercent;

    MarkSettingsDirty();
}

void FDeviceController::SetPendingDimBrightnessPercent(
    int aBrightnessPercent
)
{
    const int ClampedBrightnessPercent =
        qBound(0, aBrightnessPercent, 100);

    if (
        DimBrightnessPercent ==
        ClampedBrightnessPercent
    )
    {
        return;
    }

    DimBrightnessPercent =
        ClampedBrightnessPercent;

    MarkSettingsDirty();
}

void FDeviceController::SetPendingFanSpeedPercent(
    int aSpeedPercent
)
{
    const int ClampedSpeedPercent =
        qBound(0, aSpeedPercent, 100);

    if (FanSpeedPercent == ClampedSpeedPercent)
    {
        return;
    }

    FanSpeedPercent = ClampedSpeedPercent;
    MarkSettingsDirty();
}

void FDeviceController::SetExhaustLight1Channel(int aChannel)
{
    ExhaustLight1Channel = qBound(-1, aChannel, 15); MarkSettingsDirty();
}

void FDeviceController::SetExhaustLight2Channel(int aChannel)
{
    ExhaustLight2Channel = qBound(-1, aChannel, 15); MarkSettingsDirty();
}

void FDeviceController::SetPassiveLightsChannel(int aChannel)
{
    PassiveLightsChannel = qBound(-1, aChannel, 15); MarkSettingsDirty();
}

void FDeviceController::SetTailLightsChannel(int aChannel)
{
    TailLightsChannel = qBound(-1, aChannel, 15); MarkSettingsDirty();
}

void FDeviceController::SetLeftTurnLightsChannel(int aChannel)
{
    LeftTurnLightsChannel = qBound(-1, aChannel, 15); MarkSettingsDirty();
}

void FDeviceController::SetRightTurnLightsChannel(int aChannel)
{
    RightTurnLightsChannel = qBound(-1, aChannel, 15); MarkSettingsDirty();
}

void FDeviceController::SetHeadlightServoChannel(int aChannel)
{
    HeadlightServoChannel = qBound(-1, aChannel, 15); MarkSettingsDirty();
}

void FDeviceController::SetPendingServoClosedPulseUs(int aPulseUs)
{
    const int PulseUs = qBound(0, aPulseUs, 4095);
    if (ServoClosedPulseUs == PulseUs)
    {
        return;
    }

    ServoClosedPulseUs = PulseUs;
    bServoZeroed = false;
    MarkSettingsDirty();
}

void FDeviceController::SetPendingServoOpenPulseUs(int aPulseUs)
{
    const int PulseUs = qBound(0, aPulseUs, 4095);
    if (ServoOpenPulseUs == PulseUs)
    {
        return;
    }

    ServoOpenPulseUs = PulseUs;
    bServoZeroed = false;
    MarkSettingsDirty();
}

void FDeviceController::ZeroServo()
{
    bHeadlightsOpen = false;
    bServoZeroed = true;
    emit ConfigurationChanged();
    SendCommand(QByteArrayLiteral("SET,SERVO_ZERO,1"));
}

void FDeviceController::SetChannelRole(int aChannel, int aRole)
{
    if (aChannel < 0 || aChannel > 15 || aRole < 0 || aRole > 8)
    {
        return;
    }

    if (ChannelRoles.value(aChannel, 0) != aRole)
    {
        ChannelRoles[aChannel] = aRole;
        MarkSettingsDirty();
    }
}

void FDeviceController::LoadLocalSettings()
{
    QSettings LocalSettings;
    if (!LocalSettings.contains(QStringLiteral("settings/version")))
    {
        return;
    }

    bPassiveLightsEnabled = LocalSettings.value(
        QStringLiteral("settings/passiveLights"),
        bPassiveLightsEnabled
    ).toBool();
    bActiveLightsEnabled = LocalSettings.value(
        QStringLiteral("settings/activeLights"),
        bActiveLightsEnabled
    ).toBool();
    bExhaustEnabled = LocalSettings.value(
        QStringLiteral("settings/exhaustEnabled"),
        bExhaustEnabled
    ).toBool();
    bHeadlightsOpen = LocalSettings.value(
        QStringLiteral("settings/headlightsOpen"),
        bHeadlightsOpen
    ).toBool();
    bFansEnabled = LocalSettings.value(
        QStringLiteral("settings/fansEnabled"),
        bFansEnabled
    ).toBool();
    bAccelerometerEnabled = LocalSettings.value(
        QStringLiteral("settings/accelerometerEnabled"),
        bAccelerometerEnabled
    ).toBool();
    TriggerThresholdG = LocalSettings.value(
        QStringLiteral("settings/triggerThresholdG"),
        TriggerThresholdG
    ).toDouble();
    AccelerometerForwardAxis = LocalSettings.value(
        QStringLiteral("settings/accelerometerForwardAxis"),
        AccelerometerForwardAxis
    ).toInt();
    bAccelerometerForwardInverted = LocalSettings.value(
        QStringLiteral("settings/accelerometerForwardInverted"),
        bAccelerometerForwardInverted
    ).toBool();
    AccelerometerToleranceG = LocalSettings.value(
        QStringLiteral("settings/accelerometerToleranceG"),
        AccelerometerToleranceG
    ).toDouble();
    ActiveBrightnessPercent = LocalSettings.value(
        QStringLiteral("settings/activeBrightnessPercent"),
        ActiveBrightnessPercent
    ).toInt();
    DimBrightnessPercent = LocalSettings.value(
        QStringLiteral("settings/dimBrightnessPercent"),
        DimBrightnessPercent
    ).toInt();
    FanSpeedPercent = LocalSettings.value(
        QStringLiteral("settings/fanSpeedPercent"),
        FanSpeedPercent
    ).toInt();
    ServoClosedPulseUs = LocalSettings.value(
        QStringLiteral("settings/servoClosedPulseUs"),
        ServoClosedPulseUs
    ).toInt();
    ServoOpenPulseUs = LocalSettings.value(
        QStringLiteral("settings/servoOpenPulseUs"),
        ServoOpenPulseUs
    ).toInt();

    ExhaustLight1Channel = LocalSettings.value(
        QStringLiteral("settings/exhaustLight1Channel"),
        ExhaustLight1Channel
    ).toInt();
    ExhaustLight2Channel = LocalSettings.value(
        QStringLiteral("settings/exhaustLight2Channel"),
        ExhaustLight2Channel
    ).toInt();
    PassiveLightsChannel = LocalSettings.value(
        QStringLiteral("settings/passiveLightsChannel"),
        PassiveLightsChannel
    ).toInt();
    TailLightsChannel = LocalSettings.value(
        QStringLiteral("settings/tailLightsChannel"),
        TailLightsChannel
    ).toInt();
    LeftTurnLightsChannel = LocalSettings.value(
        QStringLiteral("settings/leftTurnLightsChannel"),
        LeftTurnLightsChannel
    ).toInt();
    RightTurnLightsChannel = LocalSettings.value(
        QStringLiteral("settings/rightTurnLightsChannel"),
        RightTurnLightsChannel
    ).toInt();
    HeadlightServoChannel = LocalSettings.value(
        QStringLiteral("settings/headlightServoChannel"),
        HeadlightServoChannel
    ).toInt();

    for (int Channel = 0; Channel < ChannelRoles.size(); ++Channel)
    {
        ChannelRoles[Channel] = LocalSettings.value(
            QStringLiteral("settings/channelRole%1").arg(Channel),
            ChannelRoles[Channel]
        ).toInt();
    }

    bLocalSettingsAvailable = true;
    bSettingsUploadPending = true;
}

bool FDeviceController::StoreLocalSettings() const
{
    QSettings LocalSettings;
    LocalSettings.setValue(QStringLiteral("settings/version"), 1);
    LocalSettings.setValue(
        QStringLiteral("settings/passiveLights"),
        bPassiveLightsEnabled
    );
    LocalSettings.setValue(
        QStringLiteral("settings/activeLights"),
        bActiveLightsEnabled
    );
    LocalSettings.setValue(
        QStringLiteral("settings/exhaustEnabled"),
        bExhaustEnabled
    );
    LocalSettings.setValue(
        QStringLiteral("settings/headlightsOpen"),
        bHeadlightsOpen
    );
    LocalSettings.setValue(
        QStringLiteral("settings/fansEnabled"),
        bFansEnabled
    );
    LocalSettings.setValue(
        QStringLiteral("settings/accelerometerEnabled"),
        bAccelerometerEnabled
    );
    LocalSettings.setValue(
        QStringLiteral("settings/triggerThresholdG"),
        TriggerThresholdG
    );
    LocalSettings.setValue(
        QStringLiteral("settings/accelerometerForwardAxis"),
        AccelerometerForwardAxis
    );
    LocalSettings.setValue(
        QStringLiteral("settings/accelerometerForwardInverted"),
        bAccelerometerForwardInverted
    );
    LocalSettings.setValue(
        QStringLiteral("settings/accelerometerToleranceG"),
        AccelerometerToleranceG
    );
    LocalSettings.setValue(
        QStringLiteral("settings/activeBrightnessPercent"),
        ActiveBrightnessPercent
    );
    LocalSettings.setValue(
        QStringLiteral("settings/dimBrightnessPercent"),
        DimBrightnessPercent
    );
    LocalSettings.setValue(
        QStringLiteral("settings/fanSpeedPercent"),
        FanSpeedPercent
    );
    LocalSettings.setValue(
        QStringLiteral("settings/servoClosedPulseUs"),
        ServoClosedPulseUs
    );
    LocalSettings.setValue(
        QStringLiteral("settings/servoOpenPulseUs"),
        ServoOpenPulseUs
    );
    LocalSettings.setValue(
        QStringLiteral("settings/exhaustLight1Channel"),
        ExhaustLight1Channel
    );
    LocalSettings.setValue(
        QStringLiteral("settings/exhaustLight2Channel"),
        ExhaustLight2Channel
    );
    LocalSettings.setValue(
        QStringLiteral("settings/passiveLightsChannel"),
        PassiveLightsChannel
    );
    LocalSettings.setValue(
        QStringLiteral("settings/tailLightsChannel"),
        TailLightsChannel
    );
    LocalSettings.setValue(
        QStringLiteral("settings/leftTurnLightsChannel"),
        LeftTurnLightsChannel
    );
    LocalSettings.setValue(
        QStringLiteral("settings/rightTurnLightsChannel"),
        RightTurnLightsChannel
    );
    LocalSettings.setValue(
        QStringLiteral("settings/headlightServoChannel"),
        HeadlightServoChannel
    );

    for (int Channel = 0; Channel < ChannelRoles.size(); ++Channel)
    {
        LocalSettings.setValue(
            QStringLiteral("settings/channelRole%1").arg(Channel),
            ChannelRoles[Channel]
        );
    }

    LocalSettings.sync();
    return LocalSettings.status() == QSettings::NoError;
}

void FDeviceController::SendSettingsToController()
{
    SendCommand(
        QByteArray("SET,PASSIVE_LIGHTS,") +
        (bPassiveLightsEnabled ? "1" : "0")
    );

    SendCommand(
        QByteArray("SET,ACTIVE_LIGHTS,") +
        (bActiveLightsEnabled ? "1" : "0")
    );

    SendCommand(
        QByteArray("SET,EXHAUST_ENABLED,") +
        (bExhaustEnabled ? "1" : "0")
    );

    SendCommand(
        QByteArray("SET,HEADLIGHT_OPEN,") +
        (bHeadlightsOpen ? "1" : "0")
    );

    SendCommand(
        QByteArray("SET,FANS_ENABLED,") +
        (bFansEnabled ? "1" : "0")
    );

    SendCommand(
        QByteArray("SET,ACCELEROMETER_ENABLED,") +
        (bAccelerometerEnabled ? "1" : "0")
    );

    SendCommand(
        QByteArray("SET,EXHAUST_TRIGGER_G,") +
        QByteArray::number(TriggerThresholdG, 'f', 3)
    );

    SendCommand(
        QByteArray("SET,ACCELEROMETER_FORWARD_AXIS,") +
        QByteArray::number(AccelerometerForwardAxis)
    );

    SendCommand(
        QByteArray("SET,ACCELEROMETER_FORWARD_INVERTED,") +
        (bAccelerometerForwardInverted ? "1" : "0")
    );

    SendCommand(
        QByteArray("SET,ACCELEROMETER_TOLERANCE_G,") +
        QByteArray::number(AccelerometerToleranceG, 'f', 3)
    );

    SendCommand(
        QByteArray("SET,ACTIVE_BRIGHTNESS,") +
        QByteArray::number(ActiveBrightnessPercent)
    );

    SendCommand(
        QByteArray("SET,DIM_BRIGHTNESS,") +
        QByteArray::number(DimBrightnessPercent)
    );

    SendCommand(
        QByteArray("SET,FAN_SPEED,") +
        QByteArray::number(FanSpeedPercent)
    );

    SendCommand(
        QByteArray("SET,SERVO_CLOSED_PULSE,") +
        QByteArray::number(ServoClosedPulseUs)
    );

    SendCommand(
        QByteArray("SET,SERVO_OPEN_PULSE,") +
        QByteArray::number(ServoOpenPulseUs)
    );

    const QList<QPair<const char*, int>> ChannelAssignments = {
        {"EXHAUST_LIGHT_1_CHANNEL", ExhaustLight1Channel},
        {"EXHAUST_LIGHT_2_CHANNEL", ExhaustLight2Channel},
        {"PASSIVE_LIGHTS_CHANNEL", PassiveLightsChannel},
        {"TAIL_LIGHTS_CHANNEL", TailLightsChannel},
        {"LEFT_TURN_LIGHTS_CHANNEL", LeftTurnLightsChannel},
        {"RIGHT_TURN_LIGHTS_CHANNEL", RightTurnLightsChannel},
        {"HEADLIGHT_SERVO_CHANNEL", HeadlightServoChannel}
    };

    for (const auto& Assignment : ChannelAssignments)
    {
        SendCommand(
            QByteArray("SET,") + Assignment.first + "," +
            QByteArray::number(
                Assignment.second < 0 ? 255 : Assignment.second
            )
        );
    }

    for (int Channel = 0; Channel < ChannelRoles.size(); ++Channel)
    {
        SendCommand(
            QByteArray("SET,CHANNEL_ROLE_") +
            QByteArray::number(Channel) + "," +
            QByteArray::number(ChannelRoles[Channel])
        );
    }

    SendCommand(
        QByteArrayLiteral("SAVE")
    );
}

void FDeviceController::SaveSettings()
{
    const bool bStoredLocally = StoreLocalSettings();
    if (!bStoredLocally)
    {
        emit SettingsSaveCompleted(false, false);
        return;
    }

    bLocalSettingsAvailable = true;
    bSettingsDirty = false;

    if (
        !bConnected ||
        BluetoothSocketPtr->state() !=
        QBluetoothSocket::SocketState::ConnectedState
    )
    {
        bSettingsUploadPending = true;
        emit ConfigurationChanged();
        emit SettingsSaveCompleted(false, true);
        return;
    }

    SendSettingsToController();
    bSettingsUploadPending = false;
    emit ConfigurationChanged();
    emit SettingsSaveCompleted(true, true);
}

void FDeviceController::ResetDefaults()
{
    bAccelerometerEnabled = true;
    TriggerThresholdG = 0.06;
    AccelerometerForwardAxis = 0;
    bAccelerometerForwardInverted = false;
    AccelerometerToleranceG = 0.02;
    ActiveBrightnessPercent = 100;
    DimBrightnessPercent = 25;
    FanSpeedPercent = 70;

    ExhaustLight1Channel = 0;
    ExhaustLight2Channel = 1;
    PassiveLightsChannel = 2;
    TailLightsChannel = 13;
    LeftTurnLightsChannel = 14;
    RightTurnLightsChannel = 15;
    HeadlightServoChannel = 12;

    ServoClosedPulseUs = 102;
    ServoOpenPulseUs = 512;
    bServoZeroed = false;
    ChannelRoles = {
        1, 2, 3, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 7, 4, 5, 6
    };

    MarkSettingsDirty();
}

void FDeviceController::TestExhaust()
{
    SendCommand(
        QByteArrayLiteral("TEST,EXHAUST")
    );
}

void FDeviceController::RunDiagnostics()
{
    if (!bConnected)
    {
        return;
    }

    bDiagnosticsPending = true;
    DiagnosticsSummary = QStringLiteral("Checking ESP32 hardware...");
    emit DiagnosticsChanged();
    SendCommand(QByteArrayLiteral("GET,DIAGNOSTICS"));
}

void FDeviceController::HandleDeviceDiscovered(
    const QBluetoothDeviceInfo& aDeviceInformationRef
)
{
    const QString FoundDeviceName =
        aDeviceInformationRef.name();

    if (
        FoundDeviceName.compare(
            TargetDeviceName,
            Qt::CaseInsensitive
        ) != 0
    )
    {
        return;
    }

    bTargetDeviceFound = true;
    DiscoveryAgentPtr->stop();

    ConnectToDevice(
        aDeviceInformationRef.address().toString(),
        FoundDeviceName
    );
}

void FDeviceController::HandleDiscoveryFinished()
{
    if (bTargetDeviceFound)
    {
        return;
    }

    SetConnectionState(
        false,
        false,
        QStringLiteral("Controller not found")
    );
}

void FDeviceController::HandleSocketConnected()
{
    ReceiveBuffer.clear();
    ReconnectTimerPtr->stop();
    bManualDisconnectRequested = false;

    SetConnectionState(
        true,
        false,
        QStringLiteral("ESP32 Connected")
    );

    SendCommand(
        QByteArrayLiteral("HELLO,1")
    );

    if (bSettingsUploadPending && !bSettingsDirty)
    {
        SendSettingsToController();
        bSettingsUploadPending = false;
        emit ConfigurationChanged();
        emit SettingsSaveCompleted(true, true);
    }
    else if (!bLocalSettingsAvailable && !bSettingsDirty)
    {
        SendCommand(
            QByteArrayLiteral("GET,CONFIG")
        );
    }

    RunDiagnostics();
}

void FDeviceController::HandleSocketDisconnected()
{
    SetConnectionState(
        false,
        false,
        QStringLiteral("Disconnected")
    );

    ScheduleReconnect();
}

void FDeviceController::HandleSocketReadyRead()
{
    ReceiveBuffer.append(
        BluetoothSocketPtr->readAll()
    );

    if (
        ReceiveBuffer.size() >
        MaximumReceiveBufferSize
    )
    {
        const qsizetype LastNewlineIndex =
            ReceiveBuffer.lastIndexOf('\n');

        ReceiveBuffer =
            LastNewlineIndex >= 0
                ? ReceiveBuffer.mid(
                    LastNewlineIndex + 1
                )
                : QByteArray();
    }

    qsizetype NewlineIndex = -1;

    while (
        (
            NewlineIndex =
            ReceiveBuffer.indexOf('\n')
        ) >= 0
    )
    {
        QByteArray Line =
            ReceiveBuffer.left(NewlineIndex).trimmed();

        ReceiveBuffer.remove(
            0,
            NewlineIndex + 1
        );

        if (!Line.isEmpty())
        {
            ParseLine(Line);
        }
    }
}

void FDeviceController::HandleSocketError()
{
    const QString ErrorText =
        BluetoothSocketPtr->errorString();

    SetConnectionState(
        false,
        false,
        ErrorText.isEmpty()
            ? QStringLiteral("Bluetooth connection failed")
            : ErrorText
    );

    ScheduleReconnect();
}

void FDeviceController::BeginBluetoothScan()
{
    bTargetDeviceFound = false;

    SetConnectionState(
        false,
        true,
        QStringLiteral("Searching for controller...")
    );

    DiscoveryAgentPtr->start(
        QBluetoothDeviceDiscoveryAgent::ClassicMethod
    );
}

void FDeviceController::ConnectToDevice(
    const QString& aAddress,
    const QString& aName
)
{
    DeviceName =
        aName.isEmpty()
            ? TargetDeviceName
            : aName;

    emit DeviceInformationChanged();

    SetConnectionState(
        false,
        false,
        QStringLiteral("Connecting...")
    );

    BluetoothSocketPtr->connectToService(
        QBluetoothAddress(aAddress),
        SerialPortServiceUuid
    );
}

void FDeviceController::SendCommand(
    const QByteArray& aCommandRef
)
{
    if (
        !bConnected ||
        BluetoothSocketPtr->state() !=
        QBluetoothSocket::SocketState::ConnectedState
    )
    {
        return;
    }

    QByteArray Command = aCommandRef.trimmed();
    Command.append('\n');

    BluetoothSocketPtr->write(Command);
}

void FDeviceController::ParseLine(
    const QByteArray& aLineRef
)
{
    const QList<QByteArray> Fields =
        aLineRef.split(',');

    if (Fields.isEmpty())
    {
        return;
    }

    if (Fields[0] == "HELLO")
    {
        ParseHello(Fields);
    }
    else if (Fields[0] == "CFG")
    {
        ParseConfiguration(Fields);
    }
    else if (Fields[0] == "TEL")
    {
        ParseTelemetry(Fields);
    }
    else if (Fields[0] == "DIAG")
    {
        ParseDiagnostics(Fields);
    }
}

void FDeviceController::ParseHello(
    const QList<QByteArray>& aFieldsRef
)
{
    if (aFieldsRef.size() < 4)
    {
        return;
    }

    DeviceName =
        QString::fromUtf8(aFieldsRef[2]);

    FirmwareVersion =
        QString::fromUtf8(aFieldsRef[3]);

    emit DeviceInformationChanged();
}

void FDeviceController::ParseConfiguration(
    const QList<QByteArray>& aFieldsRef
)
{
    if (aFieldsRef.size() < 11)
    {
        return;
    }

    bPassiveLightsEnabled =
        aFieldsRef[1].toInt() != 0;

    bActiveLightsEnabled =
        aFieldsRef[2].toInt() != 0;

    bExhaustEnabled =
        aFieldsRef[3].toInt() != 0;

    bHeadlightsOpen =
        aFieldsRef[4].toInt() != 0;

    bFansEnabled =
        aFieldsRef[5].toInt() != 0;

    bAccelerometerEnabled =
        aFieldsRef[6].toInt() != 0;

    TriggerThresholdG =
        aFieldsRef[7].toDouble();

    ActiveBrightnessPercent =
        aFieldsRef[8].toInt();

    DimBrightnessPercent =
        aFieldsRef[9].toInt();

    FanSpeedPercent =
        aFieldsRef[10].toInt();

    if (aFieldsRef.size() >= 18)
    {
        const auto ParseChannel = [](const QByteArray& aValue) {
            const int Channel = aValue.toInt();
            return Channel == 255 ? -1 : qBound(0, Channel, 15);
        };

        ExhaustLight1Channel = ParseChannel(aFieldsRef[11]);
        ExhaustLight2Channel = ParseChannel(aFieldsRef[12]);
        PassiveLightsChannel = ParseChannel(aFieldsRef[13]);
        TailLightsChannel = ParseChannel(aFieldsRef[14]);
        LeftTurnLightsChannel = ParseChannel(aFieldsRef[15]);
        RightTurnLightsChannel = ParseChannel(aFieldsRef[16]);
        HeadlightServoChannel = ParseChannel(aFieldsRef[17]);
    }

    if (aFieldsRef.size() >= 34)
    {
        for (int Channel = 0; Channel < 16; ++Channel)
        {
            ChannelRoles[Channel] = qBound(
                0,
                aFieldsRef[18 + Channel].toInt(),
                8
            );
        }
    }

    if (aFieldsRef.size() >= 36)
    {
        ServoClosedPulseUs = qBound(0, aFieldsRef[34].toInt(), 4095);
        ServoOpenPulseUs = qBound(0, aFieldsRef[35].toInt(), 4095);
        bServoZeroed = false;
    }

    if (aFieldsRef.size() >= 39)
    {
        AccelerometerForwardAxis = qBound(
            0,
            aFieldsRef[36].toInt(),
            2
        );
        bAccelerometerForwardInverted =
            aFieldsRef[37].toInt() != 0;
        AccelerometerToleranceG = qBound(
            0.0,
            aFieldsRef[38].toDouble(),
            0.20
        );
    }

    bSettingsDirty = false;
    emit ConfigurationChanged();
}

void FDeviceController::ParseTelemetry(
    const QList<QByteArray>& aFieldsRef
)
{
    if (aFieldsRef.size() < 18)
    {
        return;
    }

    bSteeringSignalPresent =
        aFieldsRef[2].toInt() != 0;

    SteeringPulseUs =
        bSteeringSignalPresent
            ? aFieldsRef[3].toInt()
            : 0;

    SteeringPercent =
        aFieldsRef[4].toInt();

    bThrottleSignalPresent =
        aFieldsRef[5].toInt() != 0;

    ThrottlePulseUs =
        bThrottleSignalPresent
            ? aFieldsRef[6].toInt()
            : 0;

    ThrottlePercent =
        aFieldsRef[7].toInt();

    ForwardAccelerationG =
        aFieldsRef[8].toDouble();

    FilteredForwardAccelerationG =
        aFieldsRef[9].toDouble();

    SideAccelerationG =
        aFieldsRef[10].toDouble();

    VerticalAccelerationG =
        aFieldsRef[11].toDouble();

    GyroscopeXDps =
        aFieldsRef[12].toDouble();

    GyroscopeYDps =
        aFieldsRef[13].toDouble();

    GyroscopeZDps =
        aFieldsRef[14].toDouble();

    TurnDirection =
        QString::fromUtf8(aFieldsRef[15]);

    bBrakeActive =
        aFieldsRef[16].toInt() != 0;

    bExhaustPulseActive =
        aFieldsRef[17].toInt() != 0;

    emit TelemetryChanged();
    emit DiagnosticsChanged();
}

void FDeviceController::ParseDiagnostics(
    const QList<QByteArray>& aFieldsRef
)
{
    if (aFieldsRef.size() < 10)
    {
        return;
    }

    bPcaConnected = aFieldsRef[1].toInt() != 0;
    PcaAddress = aFieldsRef[2].toInt();
    PcaMode1 = aFieldsRef[3].toInt();
    bAccelerometerConnected = aFieldsRef[4].toInt() != 0;
    bAccelerometerCalibrated = aFieldsRef[5].toInt() != 0;
    AccelerometerAddress = aFieldsRef[6].toInt();
    AccelerometerWhoAmI = aFieldsRef[7].toInt();
    bSteeringSignalPresent = aFieldsRef[8].toInt() != 0;
    bThrottleSignalPresent = aFieldsRef[9].toInt() != 0;
    bDiagnosticsPending = false;

    DiagnosticsSummary = QStringLiteral("Diagnostics complete: %1")
        .arg(bPcaConnected && bAccelerometerConnected
            ? QStringLiteral("ESP32 sees both I2C boards")
            : QStringLiteral("one or more boards are not responding"));

    emit DiagnosticsChanged();
    emit TelemetryChanged();
}

void FDeviceController::SetConnectionState(
    bool aConnected,
    bool aScanning,
    const QString& aStatusRef
)
{
    bConnected = aConnected;
    bScanning = aScanning;
    ConnectionStatus = aStatusRef;

    if (!aConnected)
    {
        bDiagnosticsPending = false;
        bPcaConnected = false;
        bAccelerometerConnected = false;
        bAccelerometerCalibrated = false;
        DiagnosticsSummary = QStringLiteral("Controller disconnected");
    }

    emit ConnectionChanged();
    emit DiagnosticsChanged();
}

void FDeviceController::MarkSettingsDirty()
{
    bSettingsDirty = true;
    emit ConfigurationChanged();
}

void FDeviceController::ScheduleReconnect()
{
    if (
        bManualDisconnectRequested ||
        ReconnectTimerPtr->isActive()
    )
    {
        return;
    }

    ReconnectTimerPtr->start();
}

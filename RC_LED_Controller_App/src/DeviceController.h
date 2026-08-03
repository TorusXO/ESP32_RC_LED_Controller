#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVector>

class QBluetoothDeviceDiscoveryAgent;
class QBluetoothDeviceInfo;
class QBluetoothSocket;
class QTimer;

class FDeviceController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        bool connected
        READ IsConnected
        NOTIFY ConnectionChanged
    )

    Q_PROPERTY(
        bool scanning
        READ IsScanning
        NOTIFY ConnectionChanged
    )

    Q_PROPERTY(
        QString connectionStatus
        READ GetConnectionStatus
        NOTIFY ConnectionChanged
    )

    Q_PROPERTY(
        QString deviceName
        READ GetDeviceName
        NOTIFY DeviceInformationChanged
    )

    Q_PROPERTY(
        QString firmwareVersion
        READ GetFirmwareVersion
        NOTIFY DeviceInformationChanged
    )

    Q_PROPERTY(
        bool passiveLightsEnabled
        READ ArePassiveLightsEnabled
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        bool activeLightsEnabled
        READ AreActiveLightsEnabled
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        bool exhaustEnabled
        READ IsExhaustEnabled
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        bool headlightsOpen
        READ AreHeadlightsOpen
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        bool fansEnabled
        READ AreFansEnabled
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        bool accelerometerEnabled
        READ IsAccelerometerEnabled
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        double triggerThresholdG
        READ GetTriggerThresholdG
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        int activeBrightnessPercent
        READ GetActiveBrightnessPercent
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        int dimBrightnessPercent
        READ GetDimBrightnessPercent
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(
        int fanSpeedPercent
        READ GetFanSpeedPercent
        NOTIFY ConfigurationChanged
    )

    Q_PROPERTY(int exhaustLight1Channel READ GetExhaustLight1Channel NOTIFY ConfigurationChanged)
    Q_PROPERTY(int exhaustLight2Channel READ GetExhaustLight2Channel NOTIFY ConfigurationChanged)
    Q_PROPERTY(int passiveLightsChannel READ GetPassiveLightsChannel NOTIFY ConfigurationChanged)
    Q_PROPERTY(int tailLightsChannel READ GetTailLightsChannel NOTIFY ConfigurationChanged)
    Q_PROPERTY(int leftTurnLightsChannel READ GetLeftTurnLightsChannel NOTIFY ConfigurationChanged)
    Q_PROPERTY(int rightTurnLightsChannel READ GetRightTurnLightsChannel NOTIFY ConfigurationChanged)
    Q_PROPERTY(int headlightServoChannel READ GetHeadlightServoChannel NOTIFY ConfigurationChanged)
    Q_PROPERTY(int servoClosedPulseUs READ GetServoClosedPulseUs NOTIFY ConfigurationChanged)
    Q_PROPERTY(int servoOpenPulseUs READ GetServoOpenPulseUs NOTIFY ConfigurationChanged)
    Q_PROPERTY(bool servoZeroed READ IsServoZeroed NOTIFY ConfigurationChanged)

    Q_PROPERTY(
        bool settingsDirty
        READ AreSettingsDirty
        NOTIFY ConfigurationChanged
    )


    Q_PROPERTY(
        int steeringPulseUs
        READ GetSteeringPulseUs
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        int steeringPercent
        READ GetSteeringPercent
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        int throttlePulseUs
        READ GetThrottlePulseUs
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        int throttlePercent
        READ GetThrottlePercent
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        double forwardAccelerationG
        READ GetForwardAccelerationG
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        double filteredForwardAccelerationG
        READ GetFilteredForwardAccelerationG
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        double sideAccelerationG
        READ GetSideAccelerationG
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        double verticalAccelerationG
        READ GetVerticalAccelerationG
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        double gyroscopeXDps
        READ GetGyroscopeXDps
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        double gyroscopeYDps
        READ GetGyroscopeYDps
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        double gyroscopeZDps
        READ GetGyroscopeZDps
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        QString turnDirection
        READ GetTurnDirection
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        bool brakeActive
        READ IsBrakeActive
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(
        bool exhaustPulseActive
        READ IsExhaustPulseActive
        NOTIFY TelemetryChanged
    )

    Q_PROPERTY(bool steeringSignalPresent READ HasSteeringSignal NOTIFY TelemetryChanged)
    Q_PROPERTY(bool throttleSignalPresent READ HasThrottleSignal NOTIFY TelemetryChanged)

    Q_PROPERTY(bool pcaConnected READ IsPcaConnected NOTIFY DiagnosticsChanged)
    Q_PROPERTY(int pcaAddress READ GetPcaAddress NOTIFY DiagnosticsChanged)
    Q_PROPERTY(int pcaMode1 READ GetPcaMode1 NOTIFY DiagnosticsChanged)
    Q_PROPERTY(bool accelerometerConnected READ IsAccelerometerConnected NOTIFY DiagnosticsChanged)
    Q_PROPERTY(bool accelerometerCalibrated READ IsAccelerometerCalibrated NOTIFY DiagnosticsChanged)
    Q_PROPERTY(int accelerometerAddress READ GetAccelerometerAddress NOTIFY DiagnosticsChanged)
    Q_PROPERTY(int accelerometerWhoAmI READ GetAccelerometerWhoAmI NOTIFY DiagnosticsChanged)
    Q_PROPERTY(bool diagnosticsPending READ AreDiagnosticsPending NOTIFY DiagnosticsChanged)
    Q_PROPERTY(QString diagnosticsSummary READ GetDiagnosticsSummary NOTIFY DiagnosticsChanged)
    Q_PROPERTY(QString pcaStatusText READ GetPcaStatusText NOTIFY DiagnosticsChanged)
    Q_PROPERTY(QString accelerometerStatusText READ GetAccelerometerStatusText NOTIFY DiagnosticsChanged)
    Q_PROPERTY(QString steeringSignalStatus READ GetSteeringSignalStatus NOTIFY TelemetryChanged)
    Q_PROPERTY(QString throttleSignalStatus READ GetThrottleSignalStatus NOTIFY TelemetryChanged)
    Q_PROPERTY(QString deviceStatusSummary READ GetDeviceStatusSummary NOTIFY DiagnosticsChanged)

public:
    explicit FDeviceController(
        QObject* aParentPtr = nullptr
    );

    ~FDeviceController() override;

    bool IsConnected() const;
    bool IsScanning() const;
    const QString& GetConnectionStatus() const;
    const QString& GetDeviceName() const;
    const QString& GetFirmwareVersion() const;

    bool ArePassiveLightsEnabled() const;
    bool AreActiveLightsEnabled() const;
    bool IsExhaustEnabled() const;
    bool AreHeadlightsOpen() const;
    bool AreFansEnabled() const;
    bool IsAccelerometerEnabled() const;
    double GetTriggerThresholdG() const;
    int GetActiveBrightnessPercent() const;
    int GetDimBrightnessPercent() const;
    int GetFanSpeedPercent() const;
    int GetExhaustLight1Channel() const;
    int GetExhaustLight2Channel() const;
    int GetPassiveLightsChannel() const;
    int GetTailLightsChannel() const;
    int GetLeftTurnLightsChannel() const;
    int GetRightTurnLightsChannel() const;
    int GetHeadlightServoChannel() const;
    int GetServoClosedPulseUs() const;
    int GetServoOpenPulseUs() const;
    bool IsServoZeroed() const;
    bool AreSettingsDirty() const;

    Q_INVOKABLE int GetChannelRole(int aChannel) const;
    Q_INVOKABLE void SetChannelRole(int aChannel, int aRole);

    int GetSteeringPulseUs() const;
    int GetSteeringPercent() const;
    int GetThrottlePulseUs() const;
    int GetThrottlePercent() const;
    double GetForwardAccelerationG() const;
    double GetFilteredForwardAccelerationG() const;
    double GetSideAccelerationG() const;
    double GetVerticalAccelerationG() const;
    double GetGyroscopeXDps() const;
    double GetGyroscopeYDps() const;
    double GetGyroscopeZDps() const;
    const QString& GetTurnDirection() const;
    bool IsBrakeActive() const;
    bool IsExhaustPulseActive() const;
    bool HasSteeringSignal() const;
    bool HasThrottleSignal() const;

    bool IsPcaConnected() const;
    int GetPcaAddress() const;
    int GetPcaMode1() const;
    bool IsAccelerometerConnected() const;
    bool IsAccelerometerCalibrated() const;
    int GetAccelerometerAddress() const;
    int GetAccelerometerWhoAmI() const;
    bool AreDiagnosticsPending() const;
    const QString& GetDiagnosticsSummary() const;
    QString GetPcaStatusText() const;
    QString GetAccelerometerStatusText() const;
    QString GetSteeringSignalStatus() const;
    QString GetThrottleSignalStatus() const;
    QString GetDeviceStatusSummary() const;

public slots:
    void StartScan();
    void DisconnectFromDevice();

    void SetPassiveLightsEnabled(bool aEnabled);
    void SetActiveLightsEnabled(bool aEnabled);
    void SetExhaustEnabled(bool aEnabled);
    void SetHeadlightsOpen(bool aOpen);
    void SetFansEnabled(bool aEnabled);

    void SetPendingAccelerometerEnabled(bool aEnabled);
    void SetPendingTriggerThresholdG(double aThresholdG);
    void SetPendingActiveBrightnessPercent(int aBrightnessPercent);
    void SetPendingDimBrightnessPercent(int aBrightnessPercent);
    void SetPendingFanSpeedPercent(int aSpeedPercent);
    void SetExhaustLight1Channel(int aChannel);
    void SetExhaustLight2Channel(int aChannel);
    void SetPassiveLightsChannel(int aChannel);
    void SetTailLightsChannel(int aChannel);
    void SetLeftTurnLightsChannel(int aChannel);
    void SetRightTurnLightsChannel(int aChannel);
    void SetHeadlightServoChannel(int aChannel);
    void SetPendingServoClosedPulseUs(int aPulseUs);
    void SetPendingServoOpenPulseUs(int aPulseUs);
    void ZeroServo();

    void SaveSettings();
    void ResetDefaults();
    void TestExhaust();
    void RunDiagnostics();

signals:
    void ConnectionChanged();
    void DeviceInformationChanged();
    void ConfigurationChanged();
    void TelemetryChanged();
    void DiagnosticsChanged();

private slots:
    void HandleDeviceDiscovered(
        const QBluetoothDeviceInfo& aDeviceInformationRef
    );

    void HandleDiscoveryFinished();
    void HandleSocketConnected();
    void HandleSocketDisconnected();
    void HandleSocketReadyRead();
    void HandleSocketError();

private:
    void BeginBluetoothScan();
    void ConnectToDevice(
        const QString& aAddress,
        const QString& aName
    );

    void SendCommand(
        const QByteArray& aCommandRef
    );

    void ParseLine(
        const QByteArray& aLineRef
    );

    void ParseHello(
        const QList<QByteArray>& aFieldsRef
    );

    void ParseConfiguration(
        const QList<QByteArray>& aFieldsRef
    );

    void ParseTelemetry(
        const QList<QByteArray>& aFieldsRef
    );

    void ParseDiagnostics(
        const QList<QByteArray>& aFieldsRef
    );


    void SetConnectionState(
        bool aConnected,
        bool aScanning,
        const QString& aStatusRef
    );

    void MarkSettingsDirty();
    void ScheduleReconnect();

private:
    QBluetoothDeviceDiscoveryAgent* DiscoveryAgentPtr = nullptr;
    QBluetoothSocket* BluetoothSocketPtr = nullptr;
    QTimer* ReconnectTimerPtr = nullptr;

    QByteArray ReceiveBuffer;

    bool bConnected = false;
    bool bScanning = false;
    bool bTargetDeviceFound = false;
    bool bManualDisconnectRequested = false;

    QString ConnectionStatus =
        QStringLiteral("Disconnected");

    QString DeviceName =
        QStringLiteral("RC-Light-Controller");

    QString FirmwareVersion =
        QStringLiteral("--");

    bool bPassiveLightsEnabled = true;
    bool bActiveLightsEnabled = true;
    bool bExhaustEnabled = true;
    bool bHeadlightsOpen = false;
    bool bFansEnabled = false;
    bool bAccelerometerEnabled = true;

    double TriggerThresholdG = 0.06;
    int ActiveBrightnessPercent = 100;
    int DimBrightnessPercent = 25;
    int FanSpeedPercent = 70;
    int ExhaustLight1Channel = 0;
    int ExhaustLight2Channel = 1;
    int PassiveLightsChannel = 2;
    int TailLightsChannel = 13;
    int LeftTurnLightsChannel = 14;
    int RightTurnLightsChannel = 15;
    int HeadlightServoChannel = 12;
    int ServoClosedPulseUs = 102;
    int ServoOpenPulseUs = 512;
    bool bServoZeroed = false;
    QVector<int> ChannelRoles = {
        1, 2, 3, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 7, 4, 5, 6
    };
    bool bSettingsDirty = false;


    int SteeringPulseUs = 0;
    int SteeringPercent = 0;
    int ThrottlePulseUs = 0;
    int ThrottlePercent = 0;

    double ForwardAccelerationG = 0.0;
    double FilteredForwardAccelerationG = 0.0;
    double SideAccelerationG = 0.0;
    double VerticalAccelerationG = 0.0;

    double GyroscopeXDps = 0.0;
    double GyroscopeYDps = 0.0;
    double GyroscopeZDps = 0.0;

    QString TurnDirection =
        QStringLiteral("NONE");

    bool bBrakeActive = false;
    bool bExhaustPulseActive = false;
    bool bSteeringSignalPresent = false;
    bool bThrottleSignalPresent = false;

    bool bPcaConnected = false;
    int PcaAddress = 0x40;
    int PcaMode1 = 0;
    bool bAccelerometerConnected = false;
    bool bAccelerometerCalibrated = false;
    int AccelerometerAddress = 0x68;
    int AccelerometerWhoAmI = 0;
    bool bDiagnosticsPending = false;
    QString DiagnosticsSummary = QStringLiteral("Run diagnostics to check the ESP32 hardware");
};

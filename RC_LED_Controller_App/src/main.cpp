#include "DeviceController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSettings>

static void migrateLegacySettings()
{
    QSettings CurrentSettings;
    const QString MigrationKey = QStringLiteral("migration/legacyOrganizationImported");
    if (CurrentSettings.value(MigrationKey, false).toBool())
        return;

    // Compatibility only: preserve settings saved under the former organization.
    // This cannot read another Android package's private storage.
    QSettings LegacySettings(QStringLiteral("SpiralKnightsInvoked"),
                             QCoreApplication::applicationName());
    for (const QString& Key : LegacySettings.allKeys()) {
        if (!CurrentSettings.contains(Key))
            CurrentSettings.setValue(Key, LegacySettings.value(Key));
    }
    CurrentSettings.sync();
    if (CurrentSettings.status() == QSettings::NoError) {
        CurrentSettings.setValue(MigrationKey, true);
        CurrentSettings.sync();
    }
}

int main(int aArgumentCount, char* aArgumentValuesPtr[])
{
    QGuiApplication Application(
        aArgumentCount,
        aArgumentValuesPtr
    );

    QCoreApplication::setOrganizationName(
        QStringLiteral("RCLEDController")
    );

    QCoreApplication::setApplicationName(
        QStringLiteral("RC LED Controller")
    );

    migrateLegacySettings();

    QQuickStyle::setStyle(
        QStringLiteral("Basic")
    );

    FDeviceController DeviceController;
    QQmlApplicationEngine Engine;

    Engine.rootContext()->setContextProperty(
        QStringLiteral("DeviceController"),
        &DeviceController
    );

    QObject::connect(
        &Engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &Application,
        []()
        {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection
    );

    Engine.loadFromModule(
        QStringLiteral("RCLEDController"),
        QStringLiteral("Main")
    );

    return Application.exec();
}

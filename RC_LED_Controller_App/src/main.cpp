#include "DeviceController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int aArgumentCount, char* aArgumentValuesPtr[])
{
    QGuiApplication Application(
        aArgumentCount,
        aArgumentValuesPtr
    );

    QCoreApplication::setOrganizationName(
        QStringLiteral("SpiralKnightsInvoked")
    );

    QCoreApplication::setApplicationName(
        QStringLiteral("RC LED Controller")
    );

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

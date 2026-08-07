cmake_minimum_required(VERSION 3.21)

project(
    RCLEDController
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(
    Qt6 6.5
    REQUIRED COMPONENTS
        Bluetooth
        Quick
        QuickControls2
)

qt_standard_project_setup(
    REQUIRES 6.5
)

qt_policy(SET QTP0004 NEW)
qt_policy(SET QTP0002 NEW)

qt_add_executable(
    appRCLEDController
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DeviceController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DeviceController.h
)

qt_add_qml_module(
    appRCLEDController
    URI RCLEDController
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/MainControllerPage.qml
        qml/SettingsPage.qml
        qml/ChannelAssignmentPage.qml
        qml/TelemetryPopup.qml
        qml/ServoCalibrationPopup.qml
        qml/components/ActionButton.qml
        qml/components/AppSwitch.qml
        qml/components/Card.qml
        qml/components/ControlRow.qml
        qml/components/InsetHorizontalScrollBar.qml
        qml/components/InsetVerticalScrollBar.qml
        qml/components/OptionSelector.qml
        qml/components/SegmentedControl.qml
        qml/components/Theme.js
        qml/components/ValueSlider.qml
    RESOURCES
        qml/components/qmldir
)

target_compile_features(
    appRCLEDController
    PRIVATE cxx_std_17
)

target_link_libraries(
    appRCLEDController
    PRIVATE
        Qt6::Bluetooth
        Qt6::Quick
        Qt6::QuickControls2
)

set_target_properties(
    appRCLEDController
    PROPERTIES
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        QT_ANDROID_PACKAGE_SOURCE_DIR
            "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

include(GNUInstallDirs)

install(
    TARGETS appRCLEDController
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

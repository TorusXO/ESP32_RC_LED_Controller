# RC LED Controller App

Qt 6 / Qt Quick application for the Retroid Pocket 5. The interface implements
the two 960 × 540 landscape frames from the `RC LED Controller UI` Figma file.

## Current features

- Bluetooth Classic RFCOMM connection to `RC-Light-Controller`.
- Android 12+ runtime Bluetooth permission request.
- Automatic reconnection after an unexpected disconnect.
- Main controls for passive lights, active lights and exhaust effects.
- Stored states for the future pop-up-headlight servo and cooling fans.
- Live CH1, CH2, accelerometer and gyroscope telemetry.
- Adjustable acceleration threshold, active brightness, dim brightness and fan
  speed.
- Explicit save to ESP32 NVS, reset-to-defaults and exhaust-test commands.

## Required Qt components

- Qt 6.5 or newer
- Qt Quick
- Qt Quick Controls
- Qt Bluetooth
- Qt for Android matching the installed Android SDK/NDK

## Build in Qt Creator

1. Open `CMakeLists.txt`.
2. Select a Qt 6 Android kit.
3. Set the target ABI supported by the Retroid Pocket 5. `arm64-v8a` is the
   normal choice.
4. Configure, build and deploy the `appRCLEDController` target.
5. Pair Android with `RC-Light-Controller` in Android Bluetooth settings before
   the first launch. The app then scans for that exact device name.
6. Allow Nearby Devices access when Android requests it.

The application is locked to landscape orientation by
`android/AndroidManifest.xml`.

## Interaction model

- Main-page switches send `SET` commands immediately, but do not write flash.
- Settings-page sliders are staged locally.
- `Save settings` sends the staged values and then one `SAVE` command.
- The ESP32 stores the complete configuration only when it receives `SAVE`.
- `Reset defaults` restores and stores the firmware defaults immediately.

Tapping the connection pill, footer, or `Live values` chip opens the complete
telemetry view.

The transport is Bluetooth Classic RFCOMM, not BLE. The ESP32 firmware uses
`BluetoothSerial`.

## Current hardware boundary

The pop-up-headlight and fan values are transported and saved, but the supplied
firmware does not assign them to a GPIO or PCA9685 channel. This prevents the
app from taking over an arbitrary output before the servo and fan driver wiring
are finalized.

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
- Channel setup supports separate red, green and blue roles for a common-ground
  RGB exhaust diode.

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

The Android application ID is `com.rcledcontroller.app`; its launcher activity
is `com.rcledcontroller.app.RetroidActivity`. The Java source lives under
`android/src/com/rcledcontroller/app/`. Qt uses `RCLEDController` as its
settings organization, with a one-time import of accessible legacy settings.

Changing the Android application ID creates a separate installed app, not an
update to the previous package. Its private local settings and presets do not
transfer automatically. Keep the old app until any needed setups are recovered.
Use a fresh Android build directory after the rename so generated manifests
and copied Java sources do not retain the former package.

### Inline name entry on Android

The board-name and saved-setup popups keep the text field inside the app while
the system keyboard is open. `InlineKeyboard.java` requests Android's
`IME_FLAG_NO_EXTRACT_UI` and `IME_FLAG_NO_FULLSCREEN` for Qt editors, preserving
the existing input type and Done action. The popups sit near the top of the
screen to leave room for the landscape keyboard; closing either popup hides
the keyboard and returns focus to its add button.

The small Java bridge accesses Qt 6.10's package-private `QtEditText` options,
because these flags are not exposed by QML input-method hints. It does not
modify the Qt installation. Recompile and recheck it when upgrading Qt.
Keyboard apps ultimately decide whether to honor Android's IME flags.

On-device regression check: open each name popup, type and delete text, use
Done to submit or Cancel to discard, then reopen it and hide/show the keyboard.
The keyboard should stay below the app's field, without a full-screen text
editor. Also check using the Retroid controls with a hardware keyboard attached.

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

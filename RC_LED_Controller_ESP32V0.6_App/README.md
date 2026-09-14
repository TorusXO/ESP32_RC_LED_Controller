# ESP32 RC LED Controller V0.6

This firmware adds the structured Bluetooth protocol used by the Qt app and the
first working RGB exhaust effect.

## Confirmed hardware mapping

| Function | Pin or channel |
| --- | --- |
| CH1 steering input | ESP32 GPIO35 |
| CH2 throttle input | ESP32 GPIO25 |
| I²C SDA | ESP32 GPIO21 |
| I²C SCL | ESP32 GPIO22 |
| MPU-compatible sensor | I²C `0x68`, IDs `0x68`, `0x70`, or `0x71` |
| PCA9685 | I²C `0x40` |
| Exhaust red channel | PCA9685 channel 0 |
| Exhaust green channel | PCA9685 channel 1 |
| Exhaust blue channel | PCA9685 channel 2 |
| Passive LEDs | PCA9685 channels 3–12 |
| Rear red LED | PCA9685 channel 13 |
| Left turn LED | PCA9685 channel 14 |
| Right turn LED | PCA9685 channel 15 |

GPIO35 is input-only and is correctly configured with `INPUT`, without an
internal pull-up.

## Exhaust effect

- Steady low throttle stays dark, even with accelerometer vibration.
- At 65% forward throttle, sustained load starts spaced 60–129 ms pops after
  250 ms. The high-load state releases below 55%; varied 420–849 ms gaps keep
  each burst distinct.
- A rise of at least 25 percentage points over a roughly 100 ms window triggers
  a short pop once throttle reaches 35%. Dropping at least 30 points from 65%
  or more produces a brief lift-off pop, including a release to neutral.
- Acceleration spikes require at least 40% forward throttle. The configurable
  threshold must reset below 60% before it can trigger again. A 240 ms minimum
  cooldown prevents continuous retriggering.
- Blue ignition fades into orange/red, with dark intervals between bursts.
- Reverse/braking input and lost CH2 signal cancel automatic exhaust output.
  Reconnecting the RC signal does not count as a throttle blip.
- These are throttle-based visual effects; no RPM or engine-load sensor is used.
- The app's flame preview lasts 180 ms. Red-only, green-only, and blue-only
  tests hold one color at full brightness for two seconds, suppress the other
  exhaust colors, and can be replaced or stopped early. Tests follow the board's
  saved channel roles, bypass exhaust enable/brightness, and do not save settings.
- PCA9685 outputs use totem-pole mode and non-inverted RGB PWM to drive the
  common-ground diode. Existing non-exhaust LED roles retain inverted PWM.

The default acceleration threshold is `0.06 g`. Adjust it from live driving
logs and save it from the app.

## Arduino IDE

Keep every source file in the `RC_LED_Controller_ESP32V0.6_App` folder and open
`RC_LED_Controller_ESP32V0.6_App.ino`.

Required libraries:

- ESP32 Arduino core, including `BluetoothSerial`, `Preferences` and `Wire`
- Adafruit PWM Servo Driver Library

Keep the car stationary during startup calibration.

## Saved configuration

The `SAVE` command writes the current values to the `rcled` Preferences
namespace. Continuous telemetry and slider movement do not write flash.

Headlight-open, fan-enabled and fan-speed values are already transported and
stored for the app. They do not drive hardware until output channels are
assigned.

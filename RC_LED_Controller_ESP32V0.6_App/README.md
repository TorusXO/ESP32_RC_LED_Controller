# ESP32 RC LED Controller V0.6

This firmware adds the structured Bluetooth protocol used by the Qt app and the
first working two-color exhaust effect.

## Confirmed hardware mapping

| Function | Pin or channel |
| --- | --- |
| CH1 steering input | ESP32 GPIO35 |
| CH2 throttle input | ESP32 GPIO25 |
| I²C SDA | ESP32 GPIO21 |
| I²C SCL | ESP32 GPIO22 |
| MPU-compatible sensor | I²C `0x68`, IDs `0x68`, `0x70`, or `0x71` |
| PCA9685 | I²C `0x40` |
| Yellow exhaust LED | PCA9685 channel 0 |
| Blue exhaust LED | PCA9685 channel 1 |
| Passive LEDs | PCA9685 channels 2–12 |
| Rear red LED | PCA9685 channel 13 |
| Left turn LED | PCA9685 channel 14 |
| Right turn LED | PCA9685 channel 15 |

GPIO35 is input-only and is correctly configured with `INPUT`, without an
internal pull-up.

## Exhaust effect

- A rapid forward-throttle rise creates a short flame immediately.
- Crossing the configured forward-acceleration threshold while forward
  throttle is active creates a longer flame.
- A 140 ms cooldown rejects repeated vibration triggers.
- Blue is the short inner ignition color.
- Yellow is the longer outer flame and fades through the pulse.
- Reverse throttle does not trigger the effect.
- The app's test command produces a 180 ms pulse without moving the car.

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

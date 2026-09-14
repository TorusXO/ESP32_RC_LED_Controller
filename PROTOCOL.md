# RC LED Controller Bluetooth Protocol

Transport: Bluetooth Classic Serial Port Profile (RFCOMM), UTF-8 text, one
newline-terminated record per message.

## ESP32 to app

```text
HELLO,1,RC_LED_CONTROLLER,0.6
```

```text
CFG,passive,active,exhaust,headlightOpen,fans,accelerometer,triggerG,activeBrightness,dimBrightness,fanSpeed,exhaust1Channel,exhaust2Channel,passiveChannel,tailChannel,leftTurnChannel,rightTurnChannel,servoChannel,channelRole0,...,channelRole15,servoClosedPulse,servoOpenPulse,forwardAxis,forwardInverted,accelerometerToleranceG,exhaust3Channel
```

```text
TEL,millis,ch1Valid,ch1Us,ch1Percent,ch2Valid,ch2Us,ch2Percent,forwardG,filteredForwardG,sideG,verticalG,gyroX,gyroY,gyroZ,turn,brake,exhaustPulse,reverse
```

```text
DIAG,pcaConnected,pcaAddress,pcaMode1,accelerometerConnected,accelerometerCalibrated,accelerometerAddress,accelerometerWhoAmI,ch1Signal,ch2Signal
```

`turn` is `NONE`, `LEFT`, or `RIGHT`. Boolean fields are `0` or `1`.

Other responses:

```text
ACK,key,value
ERR,error_code
```

## App to ESP32

```text
HELLO,1
GET,CONFIG
GET,DIAGNOSTICS

SET,PASSIVE_LIGHTS,0|1
SET,ACTIVE_LIGHTS,0|1
SET,EXHAUST_ENABLED,0|1
SET,HEADLIGHT_OPEN,0|1
SET,FANS_ENABLED,0|1
SET,ACCELEROMETER_ENABLED,0|1
SET,EXHAUST_TRIGGER_G,0.010..0.500
SET,ACCELEROMETER_FORWARD_AXIS,0..2
SET,ACCELEROMETER_FORWARD_INVERTED,0|1
SET,ACCELEROMETER_TOLERANCE_G,0.000..0.200
SET,ACTIVE_BRIGHTNESS,0..100
SET,DIM_BRIGHTNESS,0..100
SET,FAN_SPEED,0..100
SET,SERVO_CLOSED_PULSE,0..4095
SET,SERVO_OPEN_PULSE,0..4095
SET,SERVO_ZERO,1
SET,EXHAUST_LIGHT_1_CHANNEL,0..15
SET,EXHAUST_LIGHT_2_CHANNEL,0..15
SET,EXHAUST_LIGHT_3_CHANNEL,0..15
SET,PASSIVE_LIGHTS_CHANNEL,0..15
SET,TAIL_LIGHTS_CHANNEL,0..15
SET,LEFT_TURN_LIGHTS_CHANNEL,0..15
SET,RIGHT_TURN_LIGHTS_CHANNEL,0..15
SET,HEADLIGHT_SERVO_CHANNEL,0..15
SET,CHANNEL_ROLE_0,0..10
SET,CHANNEL_ROLE_1,0..10
SET,CHANNEL_ROLE_2,0..10
SET,CHANNEL_ROLE_3,0..10
SET,CHANNEL_ROLE_4,0..10
SET,CHANNEL_ROLE_5,0..10
SET,CHANNEL_ROLE_6,0..10
SET,CHANNEL_ROLE_7,0..10
SET,CHANNEL_ROLE_8,0..10
SET,CHANNEL_ROLE_9,0..10
SET,CHANNEL_ROLE_10,0..10
SET,CHANNEL_ROLE_11,0..10
SET,CHANNEL_ROLE_12,0..10
SET,CHANNEL_ROLE_13,0..10
SET,CHANNEL_ROLE_14,0..10
SET,CHANNEL_ROLE_15,0..10
SET,BOARD_NAME,text-without-commas

TEST,EXHAUST
TEST,EXHAUST,R
TEST,EXHAUST,G
TEST,EXHAUST,B
TEST,EXHAUST,OFF
TEST,SERVO,<channel>,<pcaPulse>
TEST,SERVO_CLEAR
SAVE
RESET
```

`SET` changes RAM immediately. `SAVE` writes the current complete configuration
to NVS. `RESET` restores and stores firmware defaults.

Channel role values are: `0` unused, `1` exhaust red, `2` exhaust green,
`3` passive lights, `4` tail lights, `5` left turning lights, `6` right turning
lights, `7` headlight servo, `8` cooling fans, `9` white reverse lights, and
`10` exhaust blue.
Any number of channels may share the same role.

`TEST,EXHAUST` previews a 180 ms flame. The optional `R`, `G`, or `B`
argument holds only the corresponding exhaust role at full brightness for two
seconds, with the other two exhaust roles off. A new test replaces the previous
one; `OFF` stops it immediately and allows normal driving effects to resume.
Tests work without an RC signal and bypass the exhaust enable/brightness settings.
They use the role assignments currently stored in ESP32 RAM, so upload edited
assignments before testing. Test commands do not change saved settings.
Invalid color arguments return `ERR,INVALID_EXHAUST_COLOR`. Accepted tests return
the existing `ACK,TEST_EXHAUST,1` response. Older firmware must be updated before
using per-color tests (it may treat these as a normal flame preview).

The `HELLO` response is:

```text
HELLO,protocol,transport_name,firmware_version,board_name,board_id
```

`board_name` is stored on the ESP32. `board_id` is the ESP32 hardware ID and
can be used by the app to match a vehicle profile.

`TEST,SERVO,<channel>,<pcaPulse>` is a temporary direct-output diagnostic. It
writes the requested raw PCA9685 pulse count (0..4095) to the selected physical
channel (0..15), bypassing channel roles and saved servo settings. The test
remains active until `TEST,SERVO_CLEAR` is received or the controller restarts.

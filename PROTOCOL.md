# RC LED Controller Bluetooth Protocol

Transport: Bluetooth Classic Serial Port Profile (RFCOMM), UTF-8 text, one
newline-terminated record per message.

## ESP32 to app

```text
HELLO,1,RC_LED_CONTROLLER,0.6
```

```text
CFG,passive,active,exhaust,headlightOpen,fans,accelerometer,triggerG,activeBrightness,dimBrightness,fanSpeed,exhaust1Channel,exhaust2Channel,passiveChannel,tailChannel,leftTurnChannel,rightTurnChannel,servoChannel,channelRole0,...,channelRole15,servoClosedPulse,servoOpenPulse,forwardAxis,forwardInverted,accelerometerToleranceG
```

```text
TEL,millis,ch1Valid,ch1Us,ch1Percent,ch2Valid,ch2Us,ch2Percent,forwardG,filteredForwardG,sideG,verticalG,gyroX,gyroY,gyroZ,turn,brake,exhaustPulse
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
SET,PASSIVE_LIGHTS_CHANNEL,0..15
SET,TAIL_LIGHTS_CHANNEL,0..15
SET,LEFT_TURN_LIGHTS_CHANNEL,0..15
SET,RIGHT_TURN_LIGHTS_CHANNEL,0..15
SET,HEADLIGHT_SERVO_CHANNEL,0..15
SET,CHANNEL_ROLE_0,0..8
SET,CHANNEL_ROLE_1,0..8
SET,CHANNEL_ROLE_2,0..8
SET,CHANNEL_ROLE_3,0..8
SET,CHANNEL_ROLE_4,0..8
SET,CHANNEL_ROLE_5,0..8
SET,CHANNEL_ROLE_6,0..8
SET,CHANNEL_ROLE_7,0..8
SET,CHANNEL_ROLE_8,0..8
SET,CHANNEL_ROLE_9,0..8
SET,CHANNEL_ROLE_10,0..8
SET,CHANNEL_ROLE_11,0..8
SET,CHANNEL_ROLE_12,0..8
SET,CHANNEL_ROLE_13,0..8
SET,CHANNEL_ROLE_14,0..8
SET,CHANNEL_ROLE_15,0..8

TEST,EXHAUST
SAVE
RESET
```

`SET` changes RAM immediately. `SAVE` writes the current complete configuration
to NVS. `RESET` restores and stores firmware defaults.

Channel role values are: `0` unused, `1` exhaust light 1, `2` exhaust light 2,
`3` passive lights, `4` tail lights, `5` left turning lights, `6` right turning
lights, `7` headlight servo, and `8` cooling fans. Any number of channels may
share the same role.

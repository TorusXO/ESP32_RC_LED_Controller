#pragma once
#include <cstdint>
inline uint16_t pinBrightness[16] = {};
inline bool pinInverted[16] = {};
inline bool pushPull = false;
class Adafruit_PWMServoDriver {
public:
    explicit Adafruit_PWMServoDriver(int) {}
    bool begin() { return true; }
    void setOutputMode(bool value) { pushPull = value; }
    void setPWMFreq(int) {}
    void setPWM(uint8_t, int, uint16_t) {}
    void setPin(uint8_t pin, uint16_t brightness, bool inverted) {
        pinBrightness[pin] = brightness;
        pinInverted[pin] = inverted;
    }
};

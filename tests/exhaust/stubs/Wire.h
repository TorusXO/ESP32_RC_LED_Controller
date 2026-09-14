#pragma once
#include <cstdint>
struct WireStub {
    void begin(int, int) {}
    void setClock(int) {}
    void beginTransmission(int) {}
    void write(int) {}
    int endTransmission(bool) { return 0; }
    int requestFrom(uint8_t, uint8_t) { return 1; }
    int read() { return 0; }
};
inline WireStub Wire;

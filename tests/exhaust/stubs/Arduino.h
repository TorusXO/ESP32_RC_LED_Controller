#pragma once
#include <algorithm>
#include <cstdint>
using std::max;
template<class T> T constrain(T v, T lo, T hi) { return std::clamp(v, lo, hi); }
inline uint32_t millis() { return 0; }
inline void delay(int) {}

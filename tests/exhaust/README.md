# Exhaust behavior regression test

Compiles the real LightingController.cpp against small hardware stubs. No board
is connected or written. From the repository root, with a C++17 compiler:

```sh
g++ -std=c++17 -Itests/exhaust/stubs -IRC_LED_Controller_ESP32V0.6_App tests/exhaust/exhaust_test.cpp RC_LED_Controller_ESP32V0.6_App/LightingController.cpp -o exhaust_test
./exhaust_test
```

Covers idle noise, throttle blips, spaced bursts at high throttle, lift-off,
accelerometer gating, reverse/signal loss, test color isolation, role mapping,
automatic timeout, replacement/stop, and millis rollover.

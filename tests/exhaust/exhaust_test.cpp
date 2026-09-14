#include "LightingController.h"
#include "Adafruit_PWMServoDriver.h"
#include <cassert>
#include <iostream>

struct Rig {
    FLightingController lights;
    FLightingInputState input;
    uint32_t time = 1000;
    Rig() { assert(lights.Begin()); input.bThrottleHasSignal = true; tick(0); }
    void tick(int throttle, float acceleration = 0, bool signal = true) {
        time += 20;
        input.ThrottlePercent = throttle;
        input.bThrottleHasSignal = signal;
        input.bAccelerometerHasSample = true;
        input.FilteredForwardAccelerationG = acceleration;
        lights.Update(input, time);
    }
    void dark() {
        assert(!lights.IsExhaustPulseActive());
        assert(pinBrightness[0] == 0 && pinBrightness[1] == 0 && pinBrightness[2] == 0);
    }
};

int main() {
    {
        Rig r;
        // Strong sensor vibration cannot light exhaust at idle/low throttle.
        for (int i = 0; i < 500; ++i) { r.tick(i % 20, i % 2 ? 0.4f : 0); r.dark(); }
        // Sharp blip fires once, then steady modest throttle settles to dark.
        r.tick(50); assert(r.lights.IsExhaustPulseActive());
        for (int i = 0; i < 100; ++i) r.tick(50);
        r.dark();
    }
    {
        Rig r;
        int bursts = 0, darkFrames = 0;
        bool previous = false;
        for (int i = 0; i < 300; ++i) {
            r.tick(85);
            bool active = r.lights.IsExhaustPulseActive();
            if (active && !previous) ++bursts;
            if (!active) ++darkFrames;
            previous = active;
        }
        assert(bursts >= 5 && bursts <= 18 && darkFrames > 150);
        r.tick(-50); r.dark();
        r.tick(85, 0.5f, false); r.dark();
        r.tick(85); r.dark(); // Reacquisition doesn't count as a throttle spike.
    }
    {
        Rig r;
        for (int i = 0; i < 15; ++i) r.tick(80);
        r.tick(0); assert(r.lights.IsExhaustPulseActive()); // Lift-off after load.
        for (int i = 0; i < 100; ++i) r.tick(0, 0.3f);
        r.dark();
    }
    {
        Rig r;
        for (int i = 0; i < 50; ++i) r.tick(45);
        r.tick(45, 0.2f); assert(r.lights.IsExhaustPulseActive());
        for (int i = 0; i < 50; ++i) r.tick(45, 0.2f);
        r.dark(); // Sustained acceleration must not repeatedly retrigger.
        FControllerConfiguration cfg;
        cfg.bAccelerometerExhaustEnabled = false;
        r.lights.ApplyConfiguration(cfg);
        r.tick(45, 0); r.tick(45, 0.3f); r.dark();
        cfg.bExhaustEnabled = false;
        r.lights.ApplyConfiguration(cfg);
        for (int i = 0; i < 100; ++i) { r.tick(100); r.dark(); }
    }
    {
        Rig r;
        FControllerConfiguration cfg;
        cfg.bExhaustEnabled = false;
        cfg.ActiveBrightnessPercent = 0;
        cfg.ChannelRoles[5] = 1; // Test follows all channels assigned the role.
        r.lights.ApplyConfiguration(cfg);
        const uint8_t roles[] = {1, 2, 10};
        for (int color = 0; color < 3; ++color) {
            r.lights.TriggerExhaustTest(r.time, roles[color]);
            for (int i = 0; i < 90; ++i) {
                r.tick(100);
                for (int pin = 0; pin < 3; ++pin) {
                    assert(pinBrightness[pin] == (pin == color ? 4095 : 0));
                    assert(!pinInverted[pin]);
                }
                assert(pinBrightness[5] == (color == 0 ? 4095 : 0));
            }
            for (int i = 0; i < 11; ++i) r.tick(0);
            r.dark();
        }
        r.lights.TriggerExhaustTest(r.time, 1); r.tick(0);
        r.lights.TriggerExhaustTest(r.time, 10); r.tick(0);
        assert(pinBrightness[0] == 0 && pinBrightness[2] == 4095);
        r.lights.TriggerExhaustTest(r.time, 255); r.tick(0); r.dark();
        r.lights.TriggerExhaustTest(r.time); r.tick(0);
        assert(r.lights.IsExhaustPulseActive());
        for (int i = 0; i < 10; ++i) r.tick(0);
        r.dark();
        // Unsigned elapsed time remains correct across millis() rollover.
        r.time = UINT32_MAX - 100;
        r.lights.TriggerExhaustTest(r.time, 2);
        for (int i = 0; i < 101; ++i) r.tick(0);
        r.dark();
    }
    {
        Rig r;
        r.lights.TriggerExhaustTest(r.time, 2);
        for (int i = 0; i < 90; ++i) {
            r.tick(i % 2 ? 100 : 40, i % 2 ? 0.4f : 0);
            assert(pinBrightness[0] == 0 && pinBrightness[1] == 4095 && pinBrightness[2] == 0);
        }
        r.tick(-50, 0, false); // Diagnostics still work without RC reception.
        assert(pinBrightness[1] == 4095);
        r.lights.TriggerExhaustTest(r.time, 255); r.tick(0); r.dark();
    }
    assert(pushPull);
    std::cout << "PASS: idle, blips, high throttle, lift-off, sensor gate, signal loss, RGB isolation, timeout, stop, rollover\n";
}

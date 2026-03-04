/*-----------------------------------------------------------------------
 * Granular Synthesis Ladder
 * NUCLEO-F767ZI (STM32F767ZI) — Control Layer Proofs
 *
 * Proof 1: Pot -> LEDs (zones)
 * Proof 1.5: Smoothing + hysteresis
 * Proof 2: USER button cycles which parameter the pot controls
 *
 * Notes discovered on this board:
 *  - USER button works on PC_13
 *  - Internal pull should be PullNone (PullUp broke it in tests)
 *-----------------------------------------------------------------------*/

#include "mbed.h"
#include <cstdio>
#include <cstring>
#include <cmath>

// LED outputs
DigitalOut led1(LED1), led2(LED2), led3(LED3);

// Pot input (Arduino A0 mapped by this target)
AnalogIn pot(A0);

// On-board USER button (known-good on this setup)

DigitalIn userBtn(PC_13);
// Optional debug: serial over ST-LINK Virtual COM
// On NUCLEO-144 (MB1137), ST-LINK VCP is wired to PD8 (TX), PD9 (RX)
BufferedSerial pc(PD_8, PD_9, 115200);

static void show_zone(int zone) {
    led1 = (zone == 0);
    led2 = (zone == 1);
    led3 = (zone == 2);
}

static void show_param(int paramIndex) {
    // 0 -> LED1, 1 -> LED2, 2 -> LED3
    led1 = (paramIndex == 0);
    led2 = (paramIndex == 1);
    led3 = (paramIndex == 2);
}

static int zone_from_value(float v, float t1, float t2) {
    if (v < t1) return 0;
    if (v < t2) return 1;
    return 2;
}

int main() {
    // Let ADC settle a moment after boot
    ThisThread::sleep_for(100ms);

    // Button configuration: PullNone (per our board tests)
    userBtn.mode(PullNone);

    // Serial banner
    {
        const char* banner = "Granular Ladder: Proof 2 running (A0 + PC13)\r\n";
        pc.write(banner, strlen(banner));
    }

    Timer logTimer;
    logTimer.start();

    Timer pickupBlink;
    pickupBlink.start();
    
    // Hysteresis thresholds
    constexpr float t1 = 0.33f;
    constexpr float t2 = 0.66f;
    constexpr float h  = 0.03f;   // tweak 0.02–0.06

    // Smoothing
    constexpr float alpha = 0.1f; // tweak 0.05–0.2
    constexpr float inv_alpha = 1.0f - alpha;

    // Three parameters (values normalized 0..1)
    float params[3] = {0.0f, 0.0f, 0.0f};
    int activeParam = 0;
    bool pickedUp[3] = {true, false, false};    // activeParam 0 starts in control.

    // Start smoothing state from current pot value
    float xs = pot.read();
    if (xs < 0.0f) xs = 0.0f;
    if (xs > 1.0f) xs = 1.0f;
    params[activeParam] = xs;

    // Zone state (for the ACTIVE parameter)
    int zone = zone_from_value(params[activeParam], t1, t2);

    // Button edge detect + debounce
    bool lastDown = false;
    Timer debounce;
    debounce.start();

    // Brief UI feedback when switching params
    bool showingParam = true;
    Timer paramFlash;
    paramFlash.start();

    while (true) {
        // --- Read pot and update active parameter ---
        const uint16_t raw = pot.read_u16();
        const uint16_t x1000 = (raw * 1000u) / 65535u; // 0..1000
        const float x = raw / 65535.0f;                // 0..1 (for your smoothing math)

        // Smooth
        xs = xs * inv_alpha + x * alpha;

        // Clamp
        if (xs < 0.0f) xs = 0.0f;
        if (xs > 1.0f) xs = 1.0f;

        // Store ONLY into the currently active parameter
        // params[activeParam] = xs;

        // Soft takeover / pickup: don't overwrite until the knob reaches the stored value.
        if (pickedUp[activeParam]) {
            params[activeParam] = xs;
        } else {
            if (fabsf(xs - params[activeParam]) < 0.03f) {
                pickedUp[activeParam] = true;
                pickupBlink.reset(); // stop the "not picked up" blink cleanly
            }
        }

        // --- Button handling: cycle activeParam ---
        // IMPORTANT: Depending on board wiring, pressed could read as 0 or 1.
        // We treat a "press" as a CHANGE into the opposite of the idle level.
        // We'll infer idle level on first iteration by sampling quickly.
        static bool idleKnown = false;
        static int idleLevel = 1;
        if (!idleKnown) {
            int sum = 0;
            for (int i = 0; i < 20; ++i) {
                sum += userBtn.read();
                ThisThread::sleep_for(2ms);
            }
            idleLevel = (sum >= 10) ? 1 : 0;
            idleKnown = true;
        }

        const bool down = (userBtn.read() != idleLevel);

        if (down && !lastDown) {
            if (debounce.elapsed_time() > 200ms) {
                debounce.reset();

                activeParam = (activeParam + 1) % 3;
                pickedUp[activeParam] = false;
                pickupBlink.reset(); // restart pickup blink for the newly selected param

                // Re-seed smoothing state from current pot read, so switching feels stable.
                // (We could also do pickup behavior later.)
                // xs = pot.read();
                // if (xs < 0.0f) xs = 0.0f;
                // if (xs > 1.0f) xs = 1.0f;
                // params[activeParam] = xs;

                // Reset zone based on the new active param value
                zone = zone_from_value(params[activeParam], t1, t2);

                // Flash which param we switched to
                showingParam = true;
                paramFlash.reset();
            }
        }
        lastDown = down;

        // --- Zone update for active parameter (using hysteresis) ---
        const float v = params[activeParam];

        if (zone == 0 && v > t1 + h) {
            zone = 1;
        } else if (zone == 1 && v < t1 - h) {
            zone = 0;
        } else if (zone == 1 && v > t2 + h) {
            zone = 2;
        } else if (zone == 2 && v < t2 - h) {
            zone = 1;
        }

        // --- Display ---
        if (showingParam) {
            // Two-flash pattern so you can always SEE a param change,
            // even if the param LED matches the current zone LED.
            //
            // Timeline (ms): ON 0-120, OFF 120-240, ON 240-360, OFF 360-480, then done
            const auto t = paramFlash.elapsed_time();
            const bool on = (t < 120ms) || (t >= 240ms && t < 360ms);

            if (on) {
                show_param(activeParam);
            } else {
                // Off phase: all LEDs off
                led1 = 0; led2 = 0; led3 = 0;
            }

            if (t > 480ms) {
                showingParam = false;
            }
        } else {
            // Not showing param indicator: show pickup feedback OR zone.
            if (!pickedUp[activeParam]) {
                // Slow blink all LEDs: ON 200ms, OFF 300ms (500ms period).
                const auto tb = pickupBlink.elapsed_time();
                const bool on = (tb % 500ms) < 200ms;
                led1 = led2 = led3 = on;
            } else {
                show_zone(zone);
            }
        }

        // --- Serial debug (5 Hz) ---
        if (logTimer.elapsed_time() > 500ms) {
            logTimer.reset();

            char buf[160];
            const int n = snprintf(
                buf, sizeof(buf),
                "p=%d  raw=%5u  x1000=%4u  zone=%d  rawBtn=%d\r\n",
                activeParam, raw, x1000, zone, userBtn.read()
            );
            if (n > 0) {
                pc.write(buf, n);
            }
        }

        // Control-rate loop
        ThisThread::sleep_for(20ms);
    }
}
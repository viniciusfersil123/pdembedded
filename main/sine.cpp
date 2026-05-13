/* Minimal 440 Hz sine wave example using UDA1334A driver.
   Keeps original `pdembedded.cpp` commented as requested.
*/
#include <cmath>
#include <cstdint>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "drivers/uda1334a/uda1334a.h"

extern "C" void app_main(void)
{
    const uint32_t sample_rate = 48000;
    const float freq = 440.0f;
    const int frames = 256;

    UDA1334A dac;
    dac.begin(sample_rate);

    std::vector<int16_t> buf(frames * 2);

    const float two_pi = 2.0f * M_PI;
    float phase = 0.0f;
    const float phase_inc = two_pi * freq / static_cast<float>(sample_rate);
    const float amp = 0.25f; // safe amplitude

    while (true)
    {
        for (int i = 0; i < frames; ++i)
        {
            float s = sinf(phase) * amp;
            int16_t v = static_cast<int16_t>(s * INT16_MAX);
            buf[2 * i] = v;     // left
            buf[2 * i + 1] = v; // right

            phase += phase_inc;
            if (phase >= two_pi) phase -= two_pi;
        }

        dac.write(buf.data(), static_cast<size_t>(frames * 2));
    }
}

/* Minimal sine wave example using UDA1334A driver.
   Potentiometer on GPIO35 (ADC1_CH7) controls frequency.
   Original heavy-based main is kept commented below.
*/
#include <cmath>
#include <cstdint>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "drivers/uda1334a/uda1334a.h"
#include "driver/adc.h"
#include "per/adc.h"

extern "C" void app_main(void)
{
    const uint32_t sample_rate = 48000;
    const int frames = 256;

    // Frequency mapping from pot: 50 Hz .. 2000 Hz
    const float min_freq = 50.0f;
    const float max_freq = 2000.0f;

    // ADC setup using per/adc classes: GPIO35 is ADC1_CHANNEL_7
    AdcChannelConfig cfg;
    cfg.InitEsp(1, ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
    AdcHandle adc;
    // increase oversampling to reduce noise and enable background polling
    adc.Init(&cfg, 1, AdcHandle::OVS_64);
    adc.SetPollInterval(10); // poll every 10 ms
    adc.Start();

    UDA1334A dac;
    dac.begin(sample_rate);

    std::vector<int16_t> buf(frames * 2);

    const float two_pi = 2.0f * M_PI;
    float phase = 0.0f;
    const float amp = 0.25f; // safe amplitude

    while (true)
    {
        uint16_t raw16 = adc.Get(0); // returns 16-bit scaled value (0..65535)
        float t = static_cast<float>(raw16) / 65535.0f;
        float freq = min_freq + t * (max_freq - min_freq);
        float phase_inc = two_pi * freq / static_cast<float>(sample_rate);

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

/* #include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "drivers/uda1334a/uda1334a.h"
#include "../.main/output/c/HeavyContextInterface.hpp"
#include "../.main/output/c/Heavy_heavy.h"

// Include auto-generated I2S configuration
#include "../build/config/i2s_config.h"

static UDA1334A dac;
static float out_ch0[I2S_BLOCK_SIZE];
static float out_ch1[I2S_BLOCK_SIZE];
static float* out_channels[2] = {out_ch0, out_ch1};

extern "C" void app_main(void)
{
    HeavyContextInterface *hv_ctx = hv_heavy_new(48000.0);
    const double sample_rate = hv_ctx->getSampleRate();

    // Initialize DAC with pins from I2S configuration
    init_i2s_config(dac, sample_rate);

    while (true)
    {
        hv_ctx->process(nullptr, out_channels, I2S_BLOCK_SIZE);
        for (int i = 0; i < I2S_BLOCK_SIZE; i++)
        {
            const float volume = 0.1f;
            dac.write(out_ch0[i] * volume, out_ch1[i] * volume);
        }
    }
}

 */
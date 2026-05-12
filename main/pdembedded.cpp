#include "freertos/FreeRTOS.h"
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

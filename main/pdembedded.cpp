#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "drivers/uda1334a/uda1334a.h"
#include "c/HeavyContextInterface.hpp"
#include "c/Heavy_heavy.h"

static UDA1334A dac;
static constexpr int SR = 48000;
static constexpr int BLOCK_SIZE = 64;
static float out_ch0[BLOCK_SIZE];
static float out_ch1[BLOCK_SIZE];
static float* out_channels[2] = {out_ch0, out_ch1};

extern "C" void app_main(void)
{
    dac.begin(SR, GPIO_NUM_27, GPIO_NUM_26, GPIO_NUM_25);
    HeavyContextInterface *hv_ctx = hv_heavy_new(SR);

    while (true)
    {
        hv_ctx->process(nullptr, out_channels, BLOCK_SIZE);
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            const float volume = 0.1f;
            dac.write(out_ch0[i] * volume, out_ch1[i] * volume);
        }
    }
}

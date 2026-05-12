#ifndef UDA1334A_H
#define UDA1334A_H

#include <stddef.h>
#include <stdint.h>

#include "driver/i2s_std.h"
#include "esp_err.h"
#include "driver/gpio.h"

#define UDA1334A_TAG "UDA1334A"

class UDA1334A
{
public:
    UDA1334A();
    ~UDA1334A();

    esp_err_t begin(uint32_t sample_rate = 48000,
                    gpio_num_t bclk_pin = GPIO_NUM_27,
                    gpio_num_t ws_pin = GPIO_NUM_26,
                    gpio_num_t dout_pin = GPIO_NUM_25);

    esp_err_t end();

    esp_err_t setSampleRate(uint32_t sample_rate);
    esp_err_t write(const int16_t *samples, size_t sample_count);
    esp_err_t write(float left_channel, float right_channel);

    bool isInitialized() const;

private:
    esp_err_t initChannel(uint32_t sample_rate,
                          gpio_num_t bclk_pin,
                          gpio_num_t ws_pin,
                          gpio_num_t dout_pin);
    void releaseChannel();

    i2s_chan_handle_t tx_handle_;
    uint32_t sample_rate_;
    gpio_num_t bclk_pin_;
    gpio_num_t ws_pin_;
    gpio_num_t dout_pin_;
    bool initialized_;
};

#endif

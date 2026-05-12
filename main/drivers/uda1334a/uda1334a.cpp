#include "uda1334a.h"

#include <algorithm>
#include <cstdint>
#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"

UDA1334A::UDA1334A()
    : tx_handle_(nullptr),
      sample_rate_(48000),
      bclk_pin_(GPIO_NUM_27),
      ws_pin_(GPIO_NUM_26),
      dout_pin_(GPIO_NUM_25),
      initialized_(false)
{
}

UDA1334A::~UDA1334A()
{
    end();
}

esp_err_t UDA1334A::begin(uint32_t sample_rate,
                          gpio_num_t bclk_pin,
                          gpio_num_t ws_pin,
                          gpio_num_t dout_pin)
{
    sample_rate_ = sample_rate;
    bclk_pin_ = bclk_pin;
    ws_pin_ = ws_pin;
    dout_pin_ = dout_pin;

    if (initialized_) {
        return ESP_OK;
    }

    return initChannel(sample_rate_, bclk_pin_, ws_pin_, dout_pin_);
}

esp_err_t UDA1334A::end()
{
    if (!initialized_) {
        return ESP_OK;
    }

    esp_err_t ret = i2s_channel_disable(tx_handle_);
    ret = (ret == ESP_OK) ? i2s_del_channel(tx_handle_) : ret;
    tx_handle_ = nullptr;
    initialized_ = false;
    return ret;
}

esp_err_t UDA1334A::setSampleRate(uint32_t sample_rate)
{
    if (sample_rate == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (sample_rate == sample_rate_ && initialized_) {
        return ESP_OK;
    }

    esp_err_t ret = end();
    if (ret != ESP_OK) {
        return ret;
    }

    sample_rate_ = sample_rate;
    return initChannel(sample_rate_, bclk_pin_, ws_pin_, dout_pin_);
}

esp_err_t UDA1334A::write(const int16_t *samples, size_t sample_count)
{
    if (!initialized_ || samples == nullptr || sample_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t bytes_written = 0;
    return i2s_channel_write(tx_handle_, samples, sample_count * sizeof(int16_t), &bytes_written, portMAX_DELAY);
}

esp_err_t UDA1334A::write(float left_channel, float right_channel)
{
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }

    int16_t frame[2];
    frame[0] = static_cast<int16_t>(std::clamp(left_channel, -1.0f, 1.0f) * INT16_MAX);
    frame[1] = static_cast<int16_t>(std::clamp(right_channel, -1.0f, 1.0f) * INT16_MAX);
    return write(frame, 2);
}

bool UDA1334A::isInitialized() const
{
    return initialized_;
}

esp_err_t UDA1334A::initChannel(uint32_t sample_rate,
                                gpio_num_t bclk_pin,
                                gpio_num_t ws_pin,
                                gpio_num_t dout_pin)
{
    if (sample_rate == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    esp_err_t ret = i2s_new_channel(&chan_cfg, &tx_handle_, nullptr);
    if (ret != ESP_OK) {
        tx_handle_ = nullptr;
        return ret;
    }

    i2s_std_config_t std_cfg = {};
    std_cfg.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate);
    std_cfg.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    std_cfg.gpio_cfg.mclk = I2S_GPIO_UNUSED;
    std_cfg.gpio_cfg.bclk = bclk_pin;
    std_cfg.gpio_cfg.ws = ws_pin;
    std_cfg.gpio_cfg.dout = dout_pin;
    std_cfg.gpio_cfg.din = I2S_GPIO_UNUSED;
    std_cfg.gpio_cfg.invert_flags.mclk_inv = false;
    std_cfg.gpio_cfg.invert_flags.bclk_inv = false;
    std_cfg.gpio_cfg.invert_flags.ws_inv = false;

    ret = i2s_channel_init_std_mode(tx_handle_, &std_cfg);
    if (ret != ESP_OK) {
        releaseChannel();
        return ret;
    }

    ret = i2s_channel_enable(tx_handle_);
    if (ret != ESP_OK) {
        releaseChannel();
        return ret;
    }

    initialized_ = true;
    ESP_LOGI(UDA1334A_TAG, "Initialized UDA1334A at %lu Hz", static_cast<unsigned long>(sample_rate));
    return ESP_OK;
}

void UDA1334A::releaseChannel()
{
    if (tx_handle_ != nullptr) {
        i2s_del_channel(tx_handle_);
        tx_handle_ = nullptr;
    }
    initialized_ = false;
}

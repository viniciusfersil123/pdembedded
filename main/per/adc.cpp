#include "per/adc.h"
// Enable ESP32 ADC API usage when building for ESP-IDF
#if defined(ESP_PLATFORM) || defined(__ESP32__)
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include <string.h>
#endif

static void Error_Handler()
{
    for(;;) {}
}

// For ESP32 we use the ADC1 legacy functions for a simple, portable
// implementation. If not building for ESP32 this file provides stubs.

static const float PD_ADC_MAX_RESOLUTION_F = 4095.0f; // 12-bit default

struct dsy_adc
{
    AdcChannelConfig pin_cfg[PD_ADC_MAX_CHANNELS];
    uint8_t           channels;
    uint16_t          last_read[PD_ADC_MAX_CHANNELS];
};

static dsy_adc adc;

#if defined(ESP_PLATFORM) || defined(__ESP32__)
static adc_oneshot_unit_handle_t g_adc1_handle = NULL;
#endif

#if defined(ESP_PLATFORM) || defined(__ESP32__)
static void adc_poll_task(void* pv)
{
    AdcHandle* h = (AdcHandle*)pv;
    for(;;)
    {
        for(uint8_t i = 0; i < adc.channels; i++)
        {
            if(adc.pin_cfg[i].adc_unit_ == 1 && adc.pin_cfg[i].adc_channel_ >= 0)
            {
                int samples = 1;
                switch(h->GetOverSampling())
                {
                    case AdcHandle::OVS_4: samples = 4; break;
                    case AdcHandle::OVS_8: samples = 8; break;
                    case AdcHandle::OVS_16: samples = 16; break;
                    case AdcHandle::OVS_32: samples = 32; break;
                    case AdcHandle::OVS_64: samples = 64; break;
                    case AdcHandle::OVS_128: samples = 128; break;
                    case AdcHandle::OVS_256: samples = 256; break;
                    case AdcHandle::OVS_512: samples = 512; break;
                    case AdcHandle::OVS_1024: samples = 1024; break;
                    default: samples = 1; break;
                }
                uint32_t acc = 0;
                        for(int s = 0; s < samples; ++s)
                        {
                            int raw = 0;
                            adc_oneshot_read(g_adc1_handle, (adc_channel_t)adc.pin_cfg[i].adc_channel_, &raw);
                            acc += (uint32_t)raw;
                        }
                uint32_t avg = acc / (uint32_t)samples;
                uint16_t scaled = (uint16_t)((avg * 65535u) / 4095u);
                adc.last_read[i] = scaled;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(h->GetPollIntervalMs()));
    }
}
#endif

void AdcChannelConfig::InitSingle(int pin, AdcChannelConfig::ConversionSpeed speed)
{
    // Store pin number directly; no libDaisy GPIO configuration used.
    pin_   = pin;
    speed_ = speed;
}

#if defined(ESP_PLATFORM) || defined(__ESP32__)
void AdcChannelConfig::InitEsp(int adc_unit, int adc_channel, adc_atten_t atten)
{
    adc_unit_    = adc_unit;
    adc_channel_ = adc_channel;
    atten_       = atten;
}
#endif

// No DMA init required for ESP32 oneshot implementation

void AdcHandle::Init(AdcChannelConfig* cfg, size_t num_channels, OverSampling ovs)
{
    oversampling_ = ovs;
    num_channels_ = num_channels;

    // initialize internal state
    adc.channels = num_channels;
    for(size_t i = 0; i < PD_ADC_MAX_CHANNELS; i++)
        adc.last_read[i] = 0;

#if defined(ESP_PLATFORM) || defined(__ESP32__)
    // Create/configure oneshot unit for ADC1 and configure channels
    adc_oneshot_unit_init_cfg_t init_cfg;
    memset(&init_cfg, 0, sizeof(init_cfg));
    init_cfg.unit_id = ADC_UNIT_1;
    init_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
    if (g_adc1_handle == NULL)
    {
        if (adc_oneshot_new_unit(&init_cfg, &g_adc1_handle) != ESP_OK)
        {
            Error_Handler();
        }
    }

    // Configure channels
    for(size_t i = 0; i < num_channels; i++)
    {
        adc.pin_cfg[i] = cfg[i];
        if(cfg[i].adc_channel_ >= 0 && cfg[i].adc_unit_ == 1)
        {
            adc_oneshot_chan_cfg_t ch_cfg;
            memset(&ch_cfg, 0, sizeof(ch_cfg));
            ch_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
            ch_cfg.atten = (adc_atten_t)cfg[i].atten_;
            adc_oneshot_config_channel(g_adc1_handle, (adc_channel_t)cfg[i].adc_channel_, &ch_cfg);
        }
        // ADC_UNIT_2 oneshot support is not implemented here
    }
#else
    (void)cfg;
    (void)num_channels;
    (void)ovs;
#endif
}

void AdcHandle::SetPollInterval(uint32_t ms)
{
    poll_interval_ms_ = ms;
}

void AdcHandle::Start()
{
    // If a poll interval was set, start a FreeRTOS task to sample continuously.
#if defined(ESP_PLATFORM) || defined(__ESP32__)
    if(poll_interval_ms_ > 0)
    {
        xTaskCreate(adc_poll_task, "adc_poll", 2048, this, tskIDLE_PRIORITY+1, nullptr);
    }
#endif
}

void AdcHandle::Stop()
{
    (void)adc;
}

uint16_t AdcHandle::Get(uint8_t chn) const
{
    uint8_t idx = chn < PD_ADC_MAX_CHANNELS ? chn : 0;
#if defined(ESP_PLATFORM) || defined(__ESP32__)
    if(adc.pin_cfg[idx].adc_unit_ == 1 && adc.pin_cfg[idx].adc_channel_ >= 0)
    {
        // If polling is configured, return last sampled averaged value.
        if(((const AdcHandle*)this)->poll_interval_ms_ > 0)
            return adc.last_read[idx];

        // On-demand oversampling: perform N reads and average
        int samples = 1;
        switch(oversampling_)
        {
            case OVS_4: samples = 4; break;
            case OVS_8: samples = 8; break;
            case OVS_16: samples = 16; break;
            case OVS_32: samples = 32; break;
            case OVS_64: samples = 64; break;
            case OVS_128: samples = 128; break;
            case OVS_256: samples = 256; break;
            case OVS_512: samples = 512; break;
            case OVS_1024: samples = 1024; break;
            default: samples = 1; break;
        }

        uint32_t acc = 0;
        for(int s = 0; s < samples; ++s)
        {
            int raw = 0;
            adc_oneshot_read(g_adc1_handle, (adc_channel_t)adc.pin_cfg[idx].adc_channel_, &raw);
            acc += (uint32_t)raw;
        }
        uint32_t avg = acc / (uint32_t)samples;
        uint16_t scaled = (uint16_t)((avg * 65535u) / 4095u);
        const_cast<dsy_adc&>(adc).last_read[idx] = scaled;
        return scaled;
    }
    return adc.last_read[idx];
#else
    return adc.last_read[idx];
#endif
}

uint16_t* AdcHandle::GetPtr(uint8_t chn) const
{
    uint8_t idx = chn < PD_ADC_MAX_CHANNELS ? chn : 0;
    return (uint16_t*)&adc.last_read[idx];
}

float AdcHandle::GetFloat(uint8_t chn) const
{
    uint8_t idx = chn < PD_ADC_MAX_CHANNELS ? chn : 0;
#if defined(ESP_PLATFORM) || defined(__ESP32__)
    // return normalized 0..1 using 16-bit scaled value
    uint16_t raw16 = adc.last_read[idx];
    return (float)raw16 / 65535.0f;
#else
    return (float)adc.last_read[idx] / 65535.0f;
#endif
}
#if !(defined(ESP_PLATFORM) || defined(__ESP32__))
static void adc_init_msp()
{
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
(void)adcHandle;
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
(void)adcHandle;
}

extern "C"
{
    void DMA1_Stream2_IRQHandler(void) {}

    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
        (void)hadc;
    }

    void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
    {
        (void)hadc;
    }
}
#endif

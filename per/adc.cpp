#include "per/adc.h"
// Enable ESP32 ADC API usage when building for ESP-IDF
#if defined(ESP_PLATFORM) || defined(__ESP32__)
#include "driver/adc.h"
#endif

using namespace daisy;

static void Error_Handler()
{
    asm("bkpt 255");
    while(1) {}
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

void AdcChannelConfig::InitSingle(Pin pin, AdcChannelConfig::ConversionSpeed speed)
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
    // Configure ADC1 width to 12-bit (common default)
    adc_bits_width_t width = ADC_WIDTH_BIT_12;
    adc1_config_width(width);

    // Configure each channel provided
    for(size_t i = 0; i < num_channels; i++)
    {
        adc.pin_cfg[i] = cfg[i];
        if(cfg[i].adc_channel_ >= 0 && cfg[i].adc_unit_ == 1)
        {
            adc1_config_channel_atten((adc1_channel_t)cfg[i].adc_channel_, cfg[i].atten_);
        }
        // For adc_unit_ == 2, users will need to call adc2 APIs directly (not implemented here)
    }
#else
    (void)cfg;
    (void)num_channels;
    (void)ovs;
#endif
}

void AdcHandle::Start()
{
    // For oneshot implementation we don't need a dedicated Start.
    (void)adc;
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
        int raw = adc1_get_raw((adc1_channel_t)adc.pin_cfg[idx].adc_channel_);
        // store scaled to 16-bit range (raw is 0..4095 for 12-bit)
        uint16_t scaled = (uint16_t)((raw * 65535u) / 4095u);
        const_cast<dsy_adc&>(adc).last_read[idx] = scaled;
        return scaled;
    }
    else
    {
        return adc.last_read[idx];
    }
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

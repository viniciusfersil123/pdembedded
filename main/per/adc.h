#pragma once
#ifndef PD_EMBEDDED_ADC_H
#define PD_EMBEDDED_ADC_H

#include <stdint.h>
#include <stdlib.h>
// Avoid libDaisy dependency: use simple types
// Enable ESP32 ADC API usage when building for ESP-IDF
#if defined(ESP_PLATFORM) || defined(__ESP32__)
#include "driver/adc.h"
#endif

#define PD_ADC_MAX_CHANNELS 16

struct AdcChannelConfig
{
    enum ConversionSpeed
    {
        SPEED_1CYCLES_5,
        SPEED_2CYCLES_5,
        SPEED_8CYCLES_5,
        SPEED_16CYCLES_5,
        SPEED_32CYCLES_5,
        SPEED_64CYCLES_5,
        SPEED_387CYCLES_5,
        SPEED_810CYCLES_5,
    };

    void InitSingle(int pin, ConversionSpeed speed = SPEED_8CYCLES_5);

    // ESP32-specific convenience initializer
    void InitEsp(int adc_unit, int adc_channel, adc_atten_t atten = ADC_ATTEN_DB_0);

    int pin_;
    ConversionSpeed speed_;

  #if defined(ESP_PLATFORM) || defined(__ESP32__)
    int adc_unit_  = 0; // 1 or 2
    int adc_channel_ = -1;
    adc_atten_t atten_ = ADC_ATTEN_DB_0;
  #endif
};

class AdcHandle
{
  public:
    enum OverSampling
    {
        OVS_NONE,
        OVS_4,
        OVS_8,
        OVS_16,
        OVS_32,
        OVS_64,
        OVS_128,
        OVS_256,
        OVS_512,
        OVS_1024,
        OVS_LAST,
    };

    AdcHandle() {}
    ~AdcHandle() {}

    void Init(AdcChannelConfig* cfg, size_t num_channels, OverSampling ovs = OVS_32);
    void SetPollInterval(uint32_t ms);
    OverSampling GetOverSampling() const { return oversampling_; }
    uint32_t GetPollIntervalMs() const { return poll_interval_ms_; }

    void Start();
    void Stop();

    uint16_t Get(uint8_t chn) const;
    uint16_t* GetPtr(uint8_t chn) const;
    float GetFloat(uint8_t chn) const;

  private:
    OverSampling oversampling_;
    size_t       num_channels_;
    uint32_t     poll_interval_ms_ = 0;
};


#endif // PD_EMBEDDED_ADC_H

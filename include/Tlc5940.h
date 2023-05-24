#pragma once

#include <Arduino.h>

const uint8_t num_tlc = 1;

// Pin definitions
const uint8_t   TLC5940_PIN_SIN  = 14;
const uint8_t   TLC5940_PIN_SCLK  = 32;
const uint8_t   TLC5940_PIN_XLAT  = 15;
const uint8_t   TLC5940_PIN_BLANK = 33;
const uint8_t   TLC5940_PIN_GSCLK =  27;

// An LED channel is used to generate a PWM signal for the GSCLK.
// A timer is used to pulse BLANK upon completion of a GSCLK cycle.

// TLC frequency configuration
const double    TLC5940_LEDC_FRQ = 20e6;  // LEDC frequency 20MHz
const uint32_t   LEDC_DUTY_0 = 2; // Number of bits for duty cycle. See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-supported-range-frequency-duty-resolution

// timer for BLANK.
const uint8_t   TLC5940_TIMER_NUM = 0;     // hardware timer number
const uint16_t  TLC5940_TIMER_DIV = 2;    // timer divider to make 40MHz from 80MHz
const uint64_t  TLC5940_TIMER_ALM = 8192;  // interrupt every 204.80us. If a different freq. is desired calculate number of ticks based on timer speed.

// To calculate TLC5940_TIMER_ALM: (1 / TLC5940_LEDC_FRQ) * 4096 is the time it takes for a GS cycle. See TLC5940 docs.
// TLC5940_TIMER_ALM = GS cycle time / period of timer

void TLC5940_init();
void TLC5940_pin_init();
void TLC5940_ledc_init();
void TLC5940_timer_init();
void IRAM_ATTR TLC5940_onTimer();
void TLC5940_task(void* pvParameters);
void TLC5940_bitbang_gs_send();
void TLC5940_setPWM(const uint16_t chan, const uint16_t pwm);

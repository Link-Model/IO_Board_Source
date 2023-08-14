/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "mcp2515/mcp2515.h"
#include "hardware/pwm.h"
// #include "sounds/bell.h"
#include "sounds/test1khz.h"
#include "hardware/dma.h"

#define AUDIO_PIN 14
#define BUTTON_PIN 0

int audio_dma_channel;

void button_callback(uint gpio, uint32_t mask);

int main() {
  gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
  const uint audio_slice = pwm_gpio_to_slice_num(AUDIO_PIN);
  static pwm_config audio_pwm_config = pwm_get_default_config();
  pwm_init(audio_slice, &audio_pwm_config, true);
  pwm_set_wrap(audio_slice, 1u << 12);
  const uint audio_pwm_channel = pwm_gpio_to_channel(AUDIO_PIN);
  pwm_set_chan_level(audio_slice, audio_pwm_channel, 1u << 11);
  audio_dma_channel = dma_claim_unused_channel(true);
  const int audio_dma_timer = dma_claim_unused_timer(true);
  static dma_channel_config audio_dma_config = dma_channel_get_default_config(audio_dma_channel);
  channel_config_set_dreq(&audio_dma_config, dma_get_timer_dreq(audio_dma_timer));
  channel_config_set_transfer_data_size(&audio_dma_config, DMA_SIZE_16);
  dma_channel_set_config(audio_dma_channel, &audio_dma_config, false);
  dma_channel_set_trans_count(audio_dma_channel, test1khz_length, false);
  volatile void* audio_write_addr = &pwm_hw->slice[audio_slice].cc + (audio_pwm_channel ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB);
  dma_channel_set_write_addr(audio_dma_channel, audio_write_addr, false);
  dma_timer_set_fraction(audio_dma_timer, 6, 15625);
  gpio_pull_up(BUTTON_PIN);
  gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, button_callback);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  while (1);
}

void button_callback(uint gpio, uint32_t mask) {
  if (gpio == BUTTON_PIN) {
    if (mask & GPIO_IRQ_EDGE_RISE) {
      dma_channel_abort(audio_dma_channel);
      gpio_put(PICO_DEFAULT_LED_PIN, 0);
    } else if (mask & GPIO_IRQ_EDGE_FALL) {
      dma_channel_set_read_addr(audio_dma_channel, test1khz, true);
      gpio_put(PICO_DEFAULT_LED_PIN, 1);
    }
    gpio_acknowledge_irq(gpio, mask);
  }
}

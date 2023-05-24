#include "Tlc5940.h"
#include "driver/ledc.h"

static const char* TLC_TAG = "TLC5940";

uint16_t pwmbuffer[16 * num_tlc];

// initialize main
void TLC5940_init() {
  ESP_LOGI(TLC_TAG, "[TLC5940] initialization started");
  memset(pwmbuffer, 0, 16 * num_tlc);

  // hardware controls
  TLC5940_pin_init();
  TLC5940_ledc_init();
  TLC5940_timer_init();
}

// assign pins
void TLC5940_pin_init() {
  pinMode(TLC5940_PIN_SIN,  OUTPUT);
  pinMode(TLC5940_PIN_XLAT,  OUTPUT);
  pinMode(TLC5940_PIN_BLANK, OUTPUT);
  pinMode(TLC5940_PIN_SCLK, OUTPUT);
  pinMode(TLC5940_PIN_GSCLK, OUTPUT);
  digitalWrite(TLC5940_PIN_XLAT,  LOW);
  digitalWrite(TLC5940_PIN_BLANK, HIGH);  // start with BLANK
}

// start GSCLK
void TLC5940_ledc_init() {

  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .bit_num    = LEDC_TIMER_2_BIT,
      .timer_num  = LEDC_TIMER_0,
      .freq_hz    = TLC5940_LEDC_FRQ
  };
  ledc_channel_config_t ledc_channel = {
      .gpio_num   = TLC5940_PIN_GSCLK,
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .channel    = LEDC_CHANNEL_0,
      .timer_sel  = LEDC_TIMER_0,
      .duty       = LEDC_DUTY_0
  };

  ledc_timer_config(&ledc_timer);
  ledc_channel_config(&ledc_channel);

}

// timer interruption
hw_timer_t* TLC5940_timer = NULL;
volatile SemaphoreHandle_t TLC5940_timerSemaphore;
portMUX_TYPE TLC5940_timerMux = portMUX_INITIALIZER_UNLOCKED;

// set up timer
void TLC5940_timer_init() {

  // create semaphore
  TLC5940_timerSemaphore = xSemaphoreCreateBinary();

  // standby TLC5940 task
  xTaskCreatePinnedToCore(TLC5940_task,   // Function to implement the task
                         "TLC5940_task",  // Name of the task
                          4096,           // Stack size in words
                          NULL,           // Task input parameter
                          3,              // Priority of the task
                          NULL,           // Task handle.
                          1);             // Core where the task should run

  // start timer
  TLC5940_timer = timerBegin(TLC5940_TIMER_NUM, TLC5940_TIMER_DIV, true);  // increment mode
  timerAttachInterrupt(TLC5940_timer, &TLC5940_onTimer, true);             // edge mode
  timerAlarmWrite(TLC5940_timer, TLC5940_TIMER_ALM, true);                 // auto-reload mode
  timerAlarmEnable(TLC5940_timer);
}

// handle timer interruption
void IRAM_ATTR TLC5940_onTimer(){

  // BLANK
  digitalWrite(TLC5940_PIN_BLANK, HIGH);
  digitalWrite(TLC5940_PIN_BLANK, LOW);

  // wake TLC5940-task up
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(TLC5940_timerSemaphore, &xHigherPriorityTaskWoken);

}

// main task loop
void TLC5940_task(void * pvParameters) {
  for(;;) {
    // wait for semaphore
    xSemaphoreTake(TLC5940_timerSemaphore, portMAX_DELAY);
    // update gs
    TLC5940_bitbang_gs_send();
  }
}

// make and send GS data through bitbang
void TLC5940_bitbang_gs_send() {
  digitalWrite(TLC5940_PIN_XLAT, LOW);
  // 16 channels
  for (int16_t c = 16 * num_tlc - 1; c >= 0; c--) {
    // 12 bits per channel, send MSB first
    for (int8_t b = 11; b >= 0; b--) {
      digitalWrite(TLC5940_PIN_SCLK, LOW);

      if (pwmbuffer[c] & (1 << b))
        digitalWrite(TLC5940_PIN_SIN, HIGH);
      else
        digitalWrite(TLC5940_PIN_SIN, LOW);

      digitalWrite(TLC5940_PIN_SCLK, HIGH);
    }
  }
  digitalWrite(TLC5940_PIN_SCLK, LOW);

  digitalWrite(TLC5940_PIN_XLAT, HIGH);
  digitalWrite(TLC5940_PIN_XLAT, LOW);
}

void TLC5940_setPWM(const uint16_t chan, const uint16_t pwm) {
  pwmbuffer[chan] = pwm;
}

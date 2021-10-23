/*
  Streaming Music from Bluetooth

  Copyright (C) 2020 Phil Schatzmann
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// modified by Elehobica 2021

// ==> Example A2DP Receiver which uses I2S to an external 32bit DAC

#include "BluetoothA2DPSink.h"
#include "AiEsp32RotaryEncoder.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // pin number is specific to your esp32 board
#endif

esp_a2d_connection_state_t last_state;

BluetoothA2DPSink a2dp_sink;
const uint8_t PIN_ROTARY_ENCODER_A      = ((uint8_t) 21);
const uint8_t PIN_ROTARY_ENCODER_B      = ((uint8_t) 32);
const uint8_t PIN_ROTARY_ENCODER_BUTTON = ((uint8_t) -1);
#define ROTARY_ENCODER_STEPS 2
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(PIN_ROTARY_ENCODER_A, PIN_ROTARY_ENCODER_B, PIN_ROTARY_ENCODER_BUTTON, -1, ROTARY_ENCODER_STEPS);

void rotary_onButtonClick() {
}

void setup() {
  Serial.begin(115200);

  // LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);

  // Rotary Encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(
    [] { rotaryEncoder.readEncoder_ISR(); },
    [] { rotary_onButtonClick(); }
  );
  rotaryEncoder.setBoundaries(0, 127, false); //minValue, maxValue, circleValues true|false
  rotaryEncoder.setAcceleration(20);

  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100, // corrected by info from bluetooth
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  a2dp_sink.set_i2s_config(i2s_config);
  a2dp_sink.start("NormalVol");
  a2dp_sink.set_volume(25);
  rotaryEncoder.setEncoderValue(25);
}

void loop() {
  // check state
  esp_a2d_connection_state_t state = a2dp_sink.get_connection_state();
  if (last_state != state){
    bool is_connected = state == ESP_A2D_CONNECTION_STATE_CONNECTED;
    Serial.println(is_connected ? "Connected" : "Not connected");
    digitalWrite(LED_BUILTIN, is_connected);
    last_state = state;
  }
  // volume control
  if (rotaryEncoder.encoderChanged()) {
    int volume = rotaryEncoder.readEncoder();
    a2dp_sink.set_volume(volume);
  }
  delay(100);
}

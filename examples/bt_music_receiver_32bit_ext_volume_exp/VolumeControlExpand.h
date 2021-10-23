#pragma once
#include "BluetoothA2DPCommon.h"

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright 2020 Phil Schatzmann
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD

/**
 * @brief scale 16bit+ volume implementation for handling of the volume of the 32bit audio data / channel
 * @author elehobica
 * @copyright Apache License Version 2
 */

struct __attribute__((packed)) Frame32 {
  int32_t channel1;
  int32_t channel2;

  Frame32(int v=0){
    channel1 = channel2 = v;
  }
};

class VolumeControlExpand : public VolumeControl {
    public:
        virtual uint8_t get_volume_input_max() = 0;
        virtual void update_audio_data(Frame* frame, Frame32* frame32, uint16_t frameCount, uint8_t volume, bool mono_downmix, bool is_volume_used) = 0;
};

class VolumeControlExpandDefault : public VolumeControlExpand {
    private:
        static constexpr double BASE = 1.3; // define curve characteristiscs (base)
        static constexpr double BITS = 16;  // define curve characteristiscs (power)
        static constexpr double VOLUME_ZERO_OFS = pow(BASE, -BITS); // offset value to set 0 to voumeFactor when volume 0
        static constexpr int32_t DAC_ZERO = 1; // to avoid pop noise caused by auto-mute function of DAC

    public:
        static constexpr double VOLUME_INPUT_MAX = 100;       
        static constexpr double VOLUME_FACTOR_MAX = pow(2.0, BITS) + 2;

        void update_audio_data(Frame* frame, Frame32* frame32, uint16_t frameCount, uint8_t volume, bool mono_downmix, bool is_volume_used)
        {
            if (mono_downmix || is_volume_used) {
                ESP_LOGD(BT_AV_TAG, "update_audio_data");
                int32_t volumeFactorMax = get_volume_factor_max();
                int32_t volumeFactor = get_volume_factor(volume);
                for (int i=0;i<frameCount;i++){
                    int32_t pcmLeft = frame[i].channel1;
                    int32_t pcmRight = frame[i].channel2;
                    // avoid -32768 makes overflow when volumeFactorMax
                    if (pcmLeft < -32767) { pcmLeft = -32767; }
                    if (pcmRight < -32767) { pcmRight = -32767; }
                    // if mono -> we provide the same output on both channels
                    if (mono_downmix) {
                        pcmRight = pcmLeft = (pcmLeft + pcmRight) / 2;
                    }
                    // adjust the volume
                    if (!is_volume_used) {
                        pcmLeft *= volumeFactorMax;
                        pcmRight *= volumeFactorMax;
                    } else {
                        pcmLeft *= volumeFactor;
                        pcmRight *= volumeFactor;
                    }
                    pcmLeft += DAC_ZERO;
                    pcmRight += DAC_ZERO;
                    frame32[i].channel1 = pcmLeft;
                    frame32[i].channel2 = pcmRight;
                }
            }
        }

        int32_t get_volume_factor(uint8_t volume) {
            double volumeFactorFloat = (pow(BASE, volume * BITS / VOLUME_INPUT_MAX - BITS) - VOLUME_ZERO_OFS) * VOLUME_FACTOR_MAX / (1.0 - VOLUME_ZERO_OFS);
            int32_t volumeFactor = volumeFactorFloat;
            if (volumeFactor > VOLUME_FACTOR_MAX) {
                volumeFactor = VOLUME_FACTOR_MAX;
            }
            return volumeFactor;
        }

        // provides the max input value
        uint8_t get_volume_input_max() {
            return VOLUME_INPUT_MAX;
        }
        
        // provides the max factor value
        int32_t get_volume_factor_max() {
            return VOLUME_FACTOR_MAX;
        }

};

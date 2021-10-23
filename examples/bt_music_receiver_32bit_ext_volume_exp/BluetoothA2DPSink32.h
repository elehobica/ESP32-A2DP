#pragma once
#include "BluetoothA2DPSink.h"
#include "VolumeControlExpand.h"

/**
 * @brief Custom 32 bit stereo output. 
 * We demonstrate how we can adapt the functionality by subclassing and overwriting a method. 
 * it seems that i2s_write_expand doesn't utilize the resolution below 16bits. 
 * It apparently makes difference in sound quality when small volume value is applied if 
 * lower bits are fully used.
 *
 * @author elehobica
 **/

class BluetoothA2DPSink32 : public BluetoothA2DPSink {
    public:
        virtual void set_volume(uint8_t volume) {
            ESP_LOGI(BT_AV_TAG, "set_volume %d", volume);
            is_volume_used = true;
            if (volume > volume_control()->get_volume_input_max()) {
                volume = volume_control()->get_volume_input_max();
            }
            s_volume = volume;

            #ifdef CURRENT_ESP_IDF
              volume_set_by_local_host(s_volume);
            #endif
        }

        virtual void set_volume_control(VolumeControlExpand *ptr){
            volume_control_ptr = ptr;
        }

    protected:
        VolumeControlExpandDefault default_volume_control;
        VolumeControlExpand *volume_control_ptr = nullptr;

        virtual VolumeControlExpand* volume_control() {
            return (volume_control_ptr != nullptr) ? volume_control_ptr : &default_volume_control;
        }

        virtual void audio_data_callback(const uint8_t *data16, uint32_t len) {
            static constexpr int blk_size = 128;
            static int32_t data32[blk_size/2];

            ESP_LOGD(BT_AV_TAG, "%s", __PRETTY_FUNCTION__);
            // convert to array of frames
            Frame* frame = (Frame*) data16;
            Frame32* frame32 = (Frame32*) data32;
            uint32_t rest_len = len;

            while (rest_len>0) {
                uint32_t blk_len = (rest_len>=blk_size) ? blk_size : rest_len;
                volume_control()->update_audio_data(frame, frame32, blk_len/4, s_volume, mono_downmix, is_volume_used);
                frame += blk_len/4;
                                
                size_t i2s_bytes_written;
                if (i2s_write(i2s_port,(void*) data32, blk_len*2, &i2s_bytes_written, portMAX_DELAY)!=ESP_OK){
                    ESP_LOGE(BT_AV_TAG, "i2s_write has failed");
                }

                if (i2s_bytes_written<blk_len*2){
                    ESP_LOGE(BT_AV_TAG, "Timeout: not all bytes were written to I2S");
                }
                rest_len -= blk_len;
            }
        }
};

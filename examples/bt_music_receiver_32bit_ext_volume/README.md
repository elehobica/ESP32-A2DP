# bt_music_receiver_32bit_ext_volume

## Overview
This project is for A2DP to output 32bit DAC.

This project supports:
* PCM5102 DAC in 32bit mode
* Volume function utilizing 32bit full range  (volume input: 0 ~ 127, volumeFactor: 0 ~ 4096)
* Volume control by Rotary Encoder

## Supported Board
* ESP32 (ESP32-DevkitC)
* PCM5102 Board

## Pin Assignment

| ESP32 Pin | GPIO Function | Assignment |
----|----|----
| GPIO2 | OUTPUT | Blue LED |
| GPIO21 | INPUT | ROTARY_ENCODER_A |
| GPIO32 | INPUT | ROTARY_ENCODER_B |
| GPIO22 | OUTPUT | PCM5102 DIN |
| GPIO25 | OUTPUT | PCM5102 LRCK |
| GPIO26 | OUTPUT | PCM5102 BCK |

Bridge short lands of PCM5102 SCK (tie to GND)

## Installing Libraries to Arduino
### Ai Esp32 Rotary Encoder
Arduino -> 'Tools' -> 'Manage Libraries...' -> Install 'Ai Esp32 Rotary Encoder' Version 1.2.0 or later<br />
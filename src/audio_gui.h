#pragma once

#include "audio/audio.h"

void DrawAudioDeviceGui(AudioManager* audio_manager, float rms);

void DrawWaveformPlot(const float* data, size_t size);

void DrawAudioFileGui(AudioManager* audio_manager);
#pragma once

#include <string>

class AudioFileManager
{
  public:
    AudioFileManager() = default;
    virtual ~AudioFileManager() = default;

    virtual bool OpenAudioFile(std::string_view file_name) = 0;

    virtual void ProcessBlock(float* out_buffer, size_t frame_size, size_t num_channels, float gain = 1.f) = 0;
};
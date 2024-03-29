#pragma once

#include "audio_file_manager.h"

#include <sndfile.h>
#include <vector>
#include <atomic>

class SndFileManagerImpl : public AudioFileManager
{
public:
    SndFileManagerImpl() = default;
    virtual ~SndFileManagerImpl() = default;

    bool OpenAudioFile(std::string_view file_name) override;
    void ProcessBlock(float* out_buffer, size_t frame_size, size_t num_channels, float gain = 1.f) override;

private:
    SNDFILE* file_ = nullptr;
    SF_INFO file_info_;
    size_t current_frame_ = 0;
    std::atomic<bool> is_playing_ = false;
    bool is_paused_ = false;

    std::vector<float> buffer_;
};
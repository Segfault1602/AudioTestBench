#include "sndfile_manager_impl.h"

#include <iostream>
#include <cassert>


bool SndFileManagerImpl::OpenAudioFile(std::string_view file_name)
{
    file_ = sf_open(file_name.data(), SFM_READ, &file_info_);
    if (!file_)
    {
        return false;
    }

    std::cout << "Opened file: " << file_name << std::endl;
    std::cout << "Channels: " << file_info_.channels << std::endl;
    std::cout << "Sample rate: " << file_info_.samplerate << std::endl;
    std::cout << "Frames: " << file_info_.frames << std::endl;
    std::cout << "Format:" << file_info_.format << std::endl;

    is_playing_ = true;

    return true;
}

void SndFileManagerImpl::ProcessBlock(float* out_buffer, size_t frame_size, size_t num_channels, float gain)
{
    if (!file_ || !is_playing_ || is_paused_)
    {
        return;
    }

    size_t total_size = frame_size * num_channels;
    if (buffer_.size() < total_size)
    {
        buffer_.resize(total_size);
    }

    size_t read = sf_readf_float(file_, buffer_.data(), frame_size);
    if (read == 0)
    {
        is_playing_ = false;
        return;
    }

    if (file_info_.channels == num_channels)
    {
        for (size_t i = 0; i < read; ++i)
        {
            for (size_t j = 0; j < num_channels; ++j)
            {
                out_buffer[i * num_channels + j] = buffer_[i * num_channels + j] * gain;
            }
        }
    }
    else
    {
        assert(file_info_.channels == 1);
        for (size_t i = 0; i < read; ++i)
        {
            for (size_t j = 0; j < num_channels; ++j)
            {
                out_buffer[i * num_channels + j] = buffer_[i];
            }
        }
    }

    current_frame_ += read;
}
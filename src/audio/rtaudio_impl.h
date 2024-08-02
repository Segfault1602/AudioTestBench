#pragma once

#include <RtAudio.h>

#include <atomic>
#include <filter.h>
#include <sndfile.h>

#include "audio.h"
#include "ring_buffer.h"
#include "test_tone.h"
#include "audio_file_manager.h"

class RtAudioManagerImpl : public AudioManager
{
  public:
    RtAudioManagerImpl();
    ~RtAudioManagerImpl() override;

    bool StartAudioStream() override;
    void StopAudioStream() override;
    bool IsAudioStreamRunning() const override;
    virtual AudioStreamInfo GetAudioStreamInfo() const override;

    void SetOutputDevice(std::string_view device_name) override;
    void SetInputDevice(std::string_view device_name) override;
    void SetAudioDriver(std::string_view driver_name) override;
    void SelectInputChannels(uint8_t channels) override;

    std::vector<std::string> GetOutputDevicesName() const override;
    std::vector<std::string> GetInputDevicesName() const override;
    std::vector<std::string> GetSupportedAudioDrivers() const override;
    std::string GetCurrentAudioDriver() const override;

    void PlayTestTone(bool play) override;
    float GetInputLevel() const override;

    size_t GetAvailableAudioBufferSize() const override;
    size_t ReadAudioBuffer(float* buffer, size_t buffer_size) override;

    AudioFileManager* GetAudioFileManager() override;

  private:
    static int RtAudioCbStatic(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime,
                               RtAudioStreamStatus status, void* userData);
    int RtAudioCbImpl(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime,
                      RtAudioStreamStatus status);

    std::unique_ptr<RtAudio> rtaudio_;
    RtAudio::StreamParameters output_stream_parameters_;
    RtAudio::StreamParameters input_stream_parameters_;

    int current_output_device_id_ = -1;
    int current_input_device_id_ = -1;

    uint8_t input_selected_channels_ = 0;

    uint32_t buffer_size_ = 512;
    uint32_t sample_rate_ = 48000;
    RtAudio::Api current_audio_api_ = RtAudio::Api::UNSPECIFIED;

    bool play_test_tone_ = false;

    TestToneGenerator test_tone_;

    std::unique_ptr<float[]> level_out_scratch_buffer_;
    sfdsp::OnePoleFilter input_level_filter_;
    std::atomic<float> input_level_ = 0.f;

    RingBuffer<float> audio_buffer_;

    std::unique_ptr<AudioFileManager> audio_file_manager_;
};
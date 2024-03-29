#include "audio_gui.h"

#include "imgui.h"

#include "imfilebrowser.h"
#include "implot.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sndfile.h>
#include <vector>

#include "audio/ring_buffer.h"

namespace
{
SF_INFO file_info;
SNDFILE* file = nullptr;
} // namespace

void DrawAudioDeviceGui(AudioManager* audio_manager, float rms)
{
    assert(audio_manager != nullptr);

    static std::vector<std::string> supported_audio_drivers = audio_manager->GetSupportedAudioDrivers();
    static std::vector<std::string> output_devices = audio_manager->GetOutputDevicesName();
    static std::vector<std::string> input_devices = audio_manager->GetInputDevicesName();

    ImGui::Begin("Audio Devices");

    // Audio Drivers Combo
    ImGui::Text("Audio Drivers ");
    ImGui::SameLine();
    static int selected_audio_driver = 0;
    if (ImGui::BeginCombo("##Audio Drivers", audio_manager->GetCurrentAudioDriver().c_str()))
    {
        for (int i = 0; i < supported_audio_drivers.size(); i++)
        {
            bool is_selected = (selected_audio_driver == i);
            if (ImGui::Selectable(supported_audio_drivers[i].c_str(), is_selected))
            {
                selected_audio_driver = i;
                std::cout << "Selected Audio Driver: " << supported_audio_drivers[i] << std::endl;
                audio_manager->SetAudioDriver(supported_audio_drivers[i]);
                // Refresh audio devices
                output_devices = audio_manager->GetOutputDevicesName();
                input_devices = audio_manager->GetInputDevicesName();
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Output Devices Combo
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Output Devices");
    ImGui::SameLine();
    static int selected_output_device = 0;
    if (ImGui::BeginCombo("##Output Devices", output_devices[selected_output_device].c_str()))
    {
        for (int i = 0; i < output_devices.size(); i++)
        {
            bool is_selected = (selected_output_device == i);
            if (ImGui::Selectable(output_devices[i].c_str(), is_selected))
            {
                selected_output_device = i;
                std::cout << "Selected Output Device: " << output_devices[i] << std::endl;
                audio_manager->SetOutputDevice(output_devices[i]);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Input Devices Combo
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Input Devices ");
    ImGui::SameLine();
    static int selected_input_device = 0;
    if (ImGui::BeginCombo("##Input Devices", input_devices[selected_input_device].c_str()))
    {
        for (int i = 0; i < input_devices.size(); i++)
        {
            bool is_selected = (selected_input_device == i);
            if (ImGui::Selectable(input_devices[i].c_str(), is_selected))
            {
                selected_input_device = i;
                std::cout << "Selected Input Device: " << input_devices[i] << std::endl;
                audio_manager->SetInputDevice(input_devices[i]);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    auto audio_stream_info = audio_manager->GetAudioStreamInfo();
    static int selected_input_channel = 0;
    ImGui::SameLine();
    ImGui::Text("Input Channels: ");
    for (int i = 0; i < audio_stream_info.num_input_channels; i++)
    {
        ImGui::SameLine();
        if (ImGui::RadioButton(std::to_string(i).c_str(), &selected_input_channel, i))
        {
            std::cout << "Selected Input Channel: " << i << std::endl;
            audio_manager->SelectInputChannels(i);
        }
    }

    ImGui::Text("Stream Status: ");
    ImGui::SameLine();
    if (audio_manager->IsAudioStreamRunning())
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Running");
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Stopped");
    }

    ImGui::Text("Sample Rate: %d", audio_stream_info.sample_rate);
    ImGui::Text("Buffer Size: %d", audio_stream_info.buffer_size);
    ImGui::Text("Num Input Channels: %d", audio_stream_info.num_input_channels);
    ImGui::Text("Num Output Channels: %d", audio_stream_info.num_output_channels);

    static bool play_test_tone = false;
    if (ImGui::Checkbox("Play Test Tone", &play_test_tone))
    {
        audio_manager->PlayTestTone(play_test_tone);
        if (play_test_tone)
        {
            file_info.channels = 1;
            file_info.samplerate = 48000;
            file_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

            file = sf_open("test_out.wav", SFM_WRITE, &file_info);
        }
        else
        {
            sf_close(file);
            file = nullptr;
        }
    }

    ImGui::ProgressBar(rms, ImVec2(-100.f, 0.f), "");

    ImGui::End();
}

void DrawWaveformPlot(const float* data, size_t size)
{
    static bool init = false;
    static RingBuffer ring_buffer;
    static std::vector<float> plot_data;
    const size_t sample_rate = 48000;

    if (!init)
    {
        init = true;
        // Plot ~ half a second?
        ring_buffer.Resize(16384);
        plot_data.resize(16384);
    }
    ImGui::Begin("Scope");
    static bool freeze = false;
    ImGui::Checkbox("Freeze", &freeze);

    ImGui::SameLine();
    uint32_t zoom_level[] = {5, 10, 50, 100};
    static int selected_zoom = 0;
    if (ImGui::BeginCombo("Zoom", std::format("{} ms", zoom_level[selected_zoom]).c_str(),
                          ImGuiComboFlags_WidthFitPreview))
    {
        for (int i = 0; i < 4; i++)
        {
            bool is_selected = (selected_zoom == i);
            if (ImGui::Selectable(std::format("{} ms", zoom_level[i]).c_str(), is_selected))
            {
                selected_zoom = i;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (file && size > 0)
    {
        size_t write = sf_writef_float(file, data, size);
    }

    if (!freeze && size > 0)
    {
        size_t available_write = ring_buffer.GetWriteAvailable();
        if (available_write < size)
        {
            // read the difference
            size_t read_size = size - available_write;
            ring_buffer.Read(plot_data.data(), read_size);
        }

        available_write = ring_buffer.GetWriteAvailable();
        assert(available_write >= size);
        ring_buffer.Write(data, size);

        size_t peek_size = ring_buffer.GetReadAvailable();
        ring_buffer.Peek(plot_data.data(), peek_size);
    }

    if (ImPlot::BeginPlot("##Scope", ImVec2(-1, -1)))
    {
        size_t zoom_samples = sample_rate * zoom_level[selected_zoom] / 1000;
        zoom_samples = min(zoom_samples, plot_data.size());

        ImPlot::SetupAxes("Time", "Signal");
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, zoom_samples, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1, 1);
        ImPlot::PlotLine("wave", plot_data.data(), zoom_samples);

        ImPlot::EndPlot();
    }

    ImGui::End();
}

void DrawAudioFileGui(AudioManager* audio_manager)
{
    static ImGui::FileBrowser file_dialog;
    static std::string audio_file;

    if (ImGui::Begin("Audio File"))
    {
        ImGui::SeparatorText("Audio Player");
        {
            if (ImGui::Button("Open File"))
            {
                file_dialog.Open();
            }
            ImGui::SameLine();
            ImGui::Text("%s", audio_file.c_str());

            ImGui::Button("Play");
            ImGui::SameLine();
            ImGui::Button("Pause");
            ImGui::SameLine();
            ImGui::Button("Stop");
        }
    }

    file_dialog.Display();
    if (file_dialog.HasSelected())
    {
        audio_file = file_dialog.GetSelected().string();
        std::cout << "Selected file: " << audio_file << std::endl;
        file_dialog.ClearSelected();
        audio_manager->GetAudioFileManager()->OpenAudioFile(audio_file);
    }

    ImGui::End();
}
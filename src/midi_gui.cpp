#include "midi_gui.h"

#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "imgui-knobs.h"
#include "imgui.h"

void DrawMidiDeviceWindow(MidiManager* midi_manager)
{
    static std::vector<std::string> devices = midi_manager->GetMidiInputDevicesName();
    static std::vector<bool> selected_devices(devices.size(), false);
    static std::vector<MidiMessage> midi_logs;

    ImGui::Begin("MIDI Devices");
    ImGui::Text("MIDI Input Devices");

    for (auto i = 0; i < devices.size(); i++)
    {
        bool is_selected = selected_devices[i];
        if (ImGui::Checkbox(devices[i].c_str(), &is_selected))
        {
            selected_devices[i] = is_selected;
            std::cout << "Selected device: " << devices[i] << std::endl;
            if (is_selected)
            {
                midi_manager->OpenMidiInputDevice(devices[i]);
            }
            else
            {
                midi_manager->CloseMidiInputDevice(devices[i]);
            }
        }
    }

    std::vector<MidiMessage> midi_msgs = midi_manager->ReadMidiMessages();
    midi_logs.insert(midi_logs.end(), midi_msgs.begin(), midi_msgs.end());

    ImGui::Separator();

    static bool auto_scroll = true;
    ImGui::Checkbox("Auto-scroll", &auto_scroll);

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("MidiLogTable", 4, flags))
    {
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 0.0f);
        ImGui::TableSetupColumn("Channel", ImGuiTableColumnFlags_WidthFixed, 0.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0.0f);
        ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthStretch, 0.0f);
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableHeadersRow();

        for (auto& midi_log : midi_logs)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<int>(midi_log.stamp));
            ImGui::TableNextColumn();
            ImGui::Text("%d", midi_log.channel);
            ImGui::TableNextColumn();
            ImGui::Text("%s", MidiMessageTypeToString(midi_log.type).c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d %d", midi_log.data1, midi_log.data2);
        }

        if (auto_scroll)
        {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void DrawMidiKnobGrid()
{
    ImGui::Begin("MIDI Knob Grid");

    static int value = 0;
    if (ImGuiKnobs::KnobInt("Knob 1", &value, 0, 127, 0, 0, ImGuiKnobVariant_WiperDot))
    {
        // Do something when the knob is changed
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            std::cout << "Right-clicked on Knob 1\n" << std::endl;
        }
    }
    ImGui::SameLine();

    if (ImGuiKnobs::KnobInt("Knob 2", &value, 0, 127))
    {
        // Do something when the knob is changed
    }

    ImGui::End();
}
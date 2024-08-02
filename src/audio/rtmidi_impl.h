#pragma once

#include "midi_manager.h"

#include <memory>
#include <vector>

#include <RtMidi.h>

class RtMidiImpl : public MidiManager
{
  public:
    RtMidiImpl();
    ~RtMidiImpl() override = default;

    std::vector<std::string> GetMidiInputDevicesName() const override;
    void OpenMidiInputDevice(const std::string_view device_name) override;
    void CloseMidiInputDevice(const std::string_view device_name) override;

    std::vector<MidiMessage> ReadMidiMessages() override;

  private:
    std::unique_ptr<RtMidiIn> midi_in_;
};
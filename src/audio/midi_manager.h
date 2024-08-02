#pragma once

#include <memory>
#include <string>
#include <vector>

enum class MidiMessageType : uint8_t
{
    NoteOff = 0x80,
    NoteOn = 0x90,
    ControllerChange = 0xB0,
    ChannelAftertouch = 0xD0,
    PitchBend = 0xE0,
    Unknown = 0x00,
};

struct MidiMessage
{
    uint64_t stamp;
    MidiMessageType type;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;
};

std::string MidiMessageTypeToString(MidiMessageType type);

class MidiManager
{
  public:
    static std::unique_ptr<MidiManager> CreateMidiManager();

    MidiManager() = default;
    virtual ~MidiManager() = default;

    virtual std::vector<std::string> GetMidiInputDevicesName() const = 0;
    virtual void OpenMidiInputDevice(const std::string_view device_name) = 0;
    virtual void CloseMidiInputDevice(const std::string_view device_name) = 0;

    virtual std::vector<MidiMessage> ReadMidiMessages() = 0;
};
#include "midi_manager.h"

#include "rtmidi_impl.h"

std::unique_ptr<MidiManager> MidiManager::CreateMidiManager()
{
    return std::make_unique<RtMidiImpl>();
}

std::string MidiMessageTypeToString(MidiMessageType type)
{
    switch (type)
    {
    case MidiMessageType::NoteOff:
        return "NoteOff";
    case MidiMessageType::NoteOn:
        return "NoteOn";
    case MidiMessageType::ControllerChange:
        return "ControllerChange";
    case MidiMessageType::ChannelAftertouch:
        return "ChannelAftertouch";
    case MidiMessageType::PitchBend:
        return "PitchBend";
    case MidiMessageType::Unknown:
    default:
        return "Unknown";
    }
}
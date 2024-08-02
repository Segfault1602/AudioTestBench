#include "rtmidi_impl.h"

#include <iostream>

RtMidiImpl::RtMidiImpl()
{
    midi_in_ = std::make_unique<RtMidiIn>();
}

std::vector<std::string> RtMidiImpl::GetMidiInputDevicesName() const
{
    std::vector<std::string> devices;
    for (unsigned int i = 0; i < midi_in_->getPortCount(); i++)
    {
        std::cout << "MIDI Input Device: " << midi_in_->getPortName(i) << std::endl;
        devices.push_back(midi_in_->getPortName(i));
    }
    return devices;
}

void RtMidiImpl::OpenMidiInputDevice(const std::string_view device_name)
{
    for (unsigned int i = 0; i < midi_in_->getPortCount(); i++)
    {
        if (midi_in_->getPortName(i) == device_name)
        {
            if (midi_in_->isPortOpen())
            {
                midi_in_->closePort();
            }
            midi_in_->openPort(i);
            break;
        }
    }
}

void RtMidiImpl::CloseMidiInputDevice(const std::string_view device_name)
{
    for (unsigned int i = 0; i < midi_in_->getPortCount(); i++)
    {
        if (midi_in_->getPortName(i) == device_name)
        {
            midi_in_->closePort();
            break;
        }
    }
}

std::vector<MidiMessage> RtMidiImpl::ReadMidiMessages()
{
    std::vector<MidiMessage> messages;
    std::vector<unsigned char> message;
    double stamp;
    while ((stamp = midi_in_->getMessage(&message)))
    {
        if (message.size() == 0)
        {
            break;
        }
        MidiMessage midi_msg;
        midi_msg.stamp = stamp * 1000;

        uint8_t type = message[0] & 0xF0;
        switch (type)
        {
        case 0x80:
            midi_msg.type = MidiMessageType::NoteOff;
            break;
        case 0x90:
            midi_msg.type = MidiMessageType::NoteOn;
            break;
        case 0xB0:
            midi_msg.type = MidiMessageType::ControllerChange;
            break;
        case 0xD0:
            midi_msg.type = MidiMessageType::ChannelAftertouch;
            break;
        case 0xE0:
            midi_msg.type = MidiMessageType::PitchBend;
            break;
        default:
            midi_msg.type = MidiMessageType::Unknown;
            break;
        }

        midi_msg.channel = message[0] & 0x0F;

        if (message.size() == 2)
        {
            midi_msg.data1 = message[1];
            midi_msg.data2 = 0;
        }
        else if (message.size() == 3)
        {
            midi_msg.data1 = message[1];
            midi_msg.data2 = message[2];
        }

        messages.push_back(midi_msg);
    }
    return messages;
}
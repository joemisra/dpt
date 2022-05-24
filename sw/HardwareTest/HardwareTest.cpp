#include "daisysp.h"
#include "../../lib/daisy_dpt.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;
using namespace dpt;
using namespace daisysp;

DPT hw;

Switch         button, toggle;
FIL            file; /**< Can't be made on the stack (DTCMRAM) */
FatFSInterface fsi;

using MyOledDisplay = OledDisplay<SSD130xI2c128x32Driver>;
MyOledDisplay         display;

uint8_t sumbuff[1024];

void UsbCallback(uint8_t *buf, uint32_t *len)
{
    for (size_t i = 0; i < *len; i++)
    {
        sumbuff[i] = buf[i];
    }
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();
    button.Debounce();
    toggle.Debounce();
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}
void MIDISendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t data[3] = {0};

    data[0] = (channel & 0x0F) + 0x90; // limit channel byte, add status byte
    data[1] = note & 0x7F;             // remove MSB on data
    data[2] = velocity & 0x7F;

    hw.midi.SendMessage(data, 3);
}

void MIDISendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t data[3] = {0};

    data[0] = (channel & 0x0F) + 0x80; // limit channel byte, add status byte
    data[1] = note & 0x7F;             // remove MSB on data
    data[2] = velocity & 0x7F;

    hw.midi.SendMessage(data, 3);
}

int main(void)
{
    hw.Init();

    //hw.midi.StartReceive();
    //hw.StartLog(false);

    //hw.usb.Init(UsbHandle::UsbPeriph::FS_EXTERNAL);

    uint32_t now, dact, usbt, gatet;
    now = dact = usbt = System::GetNow();
    gatet             = now;

    //hw.usb_midi.StartReceive();

    while(1)
    {
        now = System::GetNow();
        hw.usb_midi.Listen();

        if(hw.usb_midi.HasEvents()) {
            auto event = hw.midi.PopEvent();

            if(event.type  == MidiMessageType::NoteOn) {
                dsy_gpio_write(&hw.gate_out_1, 1);
            }
            else if(event.type  == MidiMessageType::NoteOff) {
                dsy_gpio_write(&hw.gate_out_1, 0);
            }
            else if(event.type == MidiMessageType::ControlChange) {
                ControlChangeEvent e = event.AsControlChange();
                hw.WriteCvOut(CV_OUT_2, ((float)e.value / 127.) * 5.f, false);
            }
        } else {
            dsy_gpio_write(&hw.gate_out_2, 0);
        }
        hw.Delay(50);
    }
}
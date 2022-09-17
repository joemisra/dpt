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
Oscillator osc[6];

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

    for(int i=0; i<6; i++) {
        osc[i].SetFreq(hw.controls[CV_1].Value() * 100.f);
    }

    button.Debounce();
    toggle.Debounce();



    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }

    /* These methods want 0-4095 if that last 'raw' parameter is true,
       so the amplitude of all of these oscillators is set to 2048.f, and this is bipolar.
       So we add 2048.f to bring it up.
       */
    hw.WriteCvOutExp(osc[0].Process() + 2048.f, osc[1].Process() + 2048.f, osc[2].Process() + 2048.f, osc[3].Process() + 2048.f, true);
    hw.WriteCvOut(1, osc[4].Process() + 2048.f, true);
    hw.WriteCvOut(2, osc[5].Process() + 2048.f, true);
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
    float samplerate = 96000;
    hw.Init();
    hw.SetAudioSampleRate(samplerate);

    /* Set up some basic oscillators to write to the internal/external 12-bit DACs */

    for(int i=0;i<6;i++) {
        osc[i].Init(samplerate);
        osc[i].SetWaveform(i);
        osc[i].SetAmp(2048.f);
        osc[i].Reset(i * (1/6.f));
        osc[i].SetFreq(10.f);
    }

    SdmmcHandler::Config sd_config;
    SdmmcHandler         sdcard;
    sd_config.Defaults();
    sdcard.Init(sd_config);

    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    /** Write/Read text file */
    const char *test_string = "Testing Daisy Patch SM";
    const char *test_fname  = "DaisyPatchSM-Test.txt";
    FRESULT     fres = FR_DENIED; /**< Unlikely to actually experience this */
    if(f_mount(&fsi.GetSDFileSystem(), "/", 0) == FR_OK)
    {
        /** Write Test */
        if(f_open(&file, test_fname, (FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK)
        {
            UINT   bw  = 0;
            size_t len = strlen(test_string);
            fres       = f_write(&file, test_string, len, &bw);
        }
        f_close(&file);
        if(fres == FR_OK)
        {
            /** Read Test only if Write passed */
            if(f_open(&file, test_fname, (FA_OPEN_EXISTING | FA_READ)) == FR_OK)
            {
                UINT   br = 0;
                char   readbuff[32];
                size_t len = strlen(test_string);
                fres       = f_read(&file, readbuff, len, &br);
            }
            f_close(&file);
        }
    }

    bool sdmmc_pass = fres == FR_OK;

    //hw.midi.StartReceive();
    //hw.StartLog(false);

    //hw.usb.Init(UsbHandle::UsbPeriph::FS_EXTERNAL);

    uint32_t now, dact, usbt;
    now = dact = usbt = System::GetNow();

    hw.midi.StartReceive();

    hw.StartAudio(AudioCallback);

    while(1)
    {
        /* This loop will listen for MIDI and trigger gates for feedback */
        now = System::GetNow();
        hw.midi.Listen();

        if(hw.midi.HasEvents()) {
            auto event = hw.midi.PopEvent();

            if(event.type  == MidiMessageType::NoteOn) {
                dsy_gpio_write(&hw.gate_out_1, 1);
            }
            else if(event.type  == MidiMessageType::NoteOff) {
                dsy_gpio_write(&hw.gate_out_1, 0);
            }
            /*
            else if(event.type == MidiMessageType::ControlChange) {
                ControlChangeEvent e = event.AsControlChange();
                hw.WriteCvOut(CV_OUT_2, ((float)e.value / 127.) * 5.f, false);
            }
            */
        } else {
            dsy_gpio_write(&hw.gate_out_1, 0);
            dsy_gpio_write(&hw.gate_out_2, 0);
        }
        hw.Delay(50);
    }
}
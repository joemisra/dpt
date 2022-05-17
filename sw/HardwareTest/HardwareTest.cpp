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

    /** Memory tests first to keep test short if everything else fails */
    bool sdram_pass = hw.ValidateSDRAM();
    hw.PrintLine("SDRAM Test\t-\t%s", sdram_pass ? "PASS" : "FAIL");
    bool qspi_pass = hw.ValidateQSPI();
    hw.PrintLine("QSPI Test\t-\t%s", qspi_pass ? "PASS" : "FAIL");

    /** SD card next */
    SdmmcHandler::Config sd_config;
    SdmmcHandler sdcard;
    sd_config.Defaults();

    sdcard.Init(sd_config);

    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    /** Write/Read text file */
    const char *test_string = "Testing Daisy Patch SM";
    const char *test_fname  = "DaisyPatchSM-Test.txt";
    FRESULT     fres = FR_DENIED, roro; /**< Unlikely to actually experience this */
    if(f_mount(&fsi.GetSDFileSystem(), "/", 0) == FR_OK)
    {
        /** Write Test */
        roro = f_open(&file, test_fname, (FA_CREATE_ALWAYS | FA_WRITE));
        if(roro == FR_OK)
        {
            UINT   bw  = 0;
            size_t len = strlen(test_string);
            fres       = f_write(&file, test_string, len, &bw);
        } else {
            // hay
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
    hw.PrintLine("SDMMC Test\t-\t%s", sdmmc_pass ? "PASS" : "FAIL");

    /** 5 second delay before starting streaming test. */
   //System::Delay(5000);


    /** Initialize Button/Toggle for rest of test. */
    button.Init(DPT::B7, hw.AudioCallbackRate());
    toggle.Init(DPT::B8, hw.AudioCallbackRate());

    daisysp::Phasor dacphs;
    dacphs.Init(500);
    dacphs.SetFreq(1.f);

    hw.StartAudio(AudioCallback);

    uint32_t now, dact, usbt, gatet;
    now = dact = usbt = System::GetNow();
    gatet             = now;

    char *outout = new char[256];

    MyOledDisplay::Config disp_cfg;
    disp_cfg.driver_config.transport_config.i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    disp_cfg.driver_config.transport_config.i2c_config.speed = I2CHandle::Config::Speed::I2C_1MHZ;
    disp_cfg.driver_config.transport_config.i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
    disp_cfg.driver_config.transport_config.i2c_config.pin_config.scl = DPT::B7;
    disp_cfg.driver_config.transport_config.i2c_config.pin_config.sda = DPT::B8;
    disp_cfg.driver_config.transport_config.i2c_address = 0x3c;
    display.Init(disp_cfg);

    while(1)
    {
        now = System::GetNow();
        // 500Hz samplerate for DAC output test
        if(now - dact > 2)
        {
            hw.WriteCvOut(CV_OUT_1, dacphs.Process() * 5.f, false);
            dact = now;
        }

        if(now - usbt > 100)
        {
            hw.PrintLine("Streaming Test Results");
            hw.PrintLine("######################");
            hw.PrintLine("Analog Inputs:");
            for(int i = 0; i < ADC_LAST; i++)
            {
                hw.Print("%s_%d: " FLT_FMT3,
                         i < ADC_9 ? "CV" : "ADC",
                         i + 1,
                         FLT_VAR3(hw.GetAdcValue(i)));
                if(i != 0 && (i + 1) % 4 == 0)
                    hw.Print("\n");
                else
                    hw.Print("\t");
            }
            hw.PrintLine("######################");
            hw.PrintLine("Digital Inputs:");
            hw.Print("Button: %s\t", button.Pressed() ? "ON" : "OFF");
            hw.Print("Toggle: %s\t", toggle.Pressed() ? "UP" : "DOWN");
            hw.Print("\nGATE_IN_1: %s\t",
                     hw.gate_in_1.State() ? "HIGH" : "LOW");
            hw.PrintLine("GATE_IN_2: %s",
                         hw.gate_in_2.State() ? "HIGH" : "LOW");
            hw.PrintLine("######################");
            usbt = now;
            //MIDISendNoteOn(1, 50, 100);
        }

        hw.midi.Listen();

        if(hw.midi.HasEvents()) {
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

        /** short 60ms blip off on builtin LED */
        hw.SetLed((now & 2047) > 60);

        snprintf(outout, 64, "r %d sdm %d fr %d", roro, sdmmc_pass ? 1 : 0, fres);

        display.Fill(false);
        display.SetCursor(0,0);
        display.WriteString(outout, Font_6x8, true);
        display.Update();

        hw.Delay(50);
    }
}

extern "C" void TIM5_IRQHandler(void) {
    hw.tim5.Instance->SR = 0;
    /*
    if(!patch.dac_exp.lock) {
        patch.dac_exp.WriteDac7554();
    }
    */
}
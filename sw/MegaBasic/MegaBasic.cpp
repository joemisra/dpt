/*
 * MegaBasic MIDI Polysynth for dsp.coffee DPT
 *
 * Was basic; now? not so much.
 * 
 * 2022 - Joseph Misra
 * 
 */

#include "daisysp.h"
#include "../../lib/daisy_dpt.h"
#include "SaucyVoice.h"

using namespace daisy;
using namespace dpt;
using namespace daisysp;

DPT hw;

SaucyVoice oscillators[6];

int currVoice = 0;

uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Convert MIDI note number to CV
// DAC outputs from 0 to 3v3 (3300 mV’s)
// We have 1V/Oct, so 3+ octaves on a 3v3 system
// The DAC wants values from 0 to 4095
float mtocv(uint8_t pitch)
{
    float voltsPerNote = 0.0833f; // 1/12 V
    float mV;                     // from 0 to 0xFFF (4095);

    mV = 1000 * (pitch * voltsPerNote);

    return abs(15.0 - (pitch * voltsPerNote));
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float fade;
    hw.ProcessAllControls();

    for(int i = 0; i < 6; i++) {
        oscillators[i].oscillator.SetPW(hw.GetAdcValue(dpt::CV_1));
        oscillators[i].oscillator.SetWaveshape(hw.GetAdcValue(dpt::CV_2));
        oscillators[i].zosc.SetShape(hw.GetAdcValue(dpt::CV_1));
        oscillators[i].zosc.SetFormantFreq(hw.GetAdcValue(dpt::CV_2) * 1000.);
        oscillators[i].zosc.SetMode(hw.GetAdcValue(dpt::CV_3));
        //oscillators[i].randdetune = hw.GetAdcValue(dpt::CV_5);
        oscillators[i].envelope.SetTime(2, abs(hw.GetAdcValue(dpt::CV_2)));
        oscillators[i].vibratooo.SetAmp(hw.GetAdcValue(dpt::CV_6) * 10.f);
        oscillators[i].vibratooo.SetFreq(hw.GetAdcValue(dpt::CV_7) * 30.f);
        fade = hw.GetAdcValue(dpt::CV_8);
        oscillators[i].SetFade(fade);
    }

    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = (oscillators[0].Process() + oscillators[1].Process()) * 0.5;
        out[1][i] = (oscillators[2].Process() + oscillators[3].Process()) * 0.5;
        /*
        out[1][i] = oscillators[1].Process() * -1;
        out[0][i] = oscillators[2].Process() * -1;
        out[1][i] = oscillators[3].Process();
        out[0][i] = oscillators[4].Process() * -1;
        out[1][i] = oscillators[5].Process();
        */
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioSampleRate(48000);
    hw.SetAudioBlockSize(48);
    float samplerate = hw.AudioSampleRate();

    for(int i = 0; i < 6; i++) {
         oscillators[i].Init(samplerate * 2.f, i);
    }

    uint32_t now, dact, usbt, gatet;
    now = dact = usbt = System::GetNow();
    gatet             = now;

    hw.midi.StartReceive();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.midi.Listen();

        while(hw.midi.HasEvents()) {
            auto event = hw.midi.PopEvent();

            // Basic MIDI -> CV, and forwards note on/off to MIDI
            if(event.type  == MidiMessageType::NoteOn) {
                auto e = event.AsNoteOn();
                dsy_gpio_write(&hw.gate_out_1, 1);
                hw.MIDISendNoteOn(e.channel, e.note, e.velocity);
                hw.WriteCvOut(CV_OUT_1, mtocv(e.note), false);
                
                if(e.channel == 0) {
                    if(currVoice == 4) currVoice = 0;
                    oscillators[currVoice].TrigMidi(e.note, e.velocity);
                    currVoice++;
                }
            }
            else if(event.type  == MidiMessageType::NoteOff) {
                auto e = event.AsNoteOff();
                dsy_gpio_write(&hw.gate_out_1, 0);
                hw.MIDISendNoteOff(e.channel, e.note, e.velocity);
            }
            else if(event.type == MidiMessageType::ControlChange) {
                auto e = event.AsControlChange();
                hw.WriteCvOut(CV_OUT_2, ((float)e.value / 127.) * 5.f, false);
            }
        } 
        hw.Delay(10);
    }
}


// Interrupt for updating DAC7554
/*
extern "C" void TIM5_IRQHandler(void) {
    hw.tim5.Instance->SR = 0;

    hw.WriteCvOutExp(
        oscillators[0].Process() * 5.,
        oscillators[1].Process() * 5.,
        oscillators[2].Process() * 5.,
        oscillators[3].Process() * 5.,
        false
    );

    //hw.WriteCvOut(1, oscillators[4].Process() * 5.f, false);
    //hw.WriteCvOut(0, oscillators[5].Process() * 5.f, false);
    
    if(!hw.dac_exp.lock) {
        hw.dac_exp.WriteDac7554();
    }
}
*/
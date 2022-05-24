/*
 * MegaBasic MIDI Polysynth for dsp.coffee DPT
 *
 * Notes are doubled on DAC7554 outputs
 * 
 * 2022 - Joseph Misra
 * 
 */

#include "daisysp.h"
#include "../../lib/daisy_dpt.h"
#include "SaucyVoice.h"

#define MAX_VOICES 8

using namespace daisy;
using namespace dpt;
using namespace daisysp;

DPT patch;

SaucyVoice oscillators[8];

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
    patch.ProcessAllControls();

    for(int i = 0; i < MAX_VOICES; i++) {
        oscillators[i].oscillator.SetPW(patch.GetAdcValue(dpt::CV_1));
        oscillators[i].oscillator.SetWaveshape(patch.GetAdcValue(dpt::CV_2));
        oscillators[i].zosc.SetShape(patch.GetAdcValue(dpt::CV_1));
        oscillators[i].zosc.SetFormantFreq(patch.GetAdcValue(dpt::CV_2) * 1000.);
        oscillators[i].zosc.SetMode(patch.GetAdcValue(dpt::CV_3));
        oscillators[i].envelope.SetTime(2, abs(patch.GetAdcValue(dpt::CV_2)));
        oscillators[i].vibratooo.SetAmp(patch.GetAdcValue(dpt::CV_6) * 10.f);
        oscillators[i].vibratooo.SetFreq(patch.GetAdcValue(dpt::CV_7) * 30.f);
        fade = patch.GetAdcValue(dpt::CV_8);
        oscillators[i].SetFade(fade);
    }

    // rendering these
    oscillators[4].Process();
    oscillators[5].Process();
    oscillators[6].Process();
    oscillators[7].Process();

    for(size_t i = 0; i < size; i++)
    {
            out[0][i] = (oscillators[0].Process() + oscillators[1].Process()) * 0.5;
            out[1][i] = (oscillators[2].Process() + oscillators[3].Process()) * 0.5;
    }
}

void dac7554handler(void *data) {
    patch.WriteCvOutExp(
        oscillators[4].last,
        oscillators[5].last,
        oscillators[6].last,
        oscillators[7].last,
        false);
}

int main(void)
{
    float samplerate = 48000;
    patch.Init();
    patch.SetAudioSampleRate(samplerate);
    patch.SetAudioBlockSize(1); // must be 1 to match main callback

    for(int i = 0; i < 8; i++) {
         oscillators[i].Init(samplerate * 2, i);
    }

    patch.StartAudio(AudioCallback);
    patch.InitTimer(dac7554handler, nullptr);

    patch.midi.StartReceive();

    while(1)
    {
        patch.midi.Listen();

        while(patch.midi.HasEvents()) {
            auto event = patch.midi.PopEvent();

            // Basic MIDI -> CV, and forwards note on/off to MIDI
            if(event.type  == MidiMessageType::NoteOn) {
                auto e = event.AsNoteOn();
                dsy_gpio_write(&patch.gate_out_1, 1);
                patch.MIDISendNoteOn(e.channel, e.note, e.velocity);
                patch.WriteCvOut(CV_OUT_1, mtocv(e.note), false);
                
                if(e.channel == 0) {
                    oscillators[currVoice].TrigMidi(e.note, e.velocity);
                    oscillators[currVoice+4].TrigMidi(e.note, e.velocity);
                    if(++currVoice == 4) currVoice = 0;
                }
            }
            else if(event.type  == MidiMessageType::NoteOff) {
                auto e = event.AsNoteOff();
                dsy_gpio_write(&patch.gate_out_1, 0);
                patch.MIDISendNoteOff(e.channel, e.note, e.velocity);
            }
            else if(event.type == MidiMessageType::ControlChange) {
                auto e = event.AsControlChange();
                patch.WriteCvOut(CV_OUT_2, ((float)e.value / 127.) * 5.f, false);
            }
        } 
        patch.Delay(10);
    }
}


// Interrupt for updating DAC7554
/*
extern "C" void TIM5_IRQHandler(void) {

    //patch.WriteCvOut(1, oscillators[4].Process() * 5.f, false);
    //patch.WriteCvOut(0, oscillators[5].Process() * 5.f, false);
    
    if(!patch.dac_exp.lock) {
        patch.dac_exp.WriteDac7554();
    }
}
*/
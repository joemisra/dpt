#include "daisy.h"
#include "daisysp.h"
#include "../../lib/daisy_dpt.h"

/*
    Hello, friend. Here is a minimal, normal DPT template w/ a few notes / examples.

    We hope you enjoy your time with DPT and Daisy Patch SM!

    ~ j.m.
*/

using namespace daisy;
using namespace daisysp;
using namespace dpt;

DPT patch;

void dac7554callback(void *data) {
    /*
        Update CV 3-6 here, like this (voltages -7.0 to 7.0)
            patch.WriteCvOutExp(
                1.0,
                2.0,
                3.0,
                4.0,
                false);
            );

            or for raw values, 
            patch.WriteCvOutExp(
                0,
                1023,
                2047
                4095,
                true);
            );
        */
}

void AudioCallback(AudioHandle::InputBuffer in,
                       AudioHandle::OutputBuffer out,
                       size_t size)
{
    patch.ProcessAllControls();

    /*
        CV 1 - 8 accessed
            patch.controls[CV_1].Value()

        Gate ins acccessed

            patch.gate_in_1.State();
            patch.gate_in_2.State();

        Send data to gate outs

            dsy_gpio_write(&patch.gate_out_1, 1);
            dsy_gpio_write(&patch.gate_out_2, 2);

        Send data to CV 1 and 2 (0 is both, currently it seems 1 is CV2, 2 is CV1)
            patch.WriteCvOut(0, 5.0, false);
            patch.WriteCvOut(0, [0 - 4095], true); // last argument is 'raw', send raw 12-bit data
        
    */


    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}

int main(void)
{
    patch.Init();

    // Set up callback for TIM5

    patch.StartAudio(AudioCallback);
    patch.InitTimer(dac7554callback, nullptr);

    // patch.midi.StartReceieve()

    while(1)
    {
        /*
            MIDI processing here

            patch.midi.Listen()
            first then you can call

            patch.midi.HasEvents()
            and
            auto event = patch.midi.PopEvent();

            and sort that out from there

            if(event.type == MidiMessageType::NoteOn) {
                ...
            };
        */
    }
}
/*
 * Basic Inpujian Chippp Voiceee
 *
 * 2022 - Joseph Misra
 */

#include "daisysp.h"
#include "../../lib/daisy_dpt.h"
#include "SaucyVoice.h"

using namespace daisy;
using namespace dpt;
using namespace daisysp;

void SaucyVoice::Init(float samplerate, int _index)
{
    oscillator.Init(samplerate);
    zosc.Init(samplerate);
    vibratooo.Init(samplerate);
    cf.Init();

    index = _index;

    envelope.Init(samplerate);

    oscillator.SetFreq(60);
    oscillator.SetPW(0.5);

    envelope.SetMin(0.0);
    envelope.SetMax(1.0);
    //
    envelope.SetTime(0, 0.0);  // idle
    envelope.SetTime(1, 0.01); // attack
    envelope.SetTime(2, 0.64); // decay

    vibratooo.SetAmp(5);
    vibratooo.SetFreq(2);

    cf.SetPos(0.5);
    cf.SetCurve(CROSSFADE_LIN);
    // envelope.SetTime(3, 0.0); // decay
}

void SaucyVoice::SetFade(float fade)
{
    cf.SetPos(abs(fade));
}

float SaucyVoice::Process()
{
    float z2;
    zosc.SetFreq(f);
    oscillator.SetFreq(f);
    z2 = oscillator.Process();
    last = z2 * envelope.Process();

    return last;
}

void SaucyVoice::Trig()
{
    envelope.Trigger();
}
void SaucyVoice::TrigMidi(int note, int velocity)
{
    f = mtof(note);

    oscillator.SetFreq(f);
    zosc.SetFreq(f);
    envelope.SetMax(velocity / 127.);
    envelope.Trigger();
}

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

    zosc.SetMode(0.5);
    zosc.SetShape(0.5);

    vibratooo.SetAmp(5);
    vibratooo.SetFreq(2);

    cf.SetPos(0.5);
    cf.SetCurve(CROSSFADE_CPOW);
    // envelope.SetTime(3, 0.0); // decay
}

void SaucyVoice::SetFade(float fade)
{
    cf.SetPos(abs(fade));
}

float SaucyVoice::Process()
{
    float z1, z2, vib;
    vib = vibratooo.Process();
    zosc.SetFreq(f + vib);
    z1 = zosc.Process();
    oscillator.SetFreq(f + vib);
    z2 = oscillator.Process();

    return cf.Process(z1, z2) * envelope.Process();
}

void SaucyVoice::Trig()
{
    envelope.Trigger();
}
void SaucyVoice::TrigMidi(int note, int velocity)
{
    if (index >= 4)
        note -= 24;

    f = mtof(note);

    oscillator.SetFreq(f);
    zosc.SetFreq(f);
    envelope.SetMax(velocity / 127.);
    envelope.Trigger();
}

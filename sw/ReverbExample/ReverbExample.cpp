#include "../../lib/daisy_dpt.h"
#include "daisysp.h"


using namespace daisy;
using namespace daisysp;
using namespace dpt;

DPT patch;
ReverbSc    reverb;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    patch.ProcessAnalogControls();

    /** Update Params with the four knobs */
    float time_knob = patch.controls[CV_1].Value();
    float time      = fmap(time_knob, 0.3f, 0.99f, Mapping::LINEAR);
    float damp_knob = patch.controls[CV_2].Value();
    float damp      = fmap(damp_knob, 1000.f, 19000.f, Mapping::LOG);
    float in_level = patch.controls[CV_3].Value();
    float send_level = patch.controls[CV_4].Value();

    reverb.SetFeedback(time);
    reverb.SetLpFreq(damp);

    for(size_t i = 0; i < size; i++)
    {
        float dryl  = IN_L[i] * in_level;
        float dryr  = IN_R[i] * in_level;
        float sendl = IN_L[i] * send_level;
        float sendr = IN_R[i] * send_level;
        float wetl, wetr;

        reverb.Process(sendl, sendr, &wetl, &wetr);

        OUT_L[i] = dryl + wetl;;
        OUT_R[i] = dryr + wetr;
    }
}

int main(void)
{
    float samplerate = 48000;
    patch.Init();

    reverb.Init(samplerate);
    patch.StartAudio(AudioCallback);

    while(1) {}
}

#include "daisysp.h"
#include "../../lib/daisy_dpt.h"

using namespace daisy;
using namespace dpt;
using namespace daisysp;

class SaucyVoice {
    public:
        VariableSawOscillator oscillator;
        ZOscillator zosc;
        CrossFade cf;
        Oscillator vibratooo;
        float f;
        int index;
        
        AdEnv envelope;

        float randdetune;

        void Init(float samplerate, int _index);

        void SetFade(float fade); 

        float Process();

        void Trig();

        void TrigMidi(int note, int velocity); 
};
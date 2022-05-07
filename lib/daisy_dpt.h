#pragma once

#include "daisy.h"

#ifndef DSY_DEV_DAC_7554_H
#include "dev/DAC7554.h"
#endif

namespace daisy
{
namespace dpt
{
    /** Accessors for the Analog Controls. 
     *  These cover the 8x Bipolar CV inputs
     *  as well as the 4x 0-3V3 ADC inputs on
     *  the hardware 
     *. 
     *  When reading a value with DPT::GetAdcValue()
     * 
     *  patch.GetAdcValue(patch_sm::CV_1);
     */
    enum class MappingFmap
    {
        LINEAR,
        LINEAR_INVERTED,
        EXP,
        LOG,
    };

    enum
    {
        CV_1 = 0,
        CV_2,
        CV_3,
        CV_4,
        CV_5,
        CV_6,
        CV_7,
        CV_8,
        ADC_9,
        ADC_10,
        ADC_11,
        ADC_12,
        ADC_LAST,
    };

    /** Enum for addressing the CV Outputs via the WriteCvOut function. */
    enum
    {
        CV_OUT_BOTH = 0,
        CV_OUT_1,
        CV_OUT_2,
    };

    inline float fmin(float a, float b)
    {
        float r;
    #ifdef __arm__
        asm("vminnm.f32 %[d], %[n], %[m]" : [d] "=t"(r) : [n] "t"(a), [m] "t"(b) :);
    #else
        r = (a < b) ? a : b;
    #endif // __arm__
        return r;
    }

    inline float fmax(float a, float b)
    {
        float r;
    #ifdef __arm__
        asm("vmaxnm.f32 %[d], %[n], %[m]" : [d] "=t"(r) : [n] "t"(a), [m] "t"(b) :);
    #else
        r = (a > b) ? a : b;
    #endif // __arm__
        return r;
    }

    inline float fclamp(float in, float min, float max)
    {
        return fmin(fmax(in, min), max);
    }

    inline float
    fmap(float in, float min, float max, MappingFmap curve = MappingFmap::LINEAR)
    {
        switch(curve)
        {
            case MappingFmap::EXP:
                return fclamp(min + (in * in) * (max - min), min, max);
            case MappingFmap::LOG:
            {
                const float a = 1.f / log10f(max / min);
                return fclamp(min * powf(10, in / a), min, max);
            }
            case MappingFmap::LINEAR:
            default: return fclamp(min + in * (max - min), min, max);
        }
    }


    /** @brief Board support file for DPT hardware
     *  @author shensley
     *  @ingroup boards
     * 
     *  Daisy Patch SM is a complete Eurorack module DSP engine.
     *  Based on the Daisy Seed, with circuits added for 
     *  interfacing directly with eurorack modular synthesizers.
     */
    class DPT
    {
      public:
        /** Helper for mapping pins, and accessing them using the `GetPin` function */
        enum class PinBank
        {
            A,
            B,
            C,
            D
        };

        DPT() {}
        ~DPT() {}

        /** Initializes the memories, and core peripherals for the Daisy Patch SM */
        void Init();

        void InitMidi();
    
        void InitTimer(daisy::TimerHandle::PeriodElapsedCallback cb, void *data);

        /** Starts a non-interleaving audio callback */
        void StartAudio(AudioHandle::AudioCallback cb);

        /** Starts an interleaving audio callback */
        void StartAudio(AudioHandle::InterleavingAudioCallback cb);

        /** Changes the callback that is executing.
         *  This may cause clicks if done while audio is processing.
         */
        void ChangeAudioCallback(AudioHandle::AudioCallback cb);

        /** Changes the callback that is executing.
         *  This may cause clicks if done while audio is processing.
         */
        void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

        /** Stops the transmission of audio. */
        void StopAudio();

        /** Sets the number of samples processed in an audio callback. 
         *  This will only take effect on the next invocation of `StartAudio`
         */
        void SetAudioBlockSize(size_t size);

        /** Sets the samplerate for the audio engine 
         *  This will set it to the closest valid samplerate. Options being:
         *  8kHz, 16kHz, 32kHz, 48kHz, and 96kHz
         */
        void SetAudioSampleRate(float sr);

        void SetAudioSampleRate(SaiHandle::Config::SampleRate sample_rate);

        /** Returns the number of samples processed in an audio callback */
        size_t AudioBlockSize();

        /** Returns the audio engine's samplerate in Hz */
        float AudioSampleRate();

        /** Returns the rate at which the audio callback will be called in Hz */
        float AudioCallbackRate();

        /** Starts the Control ADCs 
         * 
         *  This is started automatically when Init() is called.
         */
        void StartAdc();

        /** Stops the Control ADCs */
        void StopAdc();

        /** Reads and filters all of the analog control inputs */
        void ProcessAnalogControls();

        /** Reads and debounces any of the digital control inputs 
         *  This does nothing on this board at this time.
         */
        void ProcessDigitalControls();

        /** Does both of the above */
        void ProcessAllControls()
        {
            ProcessAnalogControls();
            ProcessDigitalControls();
        }

        /** Returns the current value for one of the ADCs */
        float GetAdcValue(int idx);

        /** Returns the STM32 port/pin combo for the desired pin (or an invalid pin for HW only pins)
         *
         *  Macros at top of file can be used in place of separate arguments (i.e. GetPin(A4), etc.)
         * 
         *  \param bank should be one of the PinBank options above
         *  \param idx pin number between 1 and 10 for each of the pins on each header.
         */
        dsy_gpio_pin GetPin(const PinBank bank, const int idx);

        /** Starts the DAC for the CV Outputs 
         * 
         *  By default this starts by running the 
         *  internal callback at 48kHz, which will 
         *  update the values based on the SetCvOut 
         *  function.
         * 
         *  This is started automatically when Init() is called.
         */
        void StartDacExp(DacHandle::DacCallback callback = nullptr);

        void StartDac(DacHandle::DacCallback callback = nullptr);

        /** Stop the DAC from updating. 
         *  This will suspend the CV Outputs from changing 
         */
        void StopDac();

        /** Sets specified DAC channel to the target voltage. 
         *  This may not be 100% accurate without calibration. 
         *  
         *  \todo Add Calibration to CV Outputs
         * 
         *  \param channel desired channel to update. 0 is both, otherwise 1 or 2 are valid.
         *  \param volage value in Volts that you'd like to write to the DAC. The valid range is -5-10V.
         *  \param raw if true, voltage value is passed directly to the dac [0-4095]
         */
        void WriteCvOut(const int channel, float voltage, bool raw);

        /** Sets expander channels to the target voltage + write. 
         *  This may not be 100% accurate without calibration. 
         *  
         *  \todo Add Calibration to CV Outputs
         * 
         *  \param a v for output 1 
         *  \param b v for output 2
         *  \param c v for output 3
         *  \param d v for output 4
         *  \param raw if true, abcd are passed directly to the dac [0-4095]
         */
        void WriteCvOutExp(float a, float b, float c, float d, bool raw);

        
        /** Convert -8 to 8 range to 4096 */
        uint16_t VoltageToCodeExp(float input);

        /** Here are some wrappers around libDaisy Static functions 
         *  to provide simpler syntax to those who prefer it. */

        /** Delays for a specified number of milliseconds */
        inline void Delay(uint32_t milliseconds)
        {
            System::Delay(milliseconds);
        }

        /** Gets a random 32-bit value */
        inline uint32_t GetRandomValue() { return Random::GetValue(); }

        /** Gets a random floating point value between the specified minimum, and maxmimum */
        inline float GetRandomFloat(float min = 0.f, float max = 1.f)
        {
            return Random::GetFloat(min, max);
        }

        void SetLed(bool state);

        /** Print formatted debug log message
         */
        template <typename... VA>
        static void Print(const char* format, VA... va)
        {
            Log::Print(format, va...);
        }

        /** Print formatted debug log message with automatic line termination
         */
        template <typename... VA>
        static void PrintLine(const char* format, VA... va)
        {
            Log::PrintLine(format, va...);
        }

        /** Start the logging session. Optionally wait for terminal connection before proceeding.
         */
        static void StartLog(bool wait_for_pc = false)
        {
            Log::StartLog(wait_for_pc);
        }

        // midi helping stuff yoinked from makingsoundmachines @ electrosmith forum
        void MIDISendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
        {
            uint8_t data[3] = {0};

            data[0] = (channel & 0x0F) + 0x90; // limit channel byte, add status byte
            data[1] = note & 0x7F;             // remove MSB on data
            data[2] = velocity & 0x7F;

            midi.SendMessage(data, 3);
        }

        void MIDISendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
        {
            uint8_t data[3] = {0};

            data[0] = (channel & 0x0F) + 0x80; // limit channel byte, add status byte
            data[1] = note & 0x7F;             // remove MSB on data
            data[2] = velocity & 0x7F;

            midi.SendMessage(data, 3);
        }

        /** @brief Tests entirety of SDRAM for validity 
         *         This will wipe contents of SDRAM when testing. 
         * 
         *  @note   If using the SDRAM for the default bss, or heap, 
         *          and using constructors as initializers do not 
         *          call this function. Otherwise, it could 
         *          overwrite changes performed by constructors.
         * 
         *  \retval returns true if SDRAM is okay, otherwise false
         */
        bool ValidateSDRAM();

        /** @brief Tests the QSPI for validity 
         *         This will wipe contents of QSPI when testing. 
         * 
         *  @note  If called with quick = false, this will erase all memory
         *         the "quick" test starts 0x400000 bytes into the memory and
         *         test 16kB of data
         * 
         *  \param quick if this is true the test will only test a small piece of the QSPI
         *               checking the entire 8MB can take roughly over a minute.
         * 
         *  \retval returns true if SDRAM is okay, otherwise false
         */
        bool ValidateQSPI(bool quick = true);

        /** Direct Access Structs/Classes */
        System      system;
        SdramHandle sdram;
        QSPIHandle  qspi;
        AudioHandle audio;
        AdcHandle   adc;
        UsbHandle   usb;
        Pcm3060     codec;
        DacHandle   dac;
        MidiUartHandler midi;
        MidiUsbHandler usb_midi;

        Dac7554     dac_exp;
        
        /** Dedicated Function Pins */
        dsy_gpio      user_led;
        AnalogControl controls[ADC_LAST];
        GateIn        gate_in_1, gate_in_2;
        dsy_gpio      gate_out_1, gate_out_2;

        /** Pin Accessors for the DPT hardware
         *  Used for initializing various GPIO, etc.
         */
        static const dsy_gpio_pin A1, A2, A3, A4, A5;
        static const dsy_gpio_pin A6, A7, A8, A9, A10;
        static const dsy_gpio_pin B1, B2, B3, B4, B5;
        static const dsy_gpio_pin B6, B7, B8, B9, B10;
        static const dsy_gpio_pin C1, C2, C3, C4, C5;
        static const dsy_gpio_pin C6, C7, C8, C9, C10;
        static const dsy_gpio_pin D1, D2, D3, D4, D5;
        static const dsy_gpio_pin D6, D7, D8, D9, D10;
        class Impl;

        TIM_HandleTypeDef tim5;

      private:
        using Log = Logger<LOGGER_INTERNAL>;

        float callback_rate_;

        /** Background callback for updating the DACs. */
        Impl* pimpl_;
    };

} // namespace patch_sm

} // namespace daisy

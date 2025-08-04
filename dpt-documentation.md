# DPT (Daisy Patch Toolkit) Module Documentation

## Overview

### Purpose
The DPT (Daisy Patch Toolkit) is a specialized Eurorack development module designed to expand the capabilities of the Daisy Patch Submodule ecosystem. It targets audio developers, synthesizer builders, and embedded system engineers who require advanced I/O capabilities and prototyping flexibility.

### Design Philosophy
Developed as an extension of the Daisy Patch platform, the DPT module provides a robust, flexible interface for complex audio and control applications. It accommodates all available pins on the Daisy Patch Submodule, enabling developers to fully utilize the platform's capabilities for advanced projects.

## Hardware Specifications

### Power System
- **Power Standard**: Eurorack ±12V
  - Negative Rail: -12V
  - Positive Rail: +12V
  - Current Draw: TBD (dependent on usage)
- **Voltage Regulation**: On-board conversion to +5V and +3.3V for digital systems

### Core Specifications
- **Platform**: Electrosmith Daisy Patch Submodule (SM)
- **Microcontroller**: STM32H750 ARM Cortex-M7
- **Clock Speed**: 480 MHz
- **Memory**:
  - Internal RAM: 512 KB
  - External SDRAM: 64 MB
  - QSPI Flash: 16 MB

### Audio System
- **Codec**: PCM3080 (high-fidelity stereo codec)
- **Audio Resolution**: 24-bit @ 48kHz
- **Internal Processing**: 32-bit floating point
- **Input/Output**: Stereo balanced

### Detailed I/O Capabilities

#### Analog Inputs
- **Potentiometers**: 8 × 9mm potentiometers
  - Normalled to 0V - +5V range
  - 10kΩ linear taper
- **CV Inputs**: 8 × CV inputs
  - Breaks normalled connection when patched
  - Potentiometer acts as attenuator when CV is present
  - Input protection via 1N1517 diodes
  - 12-bit ADC resolution

#### Control Voltage Outputs
- **Daisy Native CV Outputs**: 2 × 12-bit CV channels
  - Output Range: -5V to +10V
  - Direct from Daisy Patch SM DAC
- **External DAC Outputs**: 4 × 12-bit CV channels
  - IC: DAC7554IDGS (SPI interface)
  - Output Range: -7V to +7V
  - Can be used for low-fi audio with modifications
  - Lowpass filtered to reduce aliasing

#### Gate I/O
- **Gate Inputs**: 2 × digital gate inputs
  - 5V logic level
  - Input protection circuitry
- **Gate Outputs**: 2 × digital gate outputs
  - 5V logic level
  - LED indication per output

#### Digital Connectivity
- **MIDI**: TRS MIDI Input/Output
  - Type A/B auto-sensing on input (LPZW circuit)
  - Optically isolated input (HCPL-0631)
  - 3.3V to 5V level shifting
- **SD Card**: Vertical MicroSD card slot
  - SPI interface
  - FAT32 filesystem support
- **Expansion Header**: 16-pin header providing:
  - +5V power
  - +3.3V power
  - SPI bus
  - UART
  - I2C
  - USB connections

### Visual Feedback
- **LEDs**: 
  - 6 × Bidirectional Red/Green LEDs for CV/Gate indication
  - 2 × Single color LEDs for status

### Operational Amplifiers
- **Op-Amps**: 6 × TL072 (SOIC-8 package)
  - Used for CV scaling and buffering
  - Input/output protection

## Module Dimensions
- **Format**: Eurorack 3U
- **Width**: 20HP
- **Depth**: ~35mm (with Daisy Patch SM installed)

## Software Architecture

### Development Environment
- **Framework**: libDaisy
- **DSP Library**: DaisySP
- **Language**: C++
- **Build System**: Make / PlatformIO
- **Toolchain**: ARM GCC Embedded

### Code Structure
```cpp
// Basic DPT initialization and usage
#include "daisy_dpt.h"

DPT dpt;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size) {
    // Process audio and CV
    for (size_t i = 0; i < size; i += 2) {
        // Read inputs
        float pot_value = dpt.ReadPot(0);
        float cv_in = dpt.ReadCV(0);
        
        // Process audio
        out[i] = in[i] * pot_value;      // Left
        out[i+1] = in[i+1] * pot_value;  // Right
        
        // Update CV outputs
        dpt.SetCVOutput(0, pot_value * 5.0f);
    }
}

int main(void) {
    // Initialize hardware
    dpt.Init();
    
    // Configure audio callback
    dpt.SetAudioBlockSize(48);
    dpt.StartAudio(AudioCallback);
    
    // Main loop
    while(1) {
        // Handle non-audio rate tasks
        dpt.ProcessMidiEvents();
        dpt.UpdateLEDs();
    }
}
```

## Example Applications

### 1. MIDI Processing
```cpp
// midi_test.cpp - Comprehensive MIDI handling example
#include "daisy_dpt.h"

DPT dpt;

// MIDI event handler callback
void HandleMidiMessage(MidiEvent m) {
    switch(m.type) {
        case NoteOn:
            // Extract MIDI data
            uint8_t note = m.data[0];      // Note number (0-127)
            uint8_t velocity = m.data[1];   // Velocity (0-127)
            
            // Convert MIDI note to frequency
            float freq = mtof(note);
            
            // Scale velocity to CV range (0-5V)
            float cv_out = (velocity / 127.0f) * 5.0f;
            dpt.SetCVOutput(0, cv_out);
            
            // Trigger gate output
            dpt.SetGate(0, true);
            break;
            
        case NoteOff:
            // Release gate on note off
            dpt.SetGate(0, false);
            break;
            
        case ControlChange:
            uint8_t cc_num = m.data[0];
            uint8_t cc_val = m.data[1];
            
            // Map specific CC to CV output
            if(cc_num == 1) {  // Mod wheel
                float mod_cv = (cc_val / 127.0f) * 5.0f;
                dpt.SetCVOutput(1, mod_cv);
            }
            break;
            
        case ProgramChange:
            // Handle program changes
            uint8_t program = m.data[0];
            // Load preset, change algorithm, etc.
            break;
            
        case PitchBend:
            // Extract 14-bit pitch bend value
            int16_t bend = (m.data[1] << 7) | m.data[0];
            float bend_cv = ((bend - 8192) / 8192.0f) * 2.0f; // ±2V
            dpt.SetCVOutput(2, bend_cv + 2.5f); // Center at 2.5V
            break;
    }
}

void InitializeMidi() {
    // Configure MIDI settings
    MidiUartHandler::Config midi_config;
    midi_config.transport_config.rx = dpt.GetMidiRxPin();
    midi_config.transport_config.tx = dpt.GetMidiTxPin();
    
    dpt.midi.Init(midi_config);
    dpt.midi.StartReceive();
}

int main(void) {
    dpt.Init();
    InitializeMidi();
    
    while(1) {
        // Process all pending MIDI messages
        dpt.midi.Listen();
        while(dpt.midi.HasEvents()) {
            HandleMidiMessage(dpt.midi.PopEvent());
        }
        
        // Update gate LEDs
        dpt.SetLED(0, dpt.gate[0].State());
        dpt.SetLED(1, dpt.gate[1].State());
    }
}
```

### 2. CV Generation and DAC Control
```cpp
// cv_test.cpp - Advanced CV generation with DAC7554
#include "daisy_dpt.h"
#include <cmath>

DPT dpt;

// LFO generator class
class LFO {
private:
    float phase = 0.0f;
    float frequency = 1.0f;
    
public:
    enum Waveform { SINE, TRIANGLE, SQUARE, SAW };
    Waveform waveform = SINE;
    
    void SetFrequency(float freq) { frequency = freq; }
    
    float Process(float sample_rate) {
        phase += frequency / sample_rate;
        if(phase >= 1.0f) phase -= 1.0f;
        
        switch(waveform) {
            case SINE:
                return sinf(phase * TWOPI);
            case TRIANGLE:
                return (phase < 0.5f) ? 
                    (phase * 4.0f - 1.0f) : 
                    (3.0f - phase * 4.0f);
            case SQUARE:
                return (phase < 0.5f) ? 1.0f : -1.0f;
            case SAW:
                return phase * 2.0f - 1.0f;
        }
        return 0.0f;
    }
};

// Envelope generator
class ADSR {
private:
    enum Stage { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };
    Stage stage = IDLE;
    float output = 0.0f;
    float attack_rate, decay_rate, sustain_level, release_rate;
    
public:
    void SetAttack(float time_ms) { 
        attack_rate = 1.0f / (time_ms * 0.001f * 48000.0f); 
    }
    void SetDecay(float time_ms) { 
        decay_rate = 1.0f / (time_ms * 0.001f * 48000.0f); 
    }
    void SetSustain(float level) { 
        sustain_level = level; 
    }
    void SetRelease(float time_ms) { 
        release_rate = 1.0f / (time_ms * 0.001f * 48000.0f); 
    }
    
    void Gate(bool on) {
        if(on && stage == IDLE) {
            stage = ATTACK;
        } else if(!on && stage != IDLE) {
            stage = RELEASE;
        }
    }
    
    float Process() {
        switch(stage) {
            case ATTACK:
                output += attack_rate;
                if(output >= 1.0f) {
                    output = 1.0f;
                    stage = DECAY;
                }
                break;
            case DECAY:
                output -= decay_rate;
                if(output <= sustain_level) {
                    output = sustain_level;
                    stage = SUSTAIN;
                }
                break;
            case SUSTAIN:
                // Hold at sustain level
                break;
            case RELEASE:
                output -= release_rate;
                if(output <= 0.0f) {
                    output = 0.0f;
                    stage = IDLE;
                }
                break;
        }
        return output;
    }
};

// Global objects
LFO lfo[4];
ADSR envelope[2];

void ConfigureDAC() {
    // Initialize DAC7554 with specific settings
    dpt.dac.Init(dpt.spi_handle, dpt.dac_cs_pin);
    
    // Set reference voltage and power settings
    dpt.dac.SetPowerDown(false);
    
    // Configure output ranges for each channel
    for(int i = 0; i < 4; i++) {
        // Channel configuration handled by hardware scaling
        // Software outputs 0-1 normalized values
    }
}

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size) {
    for(size_t i = 0; i < size; i += 2) {
        // Read controls
        float lfo_rate = dpt.ReadPot(0) * 10.0f;  // 0-10 Hz
        float env_attack = dpt.ReadPot(1) * 1000.0f;  // 0-1000ms
        
        // Update parameters
        lfo[0].SetFrequency(lfo_rate);
        envelope[0].SetAttack(env_attack);
        
        // Process generators
        float lfo_out = lfo[0].Process(48000.0f);
        float env_out = envelope[0].Process();
        
        // Scale and output to DAC channels
        // LFO: Bipolar output (-7V to +7V)
        dpt.SetCVOutput(0, (lfo_out + 1.0f) * 0.5f);  // Normalized 0-1
        
        // Envelope: Unipolar output (0V to +7V)
        dpt.SetCVOutput(1, env_out);
        
        // Use remaining DAC channels for complex modulation
        float complex_mod = lfo_out * env_out;
        dpt.SetCVOutput(2, (complex_mod + 1.0f) * 0.5f);
        
        // Audio pass-through
        out[i] = in[i];
        out[i+1] = in[i+1];
    }
}

int main(void) {
    dpt.Init();
    ConfigureDAC();
    
    // Configure LFOs with different waveforms
    lfo[0].waveform = LFO::SINE;
    lfo[1].waveform = LFO::TRIANGLE;
    lfo[2].waveform = LFO::SQUARE;
    lfo[3].waveform = LFO::SAW;
    
    // Set up envelopes
    envelope[0].SetDecay(100.0f);
    envelope[0].SetSustain(0.7f);
    envelope[0].SetRelease(500.0f);
    
    dpt.SetAudioBlockSize(48);
    dpt.StartAudio(AudioCallback);
    
    while(1) {
        // Check gate inputs for envelope triggering
        envelope[0].Gate(dpt.ReadGate(0));
        envelope[1].Gate(dpt.ReadGate(1));
        
        // Visual feedback
        dpt.UpdateLEDs();
    }
}
```

### 3. Gate Processing and Sequencing
```cpp
// gate_test.cpp - Gate I/O, clock division, and sequencing
#include "daisy_dpt.h"

DPT dpt;

// Clock divider/multiplier
class ClockProcessor {
private:
    int divide_factor = 1;
    int multiply_factor = 1;
    int pulse_count = 0;
    bool last_state = false;
    uint32_t last_pulse_time = 0;
    uint32_t pulse_interval = 0;
    
public:
    void SetDivision(int div) { divide_factor = div; }
    void SetMultiplication(int mult) { multiply_factor = mult; }
    
    bool Process(bool input_gate, uint32_t current_time) {
        bool output = false;
        
        // Detect rising edge
        if(input_gate && !last_state) {
            // Calculate interval between pulses
            if(last_pulse_time > 0) {
                pulse_interval = current_time - last_pulse_time;
            }
            last_pulse_time = current_time;
            
            // Handle division
            pulse_count++;
            if(pulse_count >= divide_factor) {
                output = true;
                pulse_count = 0;
            }
        }
        
        // Handle multiplication (requires pulse interval)
        if(multiply_factor > 1 && pulse_interval > 0) {
            uint32_t sub_interval = pulse_interval / multiply_factor;
            uint32_t time_since_pulse = current_time - last_pulse_time;
            
            for(int i = 1; i < multiply_factor; i++) {
                if(time_since_pulse >= sub_interval * i &&
                   time_since_pulse < sub_interval * i + 10) {  // 10ms pulse
                    output = true;
                }
            }
        }
        
        last_state = input_gate;
        return output;
    }
};

// Simple gate sequencer
class GateSequencer {
private:
    static const int MAX_STEPS = 16;
    bool pattern[MAX_STEPS] = {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
    int current_step = 0;
    int length = 16;
    bool last_clock = false;
    
public:
    void SetPattern(int step, bool value) {
        if(step < MAX_STEPS) pattern[step] = value;
    }
    
    void SetLength(int len) { 
        length = (len > 0 && len <= MAX_STEPS) ? len : MAX_STEPS; 
    }
    
    bool Process(bool clock_input) {
        // Detect clock rising edge
        if(clock_input && !last_clock) {
            current_step = (current_step + 1) % length;
        }
        last_clock = clock_input;
        
        return pattern[current_step];
    }
    
    int GetCurrentStep() { return current_step; }
};

// Burst generator
class BurstGenerator {
private:
    int burst_count = 0;
    int bursts_remaining = 0;
    uint32_t last_burst_time = 0;
    uint32_t burst_interval = 10;  // ms between bursts
    
public:
    void Trigger(int count) {
        burst_count = count;
        bursts_remaining = count;
        last_burst_time = 0;
    }
    
    bool Process(uint32_t current_time) {
        if(bursts_remaining > 0) {
            if(current_time - last_burst_time >= burst_interval) {
                last_burst_time = current_time;
                bursts_remaining--;
                return true;
            }
        }
        return false;
    }
};

// Global objects
ClockProcessor clock_div;
GateSequencer sequencer;
BurstGenerator burst_gen;

int main(void) {
    dpt.Init();
    
    // Configure clock division from pot
    clock_div.SetDivision(4);
    
    // Set up a basic rhythm pattern
    // Kick pattern: X...X...X...X...
    sequencer.SetPattern(0, true);
    sequencer.SetPattern(4, true);
    sequencer.SetPattern(8, true);
    sequencer.SetPattern(12, true);
    
    uint32_t last_time = 0;
    
    while(1) {
        uint32_t current_time = System::GetNow();
        
        // Read external clock on Gate Input 0
        bool external_clock = dpt.ReadGate(0);
        
        // Process clock division
        bool divided_clock = clock_div.Process(external_clock, current_time);
        
        // Run sequencer on divided clock
        bool seq_output = sequencer.Process(divided_clock);
        
        // Trigger burst on Gate Input 1
        if(dpt.ReadGate(1)) {
            int burst_count = (int)(dpt.ReadPot(0) * 8.0f) + 1;  // 1-8 bursts
            burst_gen.Trigger(burst_count);
        }
        
        // Process burst generator
        bool burst_output = burst_gen.Process(current_time);
        
        // Output gates
        dpt.SetGate(0, seq_output || burst_output);
        dpt.SetGate(1, divided_clock);
        
        // Visual feedback - show sequencer position on LEDs
        for(int i = 0; i < 4; i++) {
            dpt.SetLED(i, sequencer.GetCurrentStep() / 4 == i);
        }
        
        // Avoid busy waiting
        System::Delay(1);
    }
}
```

### 4. SD Card Data Logging and Preset Management
```cpp
// sd_test.cpp - SD card operations for data logging and preset storage
#include "daisy_dpt.h"
#include "fatfs.h"

DPT dpt;

// Preset structure
struct Preset {
    char name[32];
    float cv_values[6];
    float pot_positions[8];
    uint8_t midi_channel;
    uint32_t checksum;
    
    uint32_t CalculateChecksum() {
        uint32_t sum = 0;
        uint8_t* data = (uint8_t*)this;
        for(size_t i = 0; i < sizeof(Preset) - sizeof(uint32_t); i++) {
            sum += data[i];
        }
        return sum;
    }
    
    bool Validate() {
        return checksum == CalculateChecksum();
    }
};

// Data logger for recording CV and audio
class DataLogger {
private:
    FIL file;
    bool is_logging = false;
    uint32_t sample_count = 0;
    char filename[64];
    
public:
    bool StartLogging(const char* base_name) {
        // Generate unique filename with timestamp
        sprintf(filename, "%s_%lu.csv", base_name, System::GetNow());
        
        // Open file for writing
        FRESULT result = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
        if(result == FR_OK) {
            // Write CSV header
            f_printf(&file, "Sample,Time_ms,CV1,CV2,CV3,CV4,Gate1,Gate2\n");
            is_logging = true;
            sample_count = 0;
            return true;
        }
        return false;
    }
    
    void LogData(float cv1, float cv2, float cv3, float cv4, 
                 bool gate1, bool gate2) {
        if(!is_logging) return;
        
        uint32_t time_ms = sample_count * 1000 / 48000;  // Convert to ms
        f_printf(&file, "%lu,%lu,%.3f,%.3f,%.3f,%.3f,%d,%d\n",
                 sample_count, time_ms, cv1, cv2, cv3, cv4, gate1, gate2);
        sample_count++;
        
        // Sync to SD card every 1000 samples
        if(sample_count % 1000 == 0) {
            f_sync(&file);
        }
    }
    
    void StopLogging() {
        if(is_logging) {
            f_close(&file);
            is_logging = false;
        }
    }
    
    bool IsLogging() { return is_logging; }
};

// Preset manager
class PresetManager {
private:
    static const int MAX_PRESETS = 128;
    Preset current_preset;
    
public:
    bool SavePreset(int slot) {
        char filename[32];
        sprintf(filename, "preset_%03d.dat", slot);
        
        FIL file;
        FRESULT result = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
        if(result == FR_OK) {
            // Fill preset data
            sprintf(current_preset.name, "Preset %d", slot);
            for(int i = 0; i < 6; i++) {
                current_preset.cv_values[i] = dpt.GetCVOutput(i);
            }
            for(int i = 0; i < 8; i++) {
                current_preset.pot_positions[i] = dpt.ReadPot(i);
            }
            current_preset.checksum = current_preset.CalculateChecksum();
            
            // Write preset
            UINT bytes_written;
            f_write(&file, &current_preset, sizeof(Preset), &bytes_written);
            f_close(&file);
            
            return bytes_written == sizeof(Preset);
        }
        return false;
    }
    
    bool LoadPreset(int slot) {
        char filename[32];
        sprintf(filename, "preset_%03d.dat", slot);
        
        FIL file;
        FRESULT result = f_open(&file, filename, FA_READ);
        if(result == FR_OK) {
            UINT bytes_read;
            f_read(&file, &current_preset, sizeof(Preset), &bytes_read);
            f_close(&file);
            
            if(bytes_read == sizeof(Preset) && current_preset.Validate()) {
                // Apply preset values
                for(int i = 0; i < 6; i++) {
                    dpt.SetCVOutput(i, current_preset.cv_values[i]);
                }
                return true;
            }
        }
        return false;
    }
    
    int ScanPresets() {
        DIR dir;
        FILINFO fno;
        int count = 0;
        
        if(f_opendir(&dir, "/") == FR_OK) {
            while(f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
                if(strstr(fno.fname, "preset_") && strstr(fno.fname, ".dat")) {
                    count++;
                }
            }
            f_closedir(&dir);
        }
        return count;
    }
};

// Global objects
DataLogger logger;
PresetManager presets;

void InitSDCard() {
    // Configure SD card hardware
    SdmmcHandler::Config sd_config;
    sd_config.Defaults();
    sd_config.speed = SdmmcHandler::Speed::FAST;
    
    // Initialize filesystem
    dpt.sdcard.Init(sd_config);
    
    // Mount filesystem
    f_mount(&dpt.SDFatFS, "/", 1);
    
    // Create directories if needed
    f_mkdir("/presets");
    f_mkdir("/logs");
}

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size) {
    static int log_decimation = 0;
    
    for(size_t i = 0; i < size; i += 2) {
        // Audio processing...
        out[i] = in[i];
        out[i+1] = in[i+1];
        
        // Log data at reduced rate (every 48 samples = 1kHz)
        if(logger.IsLogging() && ++log_decimation >= 48) {
            log_decimation = 0;
            logger.LogData(
                dpt.GetCVOutput(0),
                dpt.GetCVOutput(1),
                dpt.GetCVOutput(2),
                dpt.GetCVOutput(3),
                dpt.gate[0].State(),
                dpt.gate[1].State()
            );
        }
    }
}

int main(void) {
    dpt.Init();
    InitSDCard();
    
    dpt.SetAudioBlockSize(48);
    dpt.StartAudio(AudioCallback);
    
    // Check for preset count
    int preset_count = presets.ScanPresets();
    
    // Load first preset if available
    if(preset_count > 0) {
        presets.LoadPreset(0);
    }
    
    bool last_gate1 = false;
    bool last_gate2 = false;
    
    while(1) {
        bool gate1 = dpt.ReadGate(0);
        bool gate2 = dpt.ReadGate(1);
        
        // Rising edge on Gate 1: Start/stop logging
        if(gate1 && !last_gate1) {
            if(logger.IsLogging()) {
                logger.StopLogging();
                dpt.SetLED(0, false);
            } else {
                if(logger.StartLogging("/logs/cv_log")) {
                    dpt.SetLED(0, true);
                }
            }
        }
        
        // Rising edge on Gate 2: Save preset
        if(gate2 && !last_gate2) {
            int slot = (int)(dpt.ReadPot(7) * 127.0f);  // Pot 7 selects slot
            if(presets.SavePreset(slot)) {
                // Flash LED to indicate save
                for(int i = 0; i < 3; i++) {
                    dpt.SetLED(1, true);
                    System::Delay(100);
                    dpt.SetLED(1, false);
                    System::Delay(100);
                }
            }
        }
        
        last_gate1 = gate1;
        last_gate2 = gate2;
        
        System::Delay(1);
    }
}
```

### 5. Complete Integration Example
```cpp
// full_integration.cpp - Complete synthesizer voice using all DPT features
#include "daisy_dpt.h"
#include "daisysp.h"

using namespace daisysp;

DPT dpt;

// Synthesizer voice structure
class SynthVoice {
private:
    // Oscillators
    Oscillator osc1, osc2;
    
    // Filter
    MoogLadder filter;
    
    // Envelopes
    Adsr amp_env, filter_env;
    
    // LFO
    Oscillator lfo;
    
    // Voice parameters
    float frequency = 440.0f;
    float amplitude = 0.5f;
    float filter_cutoff = 1000.0f;
    float filter_res = 0.5f;
    float detune = 0.0f;
    
public:
    void Init(float sample_rate) {
        // Initialize oscillators
        osc1.Init(sample_rate);
        osc1.SetWaveform(Oscillator::WAVE_SAW);
        osc1.SetFreq(frequency);
        
        osc2.Init(sample_rate);
        osc2.SetWaveform(Oscillator::WAVE_SQUARE);
        osc2.SetFreq(frequency);
        
        // Initialize filter
        filter.Init(sample_rate);
        filter.SetFreq(filter_cutoff);
        filter.SetRes(filter_res);
        
        // Initialize envelopes
        amp_env.Init(sample_rate);
        amp_env.SetTime(ADSR_SEG_ATTACK, 0.01f);
        amp_env.SetTime(ADSR_SEG_DECAY, 0.1f);
        amp_env.SetTime(ADSR_SEG_RELEASE, 0.5f);
        amp_env.SetSustainLevel(0.7f);
        
        filter_env.Init(sample_rate);
        filter_env.SetTime(ADSR_SEG_ATTACK, 0.05f);
        filter_env.SetTime(ADSR_SEG_DECAY, 0.2f);
        filter_env.SetTime(ADSR_SEG_RELEASE, 0.3f);
        filter_env.SetSustainLevel(0.3f);
        
        // Initialize LFO
        lfo.Init(sample_rate);
        lfo.SetWaveform(Oscillator::WAVE_SIN);
        lfo.SetFreq(2.0f);
    }
    
    void SetFrequency(float freq) {
        frequency = freq;
        osc1.SetFreq(frequency);
        osc2.SetFreq(frequency * (1.0f + detune));
    }
    
    void SetParameters(float cutoff, float res, float det) {
        filter_cutoff = cutoff;
        filter_res = res;
        detune = det;
        filter.SetRes(res);
    }
    
    void NoteOn(float freq, float vel) {
        SetFrequency(freq);
        amplitude = vel;
        amp_env.Trigger();
        filter_env.Trigger();
    }
    
    void NoteOff() {
        amp_env.Release();
        filter_env.Release();
    }
    
    float Process() {
        // Generate oscillators
        float osc_mix = (osc1.Process() + osc2.Process()) * 0.5f;
        
        // Apply filter with envelope modulation
        float filter_mod = filter_env.Process();
        float modulated_cutoff = filter_cutoff + (filter_mod * 2000.0f);
        filter.SetFreq(modulated_cutoff);
        
        float filtered = filter.Process(osc_mix);
        
        // Apply amplitude envelope
        float output = filtered * amp_env.Process() * amplitude;
        
        return output;
    }
    
    bool IsActive() {
        return amp_env.IsRunning();
    }
};

// Global objects
static const int MAX_VOICES = 4;
SynthVoice voices[MAX_VOICES];
int next_voice = 0;

// Effects
ReverbSc reverb;
Overdrive distortion;

// Sequencer data
struct SequencerStep {
    uint8_t note;
    uint8_t velocity;
    bool gate;
    float cv1, cv2;
};

SequencerStep sequence[16];
int seq_position = 0;
bool seq_running = false;

// Performance recorder
class PerformanceRecorder {
private:
    static const int MAX_EVENTS = 1024;
    struct Event {
        uint32_t timestamp;
        uint8_t type;  // 0=note, 1=cc, 2=cv
        uint8_t data1;
        uint8_t data2;
        float value;
    } events[MAX_EVENTS];
    
    int write_index = 0;
    int read_index = 0;
    bool recording = false;
    bool playing = false;
    uint32_t start_time = 0;
    
public:
    void StartRecording() {
        recording = true;
        playing = false;
        write_index = 0;
        start_time = System::GetNow();
    }
    
    void StartPlayback() {
        playing = true;
        recording = false;
        read_index = 0;
        start_time = System::GetNow();
    }
    
    void Stop() {
        recording = false;
        playing = false;
    }
    
    void RecordEvent(uint8_t type, uint8_t d1, uint8_t d2, float val) {
        if(!recording || write_index >= MAX_EVENTS) return;
        
        events[write_index].timestamp = System::GetNow() - start_time;
        events[write_index].type = type;
        events[write_index].data1 = d1;
        events[write_index].data2 = d2;
        events[write_index].value = val;
        write_index++;
    }
    
    bool GetNextEvent(Event& e) {
        if(!playing || read_index >= write_index) return false;
        
        uint32_t current_time = System::GetNow() - start_time;
        if(events[read_index].timestamp <= current_time) {
            e = events[read_index++];
            return true;
        }
        return false;
    }
};

PerformanceRecorder recorder;

// Initialize synthesizer
void InitSynth() {
    float sample_rate = dpt.AudioSampleRate();
    
    // Initialize all voices
    for(int i = 0; i < MAX_VOICES; i++) {
        voices[i].Init(sample_rate);
    }
    
    // Initialize effects
    reverb.Init(sample_rate);
    reverb.SetFeedback(0.85f);
    reverb.SetLpFreq(10000.0f);
    
    distortion.Init();
    distortion.SetDrive(0.5f);
    
    // Initialize sequence
    for(int i = 0; i < 16; i++) {
        sequence[i].note = 36 + (i % 4) * 12;  // C, C, C, C pattern
        sequence[i].velocity = 64;
        sequence[i].gate = (i % 4) == 0;
    }
}

// MIDI note to frequency conversion
float MidiToFreq(uint8_t note) {
    return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

// Find free voice for note allocation
int AllocateVoice() {
    // First, look for inactive voice
    for(int i = 0; i < MAX_VOICES; i++) {
        if(!voices[i].IsActive()) {
            return i;
        }
    }
    
    // If no free voice, use round-robin
    int v = next_voice;
    next_voice = (next_voice + 1) % MAX_VOICES;
    return v;
}

// Audio callback
void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size) {
    float reverb_amount = dpt.ReadPot(6);
    float dist_amount = dpt.ReadPot(7);
    
    for(size_t i = 0; i < size; i += 2) {
        // Read CV inputs for modulation
        float cv_filter = dpt.ReadCV(0) * 2000.0f;  // Filter cutoff mod
        float cv_res = dpt.ReadCV(1);               // Resonance mod
        
        // Mix all voices
        float voice_mix = 0.0f;
        for(int v = 0; v < MAX_VOICES; v++) {
            voice_mix += voices[v].Process();
        }
        voice_mix *= 0.25f;  // Scale for 4 voices
        
        // Apply distortion
        float distorted = distortion.Process(voice_mix * (1.0f + dist_amount * 4.0f));
        
        // Apply reverb
        float rev_l, rev_r;
        reverb.Process(distorted, distorted, &rev_l, &rev_r);
        
        // Mix dry and wet signals
        float final_l = distorted + (rev_l * reverb_amount);
        float final_r = distorted + (rev_r * reverb_amount);
        
        // Output
        out[i] = final_l;
        out[i+1] = final_r;
        
        // Update CV outputs with modulation signals
        static float mod_phase = 0.0f;
        mod_phase += 0.0001f;
        if(mod_phase > 1.0f) mod_phase -= 1.0f;
        
        // Output LFO to CV
        float lfo_out = sinf(mod_phase * TWOPI);
        dpt.SetCVOutput(0, (lfo_out + 1.0f) * 2.5f);  // 0-5V
        
        // Output envelope follower
        float env_follow = fabsf(voice_mix);
        dpt.SetCVOutput(1, env_follow * 5.0f);
    }
}

// Main program
int main(void) {
    // Initialize hardware
    dpt.Init();
    dpt.SetAudioBlockSize(48);
    
    // Initialize synthesizer
    InitSynth();
    
    // Initialize MIDI
    dpt.midi.Init();
    dpt.midi.StartReceive();
    
    // Initialize SD card for preset storage
    SdmmcHandler::Config sd_config;
    sd_config.Defaults();
    dpt.sdcard.Init(sd_config);
    f_mount(&dpt.SDFatFS, "/", 1);
    
    // Start audio
    dpt.StartAudio(AudioCallback);
    
    // Main loop
    uint32_t last_tick = System::GetNow();
    uint32_t tick_period = 125;  // 120 BPM
    
    while(1) {
        // Process MIDI
        dpt.midi.Listen();
        while(dpt.midi.HasEvents()) {
            MidiEvent msg = dpt.midi.PopEvent();
            
            switch(msg.type) {
                case NoteOn: {
                    uint8_t note = msg.data[0];
                    uint8_t velocity = msg.data[1];
                    
                    if(velocity > 0) {
                        int voice = AllocateVoice();
                        voices[voice].NoteOn(MidiToFreq(note), velocity / 127.0f);
                        dpt.SetGate(0, true);
                        
                        // Record performance
                        recorder.RecordEvent(0, note, velocity, 0);
                    }
                    break;
                }
                
                case NoteOff: {
                    // In real implementation, track which voice plays which note
                    dpt.SetGate(0, false);
                    break;
                }
                
                case ControlChange: {
                    uint8_t cc = msg.data[0];
                    uint8_t value = msg.data[1];
                    
                    // Map CC to parameters
                    switch(cc) {
                        case 74:  // Filter cutoff
                            for(int i = 0; i < MAX_VOICES; i++) {
                                voices[i].SetParameters(
                                    value * 40.0f,  // 0-5080 Hz
                                    voices[0].filter_res,
                                    voices[0].detune
                                );
                            }
                            break;
                    }
                    
                    recorder.RecordEvent(1, cc, value, 0);
                    break;
                }
            }
        }
        
        // Handle sequencer
        uint32_t now = System::GetNow();
        if(seq_running && (now - last_tick) >= tick_period) {
            last_tick = now;
            
            // Trigger current step
            if(sequence[seq_position].gate) {
                int voice = AllocateVoice();
                voices[voice].NoteOn(
                    MidiToFreq(sequence[seq_position].note),
                    sequence[seq_position].velocity / 127.0f
                );
                
                // Output sequencer CV
                dpt.SetCVOutput(2, sequence[seq_position].cv1);
                dpt.SetCVOutput(3, sequence[seq_position].cv2);
                
                // Gate output
                dpt.SetGate(1, true);
            } else {
                dpt.SetGate(1, false);
            }
            
            // Advance sequencer
            seq_position = (seq_position + 1) % 16;
            
            // Update LED display
            for(int i = 0; i < 4; i++) {
                dpt.SetLED(i, (seq_position / 4) == i);
            }
        }
        
        // Handle playback of recorded performance
        if(recorder.playing) {
            PerformanceRecorder::Event e;
            while(recorder.GetNextEvent(e)) {
                if(e.type == 0) {  // Note event
                    int voice = AllocateVoice();
                    voices[voice].NoteOn(MidiToFreq(e.data1), e.data2 / 127.0f);
                }
            }
        }
        
        // Read pots for real-time control
        float filter_freq = dpt.ReadPot(0) * 5000.0f;
        float resonance = dpt.ReadPot(1);
        float detune = dpt.ReadPot(2) * 0.1f;
        
        // Update all voice parameters
        for(int i = 0; i < MAX_VOICES; i++) {
            voices[i].SetParameters(filter_freq, resonance, detune);
        }
        
        // Sequencer control
        if(dpt.ReadGate(0) && !seq_running) {
            seq_running = true;
            seq_position = 0;
        } else if(!dpt.ReadGate(0) && seq_running) {
            seq_running = false;
        }
        
        // Recording control
        if(dpt.ReadGate(1)) {
            if(!recorder.recording) {
                recorder.StartRecording();
                dpt.SetLED(5, true);  // Recording indicator
            }
        } else {
            if(recorder.recording) {
                recorder.Stop();
                dpt.SetLED(5, false);
            }
        }
        
        // Handle preset save/load with pot + button combo
        static bool last_save_state = false;
        bool save_pressed = dpt.ReadPot(4) > 0.9f;  // Use pot as button
        
        if(save_pressed && !last_save_state) {
            int preset_num = (int)(dpt.ReadPot(5) * 16.0f);
            // Save current state to preset file
            // Implementation depends on what parameters to save
        }
        last_save_state = save_pressed;
        
        System::Delay(1);
    }
}
```

## API Reference

### Core Functions

#### Initialization
```cpp
void DPT::Init()
```
Initializes all hardware subsystems including DAC, ADC, MIDI, SD card interface, and GPIO.

#### Audio Configuration
```cpp
void DPT::SetAudioBlockSize(size_t size)
void DPT::SetAudioSampleRate(SaiHandle::Config::SampleRate rate)
void DPT::StartAudio(AudioHandle::AudioCallback callback)
void DPT::StopAudio()
float DPT::AudioSampleRate()
```

#### Analog Input
```cpp
float DPT::ReadPot(int channel)      // Returns 0.0 - 1.0
float DPT::ReadCV(int channel)       // Returns 0.0 - 1.0 (0-5V input)
```

#### Control Voltage Output
```cpp
void DPT::SetCVOutput(int channel, float value)  // 0.0 - 1.0 normalized
float DPT::GetCVOutput(int channel)
```
- Channels 0-1: Daisy native DAC (-5V to +10V)
- Channels 2-5: DAC7554 (-7V to +7V)

#### Gate I/O
```cpp
bool DPT::ReadGate(int channel)      // Returns true/false
void DPT::SetGate(int channel, bool state)
```

#### MIDI
```cpp
MidiUartHandler midi;                 // Public member for MIDI access
void DPT::ProcessMidiEvents()        // Process all pending MIDI
```

#### LED Control
```cpp
void DPT::SetLED(int index, bool state)
void DPT::SetLED(int index, float brightness)  // 0.0 - 1.0 PWM
void DPT::UpdateLEDs()               // Update LED states
```

#### SD Card
```cpp
SdmmcHandler sdcard;                  // Public member for SD access
FatFS SDFatFS;                       // FatFS instance
```

### Pin Mappings

#### Daisy Patch SM Connections
- **Audio In**: Left/Right stereo input
- **Audio Out**: Left/Right stereo output
- **Gate Inputs**: GPIO pins configured for digital input
- **Gate Outputs**: GPIO pins with LED drivers
- **SPI**: Connected to DAC7554 and SD card
- **UART**: MIDI I/O

#### Expansion Header
- Pin 1-2: +5V Power
- Pin 3-4: +3.3V Power
- Pin 5-6: GND
- Pin 7-8: SPI (CLK, MOSI, MISO, CS)
- Pin 9-10: I2C (SDA, SCL)
- Pin 11-12: UART (TX, RX)
- Pin 13-14: USB (D+, D-)
- Pin 15-16: Reserved

## Development Guidelines

### Best Practices
1. **Audio Callback Efficiency**
   - Keep processing minimal in audio callback
   - Avoid memory allocation
   - Use fixed-point math where appropriate

2. **Real-time Constraints**
   - Process MIDI in main loop, not audio callback
   - Use DMA for large data transfers
   - Buffer SD card operations

3. **Hardware Protection**
   - Always check voltage ranges
   - Use provided scaling functions
   - Respect current limits

### Common Patterns

#### Parameter Smoothing
```cpp
class Parameter {
    float target, current, slew_rate;
public:
    void SetTarget(float t) { target = t; }
    float Process() {
        float diff = target - current;
        float max_change = slew_rate;
        if(fabs(diff) < max_change) {
            current = target;
        } else {
            current += (diff > 0) ? max_change : -max_change;
        }
        return current;
    }
};
```

#### Fixed-Point CV Processing
```cpp
// Convert float to 12-bit DAC value
uint16_t FloatToDAC(float value) {
    return (uint16_t)(value * 4095.0f);
}

// Scale bipolar signal for unipolar DAC
float BipolarToUnipolar(float bipolar) {
    return (bipolar + 1.0f) * 0.5f;
}
```

## Troubleshooting

### Common Issues
1. **No Audio Output**
   - Check power supply connections
   - Verify audio callback is running
   - Check codec initialization

2. **MIDI Not Working**
   - Verify TRS cable type (A/B)
   - Check optocoupler power
   - Monitor MIDI activity LED

3. **SD Card Errors**
   - Format as FAT32
   - Check card compatibility
   - Verify SPI connections

4. **CV Output Issues**
   - Measure reference voltage
   - Check op-amp power
   - Verify DAC communication

### Debug Features
- Serial output via USB
- LED status indicators
- Test points on PCB

## Hardware Design Notes

### Power Distribution
- Input protection on ±12V rails
- Separate analog and digital grounds
- Decoupling on all power pins
- Thermal considerations for regulators

### Signal Path
- Input protection on all CV/Gate inputs
- Output buffering on all CV outputs
- Proper impedance matching
- EMI considerations

### Manufacturing
- 4-layer PCB recommended
- Controlled impedance for high-speed signals
- Proper stackup for noise immunity
- Test points for production testing

## Future Development

### Planned Features
- USB host support for MIDI devices
- Expanded preset system
- WiFi/Bluetooth connectivity option
- Touch-sensitive controls
- Additional expander modules

### Community Contributions
- Example patches and applications
- Hardware modifications
- Alternative firmware
- Educational materials

## Resources

### Links
- [GitHub Repository](https://github.com/joemisra/dpt)
- [dsp.coffee](https://dsp.coffee/)
- [Daisy Documentation](https://electro-smith.github.io/libDaisy/)
- [DaisySP DSP Library](https://electro-smith.github.io/DaisySP/)

### References
- STM32H7 Reference Manual
- DAC7554 Datasheet
- PCM3080 Codec Datasheet
- Eurorack Electrical Specifications

## License
[Specify your license here - e.g., MIT, GPL, etc.]

## Acknowledgments
- Electrosmith for the Daisy platform
- makingsoundmachines
- thisisnotrocketscience
- mmalex
- lpzw for MIDI auto-sensing circuit
- The modular synthesis community

---

*DPT Module - Multifunction Electron Temple*  
*Version 1.0 - 2024*

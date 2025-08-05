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

#### Basic Include Pattern
```cpp
// The DPT examples typically use this simpler include structure:
#include "daisy_dpt.h"

// For projects using DaisySP:
#include "daisysp.h"

// The DPT header likely includes all necessary Daisy components internally

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

## Project Structure

### Basic Project Layout
```
your_project/
├── Makefile
├── your_project.cpp
├── lib/
│   ├── daisy_dpt.h
│   └── daisy_dpt.cpp
├── libDaisy/           (submodule or symlink)
└── DaisySP/            (submodule or symlink)
```

### Makefile Template
```makefile
# Project Name
TARGET = your_project

# Sources
CPP_SOURCES = your_project.cpp lib/daisy_dpt.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

# Core
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
```

### Initialization Pattern
```cpp
// your_project.cpp
#include "daisy_dpt.h"
#include "daisysp.h"  // If using DaisySP

DPT dpt;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size) {
    // Audio processing here
}

int main(void) {
    // Initialize hardware
    dpt.Init();
    
    // Get sample rate for DSP initialization
    float sample_rate = dpt.AudioSampleRate();
    
    // Initialize your DSP objects here
    
    // Configure and start audio
    dpt.SetAudioBlockSize(48);  // Lower values = lower latency
    dpt.StartAudio(AudioCallback);
    
    // Main loop for non-audio tasks
    while(1) {
        // Handle MIDI, UI updates, etc.
        System::Delay(1);
    }
}
```

## Hardware Initialization Details

### DPT Class Structure
The DPT class extends DaisyPatchSM with additional hardware:

```cpp
class DPT : public DaisyPatchSM {
public:
    // Additional hardware
    DAC7554 external_dac;
    MidiUartHandler midi;
    SdmmcHandler sdcard;
    
    // Extended GPIO
    Switch gate_in[2];
    GPIO gate_out[2];
    
    // Status LEDs
    GPIO leds[6];
    
    void Init() {
        // Base initialization
        DaisyPatchSM::Init();
        
        // Configure additional hardware
        InitDAC();
        InitMidi();
        InitGates();
        InitLEDs();
        InitSDCard();
    }
};
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

## DaisySP Integration

### Commonly Used DaisySP Classes

The DPT module works seamlessly with DaisySP, a comprehensive DSP library. Here are the most commonly used classes:

#### Oscillators
```cpp
// Basic oscillator with multiple waveforms
Oscillator osc;
osc.Init(sample_rate);
osc.SetWaveform(Oscillator::WAVE_SIN);  // Also: WAVE_TRI, WAVE_SAW, WAVE_SQUARE, WAVE_POLYBLEP_TRI, WAVE_POLYBLEP_SAW, WAVE_POLYBLEP_SQUARE
osc.SetFreq(440.0f);
osc.SetAmp(0.5f);
float sample = osc.Process();

// Variable shape oscillator
VariableShapeOscillator variosc;
variosc.Init(sample_rate);
variosc.SetFreq(220.0f);
variosc.SetPW(0.5f);      // Pulse width for square
variosc.SetWaveshape(0.5f); // Morphs between waveforms
float sample = variosc.Process();

// FM operator
Fm2 fm;
fm.Init(sample_rate);
fm.SetFrequency(440.0f);
fm.SetRatio(2.0f);      // Carrier:modulator ratio
fm.SetIndex(5.0f);      // Modulation index
float sample = fm.Process();
```

#### Filters
```cpp
// Moog ladder filter
MoogLadder moog;
moog.Init(sample_rate);
moog.SetFreq(1000.0f);   // Cutoff frequency
moog.SetRes(0.7f);       // Resonance 0-1
float filtered = moog.Process(input);

// State variable filter
Svf svf;
svf.Init(sample_rate);
svf.SetFreq(2000.0f);
svf.SetRes(0.5f);
svf.Process(input);
float lp = svf.Low();    // Low pass output
float hp = svf.High();   // High pass output
float bp = svf.Band();   // Band pass output
float notch = svf.Notch(); // Notch output

// One-pole filters
Tone tone;              // Low pass
tone.Init(sample_rate);
tone.SetFreq(1000.0f);
float lp = tone.Process(input);

ATone atone;            // High pass
atone.Init(sample_rate);
atone.SetFreq(100.0f);
float hp = atone.Process(input);
```

#### Envelopes
```cpp
// ADSR envelope
Adsr env;
env.Init(sample_rate);
env.SetTime(ADSR_SEG_ATTACK, 0.01f);
env.SetTime(ADSR_SEG_DECAY, 0.1f);
env.SetTime(ADSR_SEG_RELEASE, 0.5f);
env.SetSustainLevel(0.7f);
env.SetCurve(0.5f);     // Exponential curve
env.Trigger();          // Start envelope
env.Release();          // Release stage
float level = env.Process(gate_high);

// AD envelope
AdEnv ad;
ad.Init(sample_rate);
ad.SetTime(ADENV_SEG_ATTACK, 0.001f);
ad.SetTime(ADENV_SEG_DECAY, 0.2f);
ad.SetCurve(0.0f);      // Linear
ad.SetMax(1.0f);
ad.SetMin(0.0f);
ad.Trigger();
float level = ad.Process();

// Line generator
Line line;
line.Init(sample_rate);
line.Start(0.0f, 1.0f, 0.5f); // Start, end, time
float val = line.Process();
bool finished = line.IsRunning();
```

#### Effects
```cpp
// Reverb
ReverbSc reverb;
reverb.Init(sample_rate);
reverb.SetFeedback(0.85f);
reverb.SetLpFreq(10000.0f);
float wet_l, wet_r;
reverb.Process(dry_l, dry_r, &wet_l, &wet_r);

// Delay line
DelayLine<float, 48000> delay;  // 1 second at 48kHz
delay.Init();
delay.SetDelay(24000);           // 0.5 seconds
delay.Write(input);
float delayed = delay.Read();

// Chorus
Chorus chorus;
chorus.Init(sample_rate);
chorus.SetLfoFreq(0.3f);
chorus.SetLfoDepth(0.8f);
chorus.SetDelay(0.75f);
chorus.SetFeedback(0.2f);
float wet = chorus.Process(dry);

// Overdrive
Overdrive drive;
drive.Init();
drive.SetDrive(0.5f);
float driven = drive.Process(input);

// Compressor
Compressor comp;
comp.Init(sample_rate);
comp.SetThreshold(-20.0f);  // dB
comp.SetRatio(4.0f);        // 4:1
comp.SetAttack(0.001f);
comp.SetRelease(0.1f);
comp.SetMakeup(1.0f);
float compressed = comp.Process(input);
```

#### Utilities
```cpp
// Metro (clock/trigger generator)
Metro metro;
metro.Init(1.0f, sample_rate);  // 1 Hz
if(metro.Process()) {
    // Trigger event
}

// Phasor (ramp generator)
Phasor phasor;
phasor.Init(sample_rate);
phasor.SetFreq(0.25f);          // 0.25 Hz = 4 second ramp
float phase = phasor.Process(); // 0-1 ramp

// Port (parameter smoothing)
Port port;
port.Init(sample_rate, 0.05f);  // 50ms smoothing time
float smoothed = port.Process(target_value);

// Limiter
Limiter limiter;
limiter.Init();
limiter.SetThresh(-6.0f);       // dB
limiter.SetMakeup(6.0f);        // dB
limiter.ProcessBlock(buffer, size, 0.01f);

// DC block
DcBlock dcblock;
dcblock.Init(sample_rate);
float dc_removed = dcblock.Process(input);
```

#### Synthesis Building Blocks
```cpp
// Harmonic oscillator (additive synthesis)
HarmOscillator<16> harm;  // 16 harmonics
harm.Init(sample_rate);
harm.SetFirstHarmIdx(1);   // Start at fundamental
harm.SetFreq(220.0f);
for(int i = 0; i < 16; i++) {
    harm.SetSingleAmp(1.0f / (i + 1), i);  // 1/n amplitude
}
float sample = harm.Process();

// Grainlet (granular synthesis)
GrainletOscillator grainlet;
grainlet.Init(sample_rate);
grainlet.SetFreq(440.0f);
grainlet.SetFormantFreq(2000.0f);
grainlet.SetShape(0.5f);
grainlet.SetBleed(0.1f);
float sample = grainlet.Process();

// Modal voice (physical modeling)
ModalVoice modal;
modal.Init(sample_rate);
modal.SetFreq(440.0f);
modal.SetStructure(0.7f);
modal.SetBrightness(0.8f);
modal.SetDamping(0.9f);
modal.Trig();
float sample = modal.Process();

// String voice (Karplus-Strong)
StringVoice string;
string.Init(sample_rate);
string.SetFreq(110.0f);     // A2
string.SetBrightness(0.5f);
string.SetDamping(0.8f);
string.SetNonLinearity(0.1f);
string.Trig();
float sample = string.Process(trigger);
```

### Example: Complete Synth Voice with DaisySP
```cpp
// complete_voice.cpp - Full synth voice using DaisySP components
#include "daisy_dpt.h"
#include "daisysp.h"

DPT dpt;

class SynthVoice {
private:
    // Sound generators
    VariableShapeOscillator osc1, osc2;
    WhiteNoise noise;
    
    // Filters
    MoogLadder filter;
    Svf hpf;  // Pre-filter high pass
    
    // Envelopes
    Adsr amp_env, filter_env, mod_env;
    
    // LFOs
    Oscillator lfo1, lfo2;
    
    // Effects
    Chorus chorus;
    Overdrive drive;
    
    // Utilities
    Port glide;
    DcBlock dc_block;
    
    // Parameters
    float frequency = 440.0f;
    float target_freq = 440.0f;
    
public:
    void Init(float sample_rate) {
        // Oscillators
        osc1.Init(sample_rate);
        osc2.Init(sample_rate);
        noise.Init();
        
        // Filters
        filter.Init(sample_rate);
        hpf.Init(sample_rate);
        hpf.SetFreq(20.0f);  // Remove DC
        hpf.SetRes(0.0f);
        
        // Envelopes - different curves for each
        amp_env.Init(sample_rate);
        amp_env.SetCurve(-2.0f);  // Exponential
        
        filter_env.Init(sample_rate);
        filter_env.SetCurve(2.0f);  // Logarithmic
        
        mod_env.Init(sample_rate);
        mod_env.SetCurve(0.0f);   // Linear
        
        // LFOs at different rates
        lfo1.Init(sample_rate);
        lfo1.SetWaveform(Oscillator::WAVE_TRI);
        lfo1.SetFreq(0.3f);
        
        lfo2.Init(sample_rate);
        lfo2.SetWaveform(Oscillator::WAVE_SIN);
        lfo2.SetFreq(4.0f);
        
        // Effects
        chorus.Init(sample_rate);
        drive.Init();
        
        // Utilities
        glide.Init(sample_rate, 0.05f);  // 50ms glide
        dc_block.Init(sample_rate);
    }
    
    void NoteOn(float freq, float velocity) {
        target_freq = freq;
        amp_env.Trigger();
        filter_env.Trigger();
        mod_env.Trigger();
    }
    
    void NoteOff() {
        amp_env.Release();
        filter_env.Release();
        mod_env.Release();
    }
    
    float Process() {
        // Glide to target frequency
        frequency = glide.Process(target_freq);
        
        // LFO processing
        float lfo1_out = lfo1.Process();
        float lfo2_out = lfo2.Process();
        
        // Vibrato from LFO2
        float vibrato_freq = frequency * (1.0f + lfo2_out * 0.01f);
        
        // Set oscillator frequencies with slight detune
        osc1.SetFreq(vibrato_freq);
        osc2.SetFreq(vibrato_freq * 1.004f);  // Slight detune
        
        // PWM from LFO1
        osc1.SetPW(0.5f + lfo1_out * 0.45f);
        osc2.SetPW(0.5f - lfo1_out * 0.45f);
        
        // Generate oscillators
        float osc_mix = osc1.Process() + osc2.Process() * 0.7f;
        
        // Add noise
        float noise_level = mod_env.Process() * 0.1f;
        osc_mix += noise.Process() * noise_level;
        
        // Pre-filter high pass
        hpf.Process(osc_mix);
        osc_mix = hpf.High();
        
        // Filter with envelope
        float filter_env_amt = filter_env.Process();
        float filter_freq = 200.0f + filter_env_amt * 3000.0f;
        filter_freq += lfo1_out * 200.0f;  // LFO modulation
        
        filter.SetFreq(filter_freq);
        float filtered = filter.Process(osc_mix);
        
        // Soft clipping
        filtered = drive.Process(filtered * 0.7f);
        
        // Apply amplitude envelope
        float output = filtered * amp_env.Process();
        
        // Chorus for width
        output = chorus.Process(output);
        
        // DC blocking
        output = dc_block.Process(output);
        
        return output * 0.5f;  // Scale output
    }
    
    // Parameter setters
    void SetFilterCutoff(float freq) { 
        filter.SetFreq(freq); 
    }
    
    void SetFilterResonance(float res) { 
        filter.SetRes(res); 
    }
    
    void SetOscShape(float shape) {
        osc1.SetWaveshape(shape);
        osc2.SetWaveshape(1.0f - shape);
    }
    
    void SetAttack(float time) {
        amp_env.SetTime(ADSR_SEG_ATTACK, time);
        filter_env.SetTime(ADSR_SEG_ATTACK, time * 0.8f);
    }
    
    void SetDecay(float time) {
        amp_env.SetTime(ADSR_SEG_DECAY, time);
        filter_env.SetTime(ADSR_SEG_DECAY, time * 1.2f);
    }
    
    void SetSustain(float level) {
        amp_env.SetSustainLevel(level);
        filter_env.SetSustainLevel(level * 0.5f);
    }
    
    void SetRelease(float time) {
        amp_env.SetTime(ADSR_SEG_RELEASE, time);
        filter_env.SetTime(ADSR_SEG_RELEASE, time * 0.7f);
    }
};

// Polyphonic synthesizer with 8 voices
static const int NUM_VOICES = 8;
SynthVoice voices[NUM_VOICES];
int next_voice = 0;

// Global reverb send
ReverbSc reverb;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size) {
    // Read controls
    float cutoff = dpt.ReadPot(0) * 5000.0f;
    float resonance = dpt.ReadPot(1);
    float osc_shape = dpt.ReadPot(2);
    float attack = dpt.ReadPot(3) * 2.0f;
    float decay = dpt.ReadPot(4) * 2.0f;
    float sustain = dpt.ReadPot(5);
    float release = dpt.ReadPot(6) * 5.0f;
    float reverb_mix = dpt.ReadPot(7);
    
    for(size_t i = 0; i < size; i += 2) {
        float voice_sum = 0.0f;
        
        // Process all voices
        for(int v = 0; v < NUM_VOICES; v++) {
            voices[v].SetFilterCutoff(cutoff);
            voices[v].SetFilterResonance(resonance);
            voices[v].SetOscShape(osc_shape);
            voices[v].SetAttack(attack);
            voices[v].SetDecay(decay);
            voices[v].SetSustain(sustain);
            voices[v].SetRelease(release);
            
            voice_sum += voices[v].Process();
        }
        
        // Scale for polyphony
        voice_sum *= 0.125f;
        
        // Apply reverb
        float wet_l, wet_r;
        reverb.Process(voice_sum, voice_sum, &wet_l, &wet_r);
        
        // Mix dry and wet
        float final_l = voice_sum + wet_l * reverb_mix;
        float final_r = voice_sum + wet_r * reverb_mix;
        
        out[i] = final_l;
        out[i+1] = final_r;
    }
}

int main(void) {
    // Hardware init
    dpt.Init();
    float sample_rate = dpt.AudioSampleRate();
    
    // Initialize all voices
    for(int i = 0; i < NUM_VOICES; i++) {
        voices[i].Init(sample_rate);
    }
    
    // Initialize reverb
    reverb.Init(sample_rate);
    reverb.SetFeedback(0.87f);
    reverb.SetLpFreq(8000.0f);
    
    // MIDI setup
    dpt.midi.Init();
    dpt.midi.StartReceive();
    
    // Start audio
    dpt.SetAudioBlockSize(48);
    dpt.StartAudio(AudioCallback);
    
    // Main loop
    while(1) {
        // Process MIDI
        dpt.midi.Listen();
        while(dpt.midi.HasEvents()) {
            MidiEvent e = dpt.midi.PopEvent();
            
            if(e.type == NoteOn && e.data[1] > 0) {
                // Note to frequency
                float freq = 440.0f * powf(2.0f, (e.data[0] - 69) / 12.0f);
                
                // Trigger next voice (round-robin)
                voices[next_voice].NoteOn(freq, e.data[1] / 127.0f);
                next_voice = (next_voice + 1) % NUM_VOICES;
                
                // Visual feedback
                dpt.SetGate(0, true);
                dpt.SetLED(next_voice % 4, true);
            }
            else if(e.type == NoteOff || (e.type == NoteOn && e.data[1] == 0)) {
                // Simple note off - in practice you'd track which voice plays which note
                dpt.SetGate(0, false);
            }
        }
        
        // Update LEDs
        dpt.UpdateLEDs();
        System::Delay(1);
    }
}
```

## DaisySP Integration

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

#include "DAC7554.h"
#include "daisy_seed.h"
#include "daisy_patch_sm.h"
#include "../daisy_core.h"
#include "../per/spi.h"
#include "../per/qspi.h"
#include "../per/gpio.h"
#include "../sys/system.h"

// Driver for DAC7554 based on code from Making Sound Machines 
// Based on Code from Westlicht Performer   - https://westlicht.github.io/performer/
// Port for Daisy by Making Sound Machines  - https://github.com/makingsoundmachines

// Pins on Plinky Expander -
// 16   RX       TX     15
// 14   GND      CS     13
// 12   GND      +12V   11
// 10   MISO     CLK    9
// 8    MOSI     -12V   7
// 6    GND      +5V    5
// 4    DM       DP     3
// 2    +3v3     +3v3   1

// Pins on Daisy Seed used in this configuration:
// D7   CS
// D8   SCLK
// D9   MISO
// D10  MOSI

// Pins on Daisy Patch SM used in this configuration:
// D1   CS
// D10   SCLK
// D9   MISO
// D8  MOSI

// Usage:
// Dac7554 dac;
// dac.Init();
// dac.Set(1, [0-4095]);
// dac.Set(2, [0-4095]);
// dac.Set(3, [0-4095]);
// dac.Set(4, [0-4095]);

// dac.Write();

using namespace daisy;
using namespace patch_sm;
using namespace std;

//#define DMA_BUFF __attribute__((section(".sdram_bss")))
//uint8_t DMA_BUFFER_MEM_SECTION myBuffer[100];
#define EXTERNAL_SDRAM_SECTION __attribute__((section(".sdram_bss")))
//uint8_t DMA_BUFFER_MEM_SECTION *myBuffer;

class Dac7554Data {
    public:
    uint16_t Data;

    uint8_t Address;
    uint8_t Command;

    Dac7554Data(uint16_t address, uint16_t data) {
        uint16_t cmd = (2 << 14) | (address << 12) | data;

        Data = cmd;

        Address = (cmd >> 8) & 0xff;
        Command = cmd & 0xff;
    }
};


typedef struct
{
    uint8_t Initialized;
} Dac7554_t;

static SpiHandle         h_spi;
static dsy_gpio          pin_sync;
static Dac7554_t         Dac7554_;
static SpiHandle::Config spi_config;

void TxCpltCallback(void* context, daisy::SpiHandle::Result result) {
    return;
}

void TxStartCallback(void* context) {
    return;
}

void Dac7554::Init()
{
    // Initialize PIO - possibly not needed?
    pin_sync.mode = DSY_GPIO_MODE_OUTPUT_PP;
    pin_sync.pin  = DaisyPatchSM::D1;
    dsy_gpio_init(&pin_sync);

    // Initialize SPI

    spi_config.periph         = SpiHandle::Config::Peripheral::SPI_2;
    spi_config.mode           = SpiHandle::Config::Mode::MASTER;
    spi_config.direction      = SpiHandle::Config::Direction::TWO_LINES;
    spi_config.datasize       = 8;
    spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
    spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
    spi_config.nss            = SpiHandle::Config::NSS::HARD_OUTPUT;
    spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;

    spi_config.pin_config.sclk = DaisyPatchSM::D10;
    spi_config.pin_config.mosi = DaisyPatchSM::D9;
    spi_config.pin_config.miso = DaisyPatchSM::D8;
    spi_config.pin_config.nss  = DaisyPatchSM::D1; // {DSY_GPIOX, 0}; // DaisyPatchSM::D1; // {DSY_GPIOX, 0}; 

    h_spi.Init(spi_config);

    Dac7554_.Initialized = 1;
}

void Dac7554::Write(uint16_t gogo[4])
{
    //uint16_t cmd = (2 << 14) | (address << 12) | data;

    for(int i = 0; i < 4; i++)
        _values[i] = gogo[i];
    dac_ready = true;
}

void Dac7554::WriteDac7554()
{
    uint8_t bobo[8];

    if(!lock) {
        dac_ready = false;
        lock = true;

        for(int i=0; i<4; i++) {
            uint8_t chan = i;
            uint16_t cmd = (2 << 14) | (chan << 12) | _values[chan];

            bobo[0] = (cmd >> 8) & 0xff;
            bobo[1] = cmd & 0xff;

            //h_spi.DmaTransmit(bobo, 8, NULL, NULL, NULL);
            h_spi.BlockingTransmit(bobo, 8, 10);
        }


        lock = false;
    }
}

void Dac7554::Clear(void* context, int result) {

}

void Dac7554::Reset()
{
}

void Dac7554::SetInternalRef(bool enabled)
{
}

void Dac7554::SetClearCode(ClearCode code)
{
}
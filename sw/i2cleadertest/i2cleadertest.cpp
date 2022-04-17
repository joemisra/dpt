#include <stdio.h>
#include <string.h>
#include "../../lib/daisy_dpt.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;
using namespace dpt;

/** Typedef the OledDisplay to make syntax cleaner below 
 *  This is a 4Wire SPI Transport controlling an 128x64 sized SSDD1306
 * 
 *  There are several other premade test 
*/
using MyOledDisplay = OledDisplay<SSD130xI2c128x32Driver>;

DPT hw;
MyOledDisplay display;

uint8_t DMA_BUFFER_MEM_SECTION d[16];

void booboo(void* context, daisy::I2CHandle::Result result) {
    
}
class IITransport
{
  public:
    void Init()
    {
        I2CHandle::Config i2c_config;
        i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
        i2c_config.speed =I2CHandle::Config::Speed::I2C_1MHZ;
        i2c_config.pin_config.scl = DPT::B7;
        i2c_config.pin_config.sda = DPT::B8;
        i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
        i2c_address_ = 0x01;

        i2c_.Init(i2c_config);
    };

    void Receive() {
    }
    
    void SendCommand(uint8_t cmd)
    {
        uint8_t buf[2] = {0x04, cmd};
        i2c_.TransmitBlocking(i2c_address_, buf, 2, 1000);
    };

    void SendData(uint8_t* buff, size_t size)
    {
        for(size_t i = 0; i < size; i++)
        {
            uint8_t buf[2] = {0X40, buff[i]};
            i2c_.TransmitBlocking(i2c_address_, buf, 2, 1000);
        }
    };

    void LFO(int16_t channel, int16_t frequency, int16_t volts, int16_t skew) {
        //uint8_t buf[] = {11, 0x01, 0x11, 0x00, 0x21, 0xff, 0x00, 0x00};

        int16_t a     = channel;
        int16_t b     = frequency;
        int16_t c     = volts;
        int16_t e     = skew;

        d[0] = 11;
        d[1] = a;
        d[2] = b >> 8;
        d[3] = b & 0xff;
        d[4] = c >> 8;
        d[5] = c & 0xFF;
        d[6] = e >> 8;
        d[7] = e & 0xFF;

        i2c_.TransmitDma(0x01, d, 8, booboo, nullptr);
        //i2c_.TransmitBlocking(0x01, d, 8, 1000);
    }

    daisy::I2CHandle i2c_;
    uint8_t          i2c_address_;
};

IITransport crow;


int main(void)
{
    uint8_t message_idx;
    //hw.Configure();
    hw.Init();
    //hw.midi.Listen();

    crow.Init();

    Random rand;

    uint8_t recv[8];

    while(1)
    {
        //uint8_t buf[] = {0x01, 0x01, 0x00, 0xFF };

        int16_t b = 23457;

        //crow.i2c_.TransmitDma(0x01, buf, 8, booboo, nullptr);
        //crow.i2c_.TransmitBlocking(0x01, buf, 8, 1000);


        crow.LFO(1, (rand.GetValue() % 10000), 12000,  (rand.GetValue() % 32768));
        hw.Delay(100);
        crow.LFO(2, (rand.GetValue() % 10000), 12000,  (rand.GetValue() % 32768));
        hw.Delay(100);
        crow.LFO(3, (rand.GetValue() % 10000), 12000,  (rand.GetValue() % 32768));
        hw.Delay(100);
        crow.LFO(4, (rand.GetValue() % 10000), 12000,  (rand.GetValue() % 32768));
        hw.Delay(100);

        //crow.i2c_.ReceiveDma(0x01, recv, 8, booboo, nullptr);

        hw.Delay(1000);
    }
}


    extern "C" void TIM5_IRQHandler(void)
{
    hw.tim5.Instance->SR = 0;

    /*
    for(int i=0;i<4;i++)
        warble[i]->SetFreq((i + 1) * (int)modmult);

    patch.WriteCvOutExp(
        (warble[0]->Process() + 0.5) * 2048,
        (warble[1]->Process() + 0.5) * 2048,
        (warble[2]->Process() + 0.5) * 2048,
        (warble[3]->Process() + 0.5) * 2048);
    if(!patch.dac_exp.lock) {
        patch.dac_exp.WriteDac7554();
    }
    */
}
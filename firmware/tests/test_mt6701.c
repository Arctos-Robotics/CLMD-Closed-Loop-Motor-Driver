#include "mt6701_codec.h"
#include <assert.h>
#include <stdio.h>

static unsigned reference_crc(unsigned data) {
    unsigned polynomial=data<<6;
    for(int bit=23;bit>=6;--bit)
        if(polynomial&(1u<<bit)) polynomial^=0x43u<<(bit-6);
    return polynomial&63;
}
static unsigned frame(unsigned raw,unsigned status) {
    unsigned data=(raw<<4)|status;
    return (data<<6)|reference_crc(data);
}
int main(void) {
    // All register byte combinations, including the two unused low bits.
    for(unsigned high=0;high<256;++high) for(unsigned low=0;low<256;++low)
        assert(mt6701_i2c_angle(high,low)==((high*256+low)>>2));
    // Independent polynomial division checks all possible data/status payloads.
    for(unsigned data=0;data<(1u<<18);++data) assert(mt6701_crc6(data)==reference_crc(data));
    unsigned angles[]={0,1,8192,16383};
    for(unsigned i=0;i<4;++i) {
        unsigned wire=frame(angles[i],0);
        mt6701_sample_t s=mt6701_decode(wire,true);
        assert(s.valid && s.crc_ok && s.raw==angles[i]);
        for(unsigned bit=0;bit<24;++bit) {
            s=mt6701_decode(wire^(1u<<bit),true);
            assert(!s.crc_ok && !s.valid);
        }
    }
    assert(mt6701_decode(0,true).valid);  // Exact angle zero is legitimate.
    assert(!mt6701_decode(0,false).valid); // Grounded data must not look healthy.
    assert(!mt6701_decode(0xffffff,true).valid); // Floating/pulled-up absent sensor.
    for(unsigned status=0;status<16;++status) {
        mt6701_sample_t s=mt6701_decode(frame(1234,status),true);
        assert(s.status==status && s.crc_ok);
        assert(s.valid==(status==0 || status==4)); // PUSH alone is informational.
    }
    for(unsigned op=0;op<256;++op) for(unsigned parameter=0;parameter<256;++parameter) {
        assert(mt6701_test_allows(false,true,op,parameter));
        assert(mt6701_test_allows(true,true,op,parameter)==(op==0xf3 && parameter==0));
        assert(!mt6701_test_allows(true,false,op,parameter));
    }
    puts("MT6701 TESTS PASS: exhaustive I2C register decode, legacy SSI CRC, endpoints and motor interlock");
}

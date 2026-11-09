#include "mt6701_codec.h"

uint16_t mt6701_i2c_angle(uint8_t reg03,uint8_t reg04) {
    return ((uint16_t)reg03<<6)|(reg04>>2);
}

uint8_t mt6701_crc6(uint32_t data18) {
    // MSB first, x^6+x+1, init=0, xorout=0, exactly 18 data/status bits.
    uint8_t crc=0;
    for(int bit=17;bit>=0;--bit) {
        unsigned feedback=((crc>>5)^((data18>>bit)&1))&1;
        crc=(crc<<1)&0x3f;
        if(feedback) crc^=0x03;
    }
    return crc;
}
mt6701_sample_t mt6701_decode(uint32_t frame,bool idle_high) {
    mt6701_sample_t s={.frame=frame&0xffffff,.idle_high=idle_high};
    s.raw=s.frame>>10; s.status=(s.frame>>6)&15;
    s.crc_received=s.frame&63; s.crc_expected=mt6701_crc6(s.frame>>6);
    s.crc_ok=s.crc_received==s.crc_expected;
    // Mg[1:0]: 0 normal, 1 strong, 2 weak, 3 reserved. Mg[3]: speed saturation.
    // Mg[2] is the push flag, not a bad-angle flag. Zero angle is legitimate.
    s.valid=idle_high && s.crc_ok && !(s.status&0x0b);
    return s;
}
bool mt6701_test_allows(bool active,bool native,uint8_t opcode,uint8_t parameter) {
    return !active || (native && opcode==0xf3 && parameter==0);
}

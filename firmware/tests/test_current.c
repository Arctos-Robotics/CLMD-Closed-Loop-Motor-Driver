#include "current.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void) {
    assert(current_scale(1641)==31);
    assert(fabs(current_rms_ma(1641)-1641.497519)<.001);
    assert(current_scale(1000)==18 && fabs(current_rms_ma(1000)-974.639)<.01);
    for(unsigned ma=50;ma<1641;ma++) {
        assert(current_rms_ma(ma)<=fmax(ma,52));
        assert(current_scale(ma)<=current_scale(ma+1));
    }
    puts("CURRENT TESTS PASS: 0.12-ohm full scale, monotonic quantization, legacy 1000mA and board maximum");
}

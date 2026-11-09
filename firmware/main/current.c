#include "current.h"
#include <math.h>
static const double ma_per_scale=325.0/(0.12+0.02)/1.4142135623730951/32;
uint8_t current_scale(uint16_t ma) {
    if(ma>=CLMD_MAX_CURRENT_MA) return 31;
    int scale=(int)floor(ma/ma_per_scale)-1;
    return scale<0?0:scale>31?31:scale;
}
double current_rms_ma(uint16_t ma) {return (current_scale(ma)+1)*ma_per_scale;}

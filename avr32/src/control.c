#include <string.h>

#include "print_funcs.h"

#include "bfin.h"
#include "control.h"
#include "delay.h"

// request a parameter change.
extern u8 ctl_param_change(u32 idx, u32 val) {
    // Add bounds checking for parameter index
    if (idx >= 256) {  // Reasonable upper limit for DSP parameters
        return 1;  // Return error for invalid index
    }
    
    bfin_wait_ready();
    delay_us(50);
    bfin_set_param(idx, val);
    return 0;
}

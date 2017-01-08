
#include <string.h>
#include "MK22F51212.h"
#include "uart1.h"
#include "mcu_tracer.h"
#include "leds.h"


int main(void)
{
	SystemCoreClockUpdate();
	init_leds();
	mcu_tracer_config();

    for (;;) {

        mcu_tracer_process();
        if(debug1==0){
        	red_on();
        }else{
        	red_off();
        }
        mainloop_iterations++;
        if(mainloop_iterations>1000)mainloop_iterations=0;
    }
    /* Never leave main */
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

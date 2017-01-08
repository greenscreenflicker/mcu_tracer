/*
 * mcu_tracer.h
 *
 *  Created on: Jun 20, 2016
 *      Author: mmh
 */

#ifndef INCLUDES_MCU_TRACER_H_
#define INCLUDES_MCU_TRACER_H_

#include <stdint.h>
#define MCU_TRACER_STARTBYTE 0xA5
#define MCU_TRACER_SENDBUF_SIZE 500


typedef struct mcu_tracer{
  char type;
  char rw;
  char varname[30];
  union {
    int32_t *data_l;
    float    *data_f;
  };
  union {
    int32_t data_lmin;
    float    data_fmin;
  };
  union {
    int32_t data_lmax;
    float    data_fmax;
  };
} mcu_tracer_t;


extern int32_t mainloop_iterations;
extern int32_t debug1;



//extern long green_ton, green_toff;

//Prototypes
void mcu_tracer_msg(const char* msg);
void mcu_tracer_inform(uint16_t addr);
void mcu_tracer_init(void);
void mcu_tracer_ping(void);
void mcu_tracer_emergency_reply(void);
void mcu_tracer_emergency(void);
void mcu_tracer_vals(void);
void mcu_tracer_update(uint16_t addr, int32_t val);
void mcu_tracer_fill(void);
void mcu_tracer_process(void);
void mcu_tracer_config(void);
void mcu_tracer_flush_buffer(void);
void mcu_tracer_reject_data_check(void);


#define MCU_TRACER_DATA_TYPE_INT 	1
#define MCU_TRACER_DATA_TYPE_FLOAT 	2
#define MCU_TRACER_DATA_TYPE_BOOL	3

#endif /* INCLUDES_MCU_TRACER_H_ */

#define MCU_TRACER_STARTBYTE 0xA5
#define PRINT_RS232_DATA 0

//
int monitor_test_connection(int number, int baud);
void * monitor_recieve_thread(void);
void monitor_recieve_terminate(void);
void monitor_master_allow_decoding(void);
void monitor_master_decode_string(unsigned char* buf, int len);
void monitor_master_req_init(void);
void monitor_master_req_data(void);
void monitor_master_write_var(uint16_t num, int32_t val);
void monitor_master_emergency(void);
void monitor_master_req_frequently_enable(void);
void monitor_master_req_frequently_disable(void);
void monitor_rs232_disconnect(void);

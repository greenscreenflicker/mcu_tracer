#define MCU_TRACER_STARTBYTE 0xA5

int test_connection(int number, int baud);
int monitor_master_order(int type, int pointer, long long data);
void monitor_rs232_flush_recieved_data(void);
void monitor_rs232_disconnect(void);
int monitor_init(void);
int monitor_master_streaming_start(void);
int monitor_master_streaming_stop(void);
int monitor_master_emergency_stop(void);
void monitor_master_get_variables(void);
struct set_variables* dummydata5(void);
struct set_variables* monitor_master_get_variables2(void);
//
int monitor_test_connection(int number, int baud);
void * monitor_recieve_thread(void);
void monitor_recieve_terminate(void);
void monitor_master_allow_decoding(void);
void monitor_master_decode_string(unsigned char* buf, int len);
void monitor_master_req_init(void);
void monitor_master_req_data(void);
void monitor_master_write_var(uint16_t num, int32_t val);
void monitor_master_req_frequently_enable(void);
void monitor_master_req_frequently_disable(void);

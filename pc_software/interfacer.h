#define MCU_TRACER_STARTBYTE 0xA5

int test_connection(int number, int baud);
int monitor_master_order(int type, int pointer, long long data);
void monitor_rs232_flush_recieved_data(void);
void monitor_rs232_disconnect(void);
int monitor_init(void);
int monitor_master_streaming_start(void);
int monitor_master_streaming_stop(void);
int monitor_master_emergency_stop(void);

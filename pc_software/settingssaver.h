
typedef struct settingsaver_connection{
	int comport;
	int baud;
} settingsaver_connection_t;

extern settingsaver_connection_t *settings_connection;

void settingssaver_load(void);
void settingssaver_store(void);

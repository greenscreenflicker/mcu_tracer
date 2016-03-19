#include "unified_header.h"


GKeyFile *key_file;
settingsaver_connection_t *settings_connection;


void settingsaver_get_home_directory(char* buf){

	struct passwd *pw = getpwuid(getuid());

	const char *homedir = pw->pw_dir;
	strcpy(buf,homedir);
}

void settingssaver_filename(char* filename){
	char homedir[200];
	settingsaver_get_home_directory(&homedir[0]);
	sprintf(filename,"%s/mcutracer.ini",&homedir[0]);
	//printf("would save to '%s'\n",filename);
	
}

void settingssaver_load(void){
	//printf("Loading settings\n");
	gboolean loadstat;
	GError *error=NULL;
	settings_connection=calloc(sizeof(settingsaver_connection_t),1);
	
	char filename[200];
	settingssaver_filename(&filename[0]);
	
	key_file=g_key_file_new ();
	loadstat=g_key_file_load_from_file (key_file,
                           filename,
						 G_KEY_FILE_KEEP_COMMENTS | 
						  G_KEY_FILE_KEEP_TRANSLATIONS,
                           &error);
                           
    if(!loadstat){ 
		printf("SettingsSaver Load:%s\n", error->message);
	}else{
		//printf("we can load\n");
		//we could load file
		settings_connection->comport = g_key_file_get_integer(key_file,
                                           "connection",
                                           "comport",
                                           &error);
		settings_connection->baud = g_key_file_get_integer(key_file,
                                           "connection",
                                           "baud",
                                           &error);
	}
	fflush(stdout);
	
}

void settingssaver_store(void){
	GError *error=NULL;
	char filename[200];
	settingssaver_filename(&filename[0]);
	
	//printf("Storing settings\n");
	if(!key_file){
		g_debug("SettingsSaver Store: Key file cannot be loaded");
		return;
	}
	g_key_file_set_integer (key_file,
                        "connection",
                        "comport",
                        settings_connection->comport);
	g_key_file_set_integer (key_file,
                        "connection",
                        "baud",
                        settings_connection->baud);
	gboolean storestat;
	storestat=g_key_file_save_to_file (key_file,
							 filename,
							 &error);
							 
	if(!storestat){ 
		printf("SettingsSaver Store:%s", error->message);
	}
	fflush(stdout);
}

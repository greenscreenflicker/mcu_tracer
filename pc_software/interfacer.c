#include "unified_header.h"

typedef struct set_variables{
	char label[30];
	int type; //0 notused (terminator), 1 long, 2 float, 3 toogle
	int rw;
	long data_l;
	double data_f;
	GtkWidget* label_widget;
	GtkWidget* data_widget;
} set_var_t;

int _comport=-1;

uint16_t xor_checksum(uint16_t *data, int length){
	//
	uint16_t checksum=0;
	int n;
	for(n=0;n<length;n++){
		checksum=checksum^(*data);
		data++;
		//printf("checksum is:%X",checksum);
	}
	return checksum;
}

unsigned char xor_checksum2(unsigned char *data, int length){
	//
	unsigned char checksum=0;
	int n;
	for(n=0;n<length;n++){
		checksum=checksum^(*data);
		data++;
		//printf("checksum is:%X",checksum);
	}
	return checksum;
}

void monitor_master_order_verbose(uint16_t *master_dataframe){
	//
	printf("Master Order: Startbit 0x%X %s\n",master_dataframe[0], ((master_dataframe[0] && 0xAA55) ? "ack" : "nak"));
	//Debug Mode:
	int type=(master_dataframe[1]& 0x00FF);
	if(type==0){
		printf("Send data:...\nPointer%i\n\t0x%X 0x%X 0x%X 0x%X\n",
			master_dataframe[2],
			master_dataframe[3],
			master_dataframe[4],
			master_dataframe[5],
			master_dataframe[6]
			);
	}else if(type==1){
		printf("Send init...\n");
	}else if(type==2){
		printf("Send start streaming\n");
	}else if(type==3){
		printf("Send stop streaming\n");
	}else if(type==4){
		printf("Send emergency stop\n");
	}else{
		printf("Error: Type not known.\n");
	}
	printf("Checksum: 0x%X\n",master_dataframe[7]);
	fflush(stdout);
}

int monitor_master_order(int type, int pointer, long long data){
	uint16_t *master_dataframe;
	master_dataframe=alloca(sizeof(uint16_t)*8);
	if((type<0)||(type>4)){
		printf("ERROR: monitor_master_order: Command invalid\n");
		return -1;
	}
	//Null all
	memset(master_dataframe,0,sizeof(uint16_t)*8);
	master_dataframe[0]=0xAA55; //Startbit
	master_dataframe[1]=0xAA00 | (0x00FF & type);
	if(type==0){
		printf("data is %x%x\n",(int)(data>>32),(int)data);
		master_dataframe[2]=(pointer && 0xFFFF); // Fill pointer to string
		master_dataframe[3]=((data>>(16*3))& 0xFFFF);
		master_dataframe[4]=((data>>(16*2))& 0xFFFF);
		master_dataframe[5]=((data>>(16*1))& 0xFFFF);
		master_dataframe[6]=((data)& 0xFFFF);
	}
	master_dataframe[7]=xor_checksum(master_dataframe,7);
	monitor_master_order_verbose(master_dataframe);
	
	//ToDO: Impelement sending here!
	//check if serial port opened
	if(_comport>-1){
		//comport initalisiert
		int returnval;

		returnval=RS232_SendBuf(_comport,(char*) master_dataframe,sizeof(uint16_t)*8);
		if(returnval==(sizeof(uint16_t)*8)){
			return 1;
		}else{
			return -1;
		}
	}else{
		printf("Warning: Comport not initalisized.\n");
	}
	return -1;
}

//convienience Functions
int monitor_master_data(int pointer, long long data){
	return monitor_master_order(0,pointer,data);
}

int monitor_init(void){
	//More to do!
	return monitor_master_order(1,0,0);
}

int monitor_master_streaming_start(void){
	return monitor_master_order(2,0,0);
}

int monitor_master_streaming_stop(void){
	return monitor_master_order(3,0,0);
}

int monitor_master_emergency_stop(void){
	return monitor_master_order(4,0,0);
}


void mcu_tracer_init(void){
	/*
	unsigned char buffer[20];
	buffer[0]=MCU_TRACER_STARTBYTE;
	buffer[1]=0x01; // Init byte
	buffer[2]=buffer[0]^buffer[1];
	printf("sending %x,%x,%x\n",buffer[0],buffer[1],buffer[2]);
	RS232_SendBuf(_comport,buffer,3);
	*/
	RS232_SendByte(_comport,MCU_TRACER_STARTBYTE);
	RS232_SendByte(_comport,1);
	RS232_SendByte(_comport,0xa4);
}

int monitor_master_find_startbyte(unsigned char* buf, int len){
	int i;
	for(i=0; i < len; i++)
	{
		if(buf[i]==MCU_TRACER_STARTBYTE){
			//printf("found startbyte at %i\n",i);
			return i;
		}
	}
	return -1; //noting found
}

int monitor_master_check_parity(unsigned char* buf, int len){
	int startbyte=monitor_master_find_startbyte(buf,len);
	unsigned char checksum;
	checksum=xor_checksum2(buf+startbyte,len-startbyte);
	if(checksum==0){
		//printf("integrity: no error detected");
		return 1;
	}else{
		//printf("integrity: error detected");
		return 0;
	}
}

void monitor_master_decode_string(unsigned char* buf, int len){
	int startbyte=monitor_master_find_startbyte(buf, len);
	if(startbyte==-1){
		//Error
		printf("startbyte was not found. not a legic msg. Ignoring.\n");
		return;
	}
	int order=buf[startbyte+1];
	if(order==1){
		int decodepos=startbyte+1;
		char variablename[100];
		do{
			int type=buf[decodepos++];
			if(type==0) break;
			int rw=buf[decodepos++];
			int copy=0;
			while(buf[decodepos]!=1){
				if(decodepos>len){
					printf("error copying stringname\n");
					return;
				}
				variablename[copy++]=buf[decodepos++];
			}
			variablename[copy]=0;
			printf("Variable '%s', type:%i RW:%i decodepos:%i\n",variablename,type,rw,decodepos);
			decodepos++;
		}while(1);
	}else{
		printf("Order unkown, ignoring msg.");
	}
}

void print_hex_data(unsigned char* buf, int len){
	int i;
	printf("[%i]:",len);
	for(i=0; i < len; i++){
		printf("%x-",buf[i]);
	}
	printf("\n");
	fflush(stdout);
 }

int monitor_master_pack_data(unsigned char *data,unsigned char* ret, int len){
	unsigned char buf[4096];
	RS232_PollComport(_comport, buf, 4095); //check buffer is empty
	
	buf[0]=MCU_TRACER_STARTBYTE;
	memcpy(&buf[1],data,len);
	buf[1+len]=xor_checksum2(data,len)^MCU_TRACER_STARTBYTE;
	RS232_SendBuf(_comport,buf,len+2);
	print_hex_data(buf,len+2);
	usleep(10000);
	int n;
	int upcount=0;
	int retry=10;
	while((retry--)>0){
		while(upcount<10000){
			n = RS232_PollComport(_comport, buf, 4095);
			if(n > 0){
				int start=monitor_master_find_startbyte(buf,n);
				//printf("startpos %i\n",start);
				int parityok=monitor_master_check_parity(&buf[0],n);
				if(parityok==1){
					//Copy data to returnstring
					printf("fr:");
					print_hex_data(&buf[0],n);
					//remove first byte and checksum
					memcpy(ret,buf+start+1,n-start-2);
					return (n-start-2);
				}else{
					printf("parity failed\n");
					break;
				}
			}
			
		}
		printf("got no answer, retrying...\n");
		RS232_SendBuf(_comport,buf,len+2);
	}
	return -1;
}

int monitor_master_rec_data(unsigned char *buf){
	//set_variables_data=calloc
}

struct set_variables* dummydata5(void){
	struct set_variables *mydd;
	//important to use calloc
	
	mydd=calloc(sizeof(set_var_t),100);
	
	strcpy(mydd[0].label,"Long2");
	mydd[0].type=1;
	mydd[0].data_l=1234;
	strcpy(mydd[1].label,"Float");
	mydd[1].type=2;
	mydd[1].data_f=5678.4;
	strcpy(mydd[2].label,"Toggle A");
	mydd[2].type=3;
	mydd[2].data_l=1;
	strcpy(mydd[3].label,"Toggle D");
	mydd[3].type=3;
	mydd[3].data_l=0;
	strcpy(mydd[4].label,"Toggle Dn");
	mydd[4].type=3;
	mydd[4].rw=1;
	mydd[4].data_l=0;
	strcpy(mydd[5].label,"Float");
	mydd[5].type=2;
	mydd[5].rw=1;
	mydd[5].data_f=5678.4;
	
	return mydd;
}

struct set_variables* monitor_master_get_variables2(void){
	unsigned char data[]={1};
	unsigned char retbuf[5000];

	int len=monitor_master_pack_data(data,retbuf,1);
	printf("rec:");
	print_hex_data(retbuf,len);
	
	
	struct set_variables *mydd;
	//important to use calloc
	mydd=calloc(sizeof(set_var_t),100);
	
	int order=retbuf[0];
	printf("order: %i\n",order);
	if(order==1){
		int decodepos=1;
		char variablename[100];
		int iteration=0;
		do{
			int type=retbuf[decodepos++];
			if(type==0) break;
			int rw=retbuf[decodepos++];
			int copy=0;
			while(retbuf[decodepos]!=1){
				if(decodepos>len){
					printf("error copying stringname\n");
					return;
				}
				variablename[copy++]=retbuf[decodepos++];
			}
			variablename[copy]=0;
			printf("Variable '%s', type:%i RW:%i decodepos:%i\n",variablename,type,rw,decodepos);
			
			if(strlen(variablename)>29){
				printf("variable string too long\n");
			}
			strcpy(mydd[iteration].label,variablename);
			mydd[iteration].type=type;
			mydd[iteration].rw=rw;
			mydd[iteration].data_l=0;
			mydd[iteration].data_f=0;
			
			iteration++;
			decodepos++;
		}while(1);
	}else{
		printf("Order unkown, ignoring msg.");
	}

	

	return mydd;
}

void monitor_master_get_variables(void){
	unsigned char data[]={1};
	unsigned char retbuf[5000];
	set_var_t *data_array;

	int len=monitor_master_pack_data(data,retbuf,1);
	printf("rec:");
	print_hex_data(retbuf,len);
	
	
	int order=retbuf[0];
	printf("order: %i\n",order);
	if(order==1){
		int decodepos=1;
		char variablename[100];
		int iteration=0;
		do{
			int type=retbuf[decodepos++];
			if(type==0) break;
			int rw=retbuf[decodepos++];
			int copy=0;
			while(retbuf[decodepos]!=1){
				if(decodepos>len){
					printf("error copying stringname\n");
					return;
				}
				variablename[copy++]=retbuf[decodepos++];
			}
			variablename[copy]=0;
			printf("Variable '%s', type:%i RW:%i decodepos:%i\n",variablename,type,rw,decodepos);
			
			if(strlen(variablename)>29){
				printf("variable string too long\n");
			}
	
			iteration++;
			decodepos++;
		}while(1);
	}else{
		printf("Order unkown, ignoring msg.");
	}
	
}

int test_connection(int number, int baud){
	 int i, n,
	  cport_nr=number,        
	  bdrate=baud;      

	unsigned char buf[4096];
	printf("up (com:%i b:%i)\n",cport_nr,bdrate);
	char mode[]={'8','N','1',0};


	if(RS232_OpenComport(cport_nr, bdrate, mode)){
		printf("Can not open comport\n");
		return(-1);
	}
	//Save comport number
	_comport=number;
	char connection_str[]={MCU_TRACER_STARTBYTE,1,MCU_TRACER_STARTBYTE^1};
	RS232_PollComport(_comport, buf, 4095);
	RS232_SendBuf(_comport,connection_str,3);
	//usleep(10000);
	int il=0;
	while((il++)<100){
		n = RS232_PollComport(_comport, buf, 4095);
		if(n > 0){
			buf[n] = 0; 
			//monitor_master_pass_string(buf, n);
			/*
			printf("received %i bytes: %s\n", n, (char *)buf);
			for(i=0; i < n; i++)
			{
				printf("%x-",buf[i]);
			}*/
			int parityok=monitor_master_check_parity(&buf[0],n);
			if(parityok==1){
				return 1;
			}
			
		
		}
		if(n==27) break;

		#ifdef _WIN32
			Sleep(1000);
		#else
			usleep(10000);  /* sleep for 100 milliSeconds */
		#endif
		RS232_SendBuf(_comport,connection_str,3);
	}
	fflush(stdout);
	RS232_CloseComport(_comport);
	_comport=-1;
	return(-2);
}



int monitor_rs232_connect(int number, int baud){
	if(_comport>-1) return;
	int cport_nr=number,bdrate=baud;       

	char mode[]={'8','N','1',0};
	printf("openening port %i\n",number);
	fflush(stdout);


	//Save comport number
	_comport=number;

	if(RS232_OpenComport(cport_nr, bdrate, mode)){
		printf("Can not open comport\n");
		fflush(stdout);
		return -1;
	}
	return 1;
}

void monitor_rs232_disconnect(void){
	if(_comport>=0){
		RS232_CloseComport(_comport);
	}else{
		printf("Cannot close an not opened comport.\n");
	}
}

void monitor_rs232_flush_recieved_data(void){
	//
	int size;
	char* data=alloca(40); //using alloca not to worry about freeing it
	size=RS232_PollComport(_comport,(char*)data,40);
	data[size]=0;
	//printf("received %i bytes: %s\n", size, (char *)data);
	
	//lass uns das zu 16 bit casten, damit wir das leichter verarbeiten koennen
	uint16_t *recieved;
	recieved=(uint16_t*)data;
	int bitsize=size/2;
	int n;
	for(n=0;n<bitsize;n++){
		printf("0x%X-",*recieved);
		recieved=recieved+1;
	}
	printf("\n");
}



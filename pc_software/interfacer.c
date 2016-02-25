#include "unified_header.h"

//Implementing the communication protocol for Monitor.

//Startword   0xAA55 (16)
//Commandbyte 0xAA (8)
//Command: 	0: set Value (8)
//			1: send Init (8)
//			2: Start (8)
//			3; Stop (8)
//			4: Emergency Stop (8)
//Array Index: 	n 16 Bit (basically the pointer to the variable)
//Long for Data
//Data high 	n 16 Bit (high)
//Data high		n 16 Bit
//Data low		n 16 Bit
//Data low		n 16 Bit
//Checksum = XOR(ALL)

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


int test_connection(int number, int baud){
	int i=0,
	cport_nr=number,        /* /dev/ttyS0 (COM1 on windows) */
	bdrate=baud;       /* 9600 baud */

	char mode[]={'8','N','1',0},
	str[512];
	printf("openening port %i\n",number);
	fflush(stdout);

	strcpy(str, "serial working test.\n");

	//Save comport number
	_comport=number;

	if(RS232_OpenComport(cport_nr, bdrate, mode)){
		printf("Can not open comport\n");
		fflush(stdout);
		return -1;
	}
	
	//empty comport
	unsigned char buf[4096];
	RS232_PollComport(cport_nr, buf, 4095);
	
	RS232_cputs(cport_nr, str);
	
	int n;
	while(1){
		n = RS232_PollComport(cport_nr, buf, 4095);
		if(n > 0){
			buf[n] = 0;   /* always put a "null" at the end of a string! */
			printf("received %i bytes: %s\n", n, (char *)buf);
			break;
		}
		usleep(100);
		i++;
		if(i>1000){
			RS232_CloseComport(cport_nr);
			return -2;
		}
	}
	RS232_CloseComport(cport_nr);
		//Save comport number
	_comport=-1;
	return 1;
	
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



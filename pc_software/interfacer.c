#include "unified_header.h"

int _comport=-1;
int _run_rec_thread;

GMutex *lock_rs232; 
GMutex *lock_paritycheckbit;
GMutex *lock_rec_thread;




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
	if(startbyte<0) return 0; //startbyte not found
	unsigned char checksum=xor_checksum2(buf+startbyte,len-startbyte);
	if(checksum==0){
		//printf("integrity: no error detected");
		return 1;
	}else{
		//printf("integrity: error detected");
		return 0;
	}
}



void print_hex_data(unsigned char* buf, int len){
	int i;
	printf("[%i]:",len);
	for(i=0; i < len; i++){
		printf("%x(%c)-",buf[i],buf[i]);
	}
	printf("\n");
	fflush(stdout);
 }





///////////////////////////////////////////////////////
//interfacer for threads
int monitor_parity_is_working;
int recieve_debug_window_active=0;
int monitor_rs232_rec_terminate=0;
int monitor_rs232_req_terminate=2;
pthread_mutex_t plock_rs232= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plock_rs232_parity_check= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plock_rs232_terminate= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plock_rs232_req= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plock_rec_debug_window= PTHREAD_MUTEX_INITIALIZER;
pthread_t th_rec, th_req;

//sends data to the mcu, run from gtk thread
int monitor_test_connection(int number, int baud){
	 int i, n,
	  cport_nr=number,        
	  bdrate=baud;      

	unsigned char buf[4096];
	#if PRINT_RS232_DATA==1
	printf("up (com:%i b:%i)\n",cport_nr,bdrate);
	#endif
	char mode[]={'8','N','1',0};


	if(RS232_OpenComport(cport_nr, bdrate, mode)){
		printf("Can not open comport\n");
		return(-1);
	}
	//Save comport number
	_comport=number;
      
   
	pthread_t tr, tw;
	//casting is needed to avoid warning, though not beautiful.
	 pthread_create(&th_rec,NULL,(void * (*)(void *))monitor_recieve_thread,NULL);
   
	char connection_str[]={MCU_TRACER_STARTBYTE,0,MCU_TRACER_STARTBYTE};
	int parityok=0;
	
	i=1000;
	while((i--)>0){
		if((i%100)==0){
			pthread_mutex_lock(&plock_rs232);
			RS232_SendBuf(_comport,connection_str,3);
			pthread_mutex_unlock(&plock_rs232);
		}
		/*
		g_mutex_lock (lock_rs232);
		RS232_SendBuf(_comport,connection_str,3);
		g_mutex_unlock (lock_rs232);
		
		g_mutex_lock (lock_paritycheckbit);
		parityok=monitor_parity_is_working;
		g_mutex_unlock (lock_paritycheckbit);
		*/
		usleep(1000);
		pthread_mutex_lock(&plock_rs232_parity_check);
		parityok=monitor_parity_is_working;
		pthread_mutex_unlock(&plock_rs232_parity_check);
		if(parityok==1) return 1; //report our success
	}
	
	//monitor_recieve_thread_terminate();
	monitor_recieve_terminate();
	fflush(stdout);
	RS232_CloseComport(_comport);
	_comport=-1;
	return(-2);
}

int monitor_master_send(unsigned char *data, int len){
	unsigned char buf[4096];

	
	buf[0]=MCU_TRACER_STARTBYTE;
	memcpy(&buf[1],data,len);
	buf[1+len]=xor_checksum2(data,len)^MCU_TRACER_STARTBYTE;
	#if PRINT_RS232_DATA==1
	printf("Send:");
	print_hex_data(&buf[0],len+2);
	printf("\n");
	#endif
	//printf("\n");
	return RS232_SendBuf(_comport,buf,len+2);
}

void monitor_master_req_init(void){
	//requests init data;
	unsigned char data[]={1};
	monitor_master_send(data,1);
}

void monitor_master_req_data(void){
	//requests data stream of all data;
	unsigned char data[]={2};
	monitor_master_send(data,1);
}

void monitor_master_write_var(uint16_t num, int32_t val){
	//writes value to the mcu
	unsigned char data[7]={3,1,0,0,0,0,0};
	data[1]=(unsigned char)(num>>8);
	data[2]=(unsigned char)(num);
	data[3]=(unsigned char)(val>>8*3);
	data[4]=(unsigned char)(val>>8*2);
	data[5]=(unsigned char)(val>>8*1);
	data[6]=(unsigned char)(val>>8*0);
	monitor_master_send(data,7);
}

void monitor_master_func_data(void){
	unsigned char data[]={0x08};
	monitor_master_send(data,1);
}


void monitor_master_func_exec(uint8_t id){
	unsigned char data[2]={0x09,id};
	monitor_master_send(data,2);
}


void monitor_master_emergency(void){
	//requests data stream of all data;
	unsigned char data[]={0xFF};
	monitor_master_send(data,1);
}


void monitor_master_req_frequently(void){
	//
	int request_terminate=0;
	while(request_terminate==0){
		usleep(100000);
		monitor_master_req_data();
		pthread_mutex_lock(&plock_rs232_req);
		request_terminate=monitor_rs232_req_terminate;
		pthread_mutex_unlock(&plock_rs232_req);
	}
	pthread_mutex_lock(&plock_rs232_req);
	monitor_rs232_req_terminate=2;
	pthread_mutex_unlock(&plock_rs232_req);
	//printf("reqthread terminated\n");
}

void monitor_master_req_frequently_enable(void){
	int request_terminate;
	pthread_mutex_lock(&plock_rs232_req);
	request_terminate=monitor_rs232_req_terminate;
	pthread_mutex_unlock(&plock_rs232_req);
	if(request_terminate==2){
		monitor_rs232_req_terminate=0;
		pthread_create(&th_req,NULL,(void * (*)(void *))monitor_master_req_frequently,NULL);
		//printf("reqthread created\n");
	}
}

void monitor_master_req_frequently_disable(void){
	pthread_mutex_lock(&plock_rs232_req);
	monitor_rs232_req_terminate=1;
	pthread_mutex_unlock(&plock_rs232_req);
}


void monitor_recieve_terminate(void){
	pthread_mutex_lock(&plock_rs232_terminate);
	monitor_rs232_rec_terminate=1;
	pthread_mutex_unlock(&plock_rs232_terminate);
}

void monitor_master_allow_decoding(void){
	pthread_mutex_lock(&plock_rec_debug_window);
	recieve_debug_window_active=1;
	pthread_mutex_unlock(&plock_rec_debug_window);
}

unsigned char workingbuffer[10000];
unsigned char iterater[10000];
int portionierer_current_buffer_pos=0;

void portionierer_process(unsigned char *data, int len){
	//printf("i would process this:");
	//print_hex_data(data, len);
	if(len<1) exit(1);
	if(len<3){
		printf("Portionierer: Here must be something wrong\n");
	}
	#if PRINT_RS232_DATA==1
	printf("REC:");
	print_hex_data(&data[0],len);
	printf("\n");
	#endif
	monitor_master_decode_string(data,len);
}


int monitor_master_check_parity_dump(unsigned char* buf, int len){
	unsigned char checksum=xor_checksum2(buf,len-1);
	if(checksum==buf[len-1]){
		//printf("integrity: no error detected");
		return 1;
	}else{
		//printf("integrity: 0x%x expected but is 0x%x\n",checksum,buf[len-1]);
		return 0;
	}
}

void portionierer2_move_mem(unsigned char *start,int pos){
	memcpy(&iterater[0],&workingbuffer[pos],portionierer_current_buffer_pos-pos);
	memcpy(&workingbuffer[0],&iterater[0],portionierer_current_buffer_pos-pos);
	portionierer_current_buffer_pos=portionierer_current_buffer_pos-pos;
}

void monitor_master_portionierer2(unsigned char *data, int len){
	//Copy data in our area, so we can iterate over it
	memcpy(&workingbuffer[portionierer_current_buffer_pos],data,len);
	portionierer_current_buffer_pos=portionierer_current_buffer_pos+len;
	int startpos,endpos,endpos_ret;
	
	int countstartbytes;

	//printf("\n\nbuffer:");
	//print_hex_data(&workingbuffer[0],portionierer_current_buffer_pos);	
	PORTIONERER_CONTINUE_PROCESS:
	countstartbytes=0;
	//looking for startbyte
	startpos=monitor_master_find_startbyte(&workingbuffer[0],portionierer_current_buffer_pos);
	if(startpos==-1){
		//no data was found, we delete all
		//printf("Portionier: We only got garbage-deleting it\n");
		portionierer_current_buffer_pos=0;
		return;
	}
	

	endpos=startpos+1;
	while(1){
		endpos_ret=monitor_master_find_startbyte(&workingbuffer[endpos],portionierer_current_buffer_pos-endpos);
		if(endpos_ret==-1){
			//we have nothing more to come
			int ret;
			ret=monitor_master_check_parity_dump(&workingbuffer[startpos],portionierer_current_buffer_pos-startpos);
			if(ret){
				//we can use result;
				///printf("Portionierer: This last part is valid.\n");
				portionierer_process(&workingbuffer[startpos],portionierer_current_buffer_pos-startpos);
				//free buffer
				portionierer_current_buffer_pos=0;
				return;
			}else{
				//result is unusable, maybe there's more to come;
				//wait for a later call to come more
				//printf("Portionier: Nothing more to process\n");
				return;
			}
		}else{
			endpos_ret=endpos+endpos_ret;
			//printf("Portionier: We have a possible datastring inbetween %i-%i\nDatastring:",startpos,endpos_ret);
			//print_hex_data(&workingbuffer[startpos],endpos_ret-startpos);
			int ret;
			ret=monitor_master_check_parity_dump(&workingbuffer[startpos],endpos_ret-startpos);
			if(ret){
				//printf("Portionier: String valid\n");
				portionierer_process(&workingbuffer[startpos],endpos_ret-startpos);
				//remove string from data
				portionierer2_move_mem(&workingbuffer[endpos_ret],endpos_ret);
				//printf("->new data buffer looks like this\nworker:");
				//print_hex_data(&workingbuffer[0],portionierer_current_buffer_pos);
				//check if we have more data to process
				goto PORTIONERER_CONTINUE_PROCESS;
				//return;				
			}else{
				//printf("Portionier: Looking for next string\n");
				endpos=endpos_ret+1;
				
				if(countstartbytes>5){
					//printf("Portionier: we possibly have wrong integrity bit\n");
					//lets find second start byte
					startpos=monitor_master_find_startbyte(&workingbuffer[0],portionierer_current_buffer_pos);
					startpos=monitor_master_find_startbyte(&workingbuffer[startpos+1],portionierer_current_buffer_pos-startpos)+startpos+1;
					portionierer2_move_mem(&workingbuffer[startpos],startpos);
					//printf("(%i)new data buffer looks like this\ncrop:",startpos);
					//print_hex_data(&workingbuffer[0],portionierer_current_buffer_pos);
					//check if we have more data to process
					goto PORTIONERER_CONTINUE_PROCESS;					
				}
				countstartbytes=countstartbytes+1;
			}
		}
	}
	
}

void * monitor_recieve_thread(void){
	//recieves data;
	unsigned char buf[4096];
	int parityok=0;
	int terminate=0;
	pthread_mutex_lock(&plock_rs232_terminate);
	monitor_rs232_rec_terminate=0;
	pthread_mutex_unlock(&plock_rs232_terminate);
	while(terminate==0){
		pthread_mutex_lock(&plock_rs232);
		int n = RS232_PollComport(_comport, buf, 4095);
		pthread_mutex_unlock(&plock_rs232);
		if(n > 0){
			buf[n] = 0; 
			#if PRINT_RS232_DATA==1
			printf("recieved:");
			print_hex_data(&buf[0],n);
			#endif
			
			int may_we_decode;
			pthread_mutex_lock(&plock_rec_debug_window);
			may_we_decode=recieve_debug_window_active;
			pthread_mutex_unlock(&plock_rec_debug_window);
			if(may_we_decode==1){
				//
				//monitor_master_decode_string(&buf[0],n);
				monitor_master_portionierer2(&buf[0],n);
			}else{
				parityok=monitor_master_check_parity(&buf[0],n);
				pthread_mutex_lock(&plock_rs232_parity_check);
				monitor_parity_is_working=parityok;
				pthread_mutex_unlock(&plock_rs232_parity_check);
			}
			
		}
		usleep(1000);
		
		//check if we're allowed to decode our recieved string

		
		
		pthread_mutex_lock(&plock_rs232_terminate);
		terminate=monitor_rs232_rec_terminate;
		pthread_mutex_unlock(&plock_rs232_terminate);
	}        
	

	//printf("rec thread terminated\n");
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
		struct set_variables *mydd;
		mydd=calloc(sizeof(set_var_t),len/4); //generously overestimate number of elements we will need
		int decodepos=startbyte+2;
		int element=0;
		char variablename[100];
		do{
			int type=buf[decodepos++];
			if(type==0) break;
			int rw=buf[decodepos++];
			int copy=0;
			while(buf[decodepos]!=1){
				if(decodepos>len){
					printf("Order1: error copying stringname\n");
					return;
				}
				variablename[copy++]=buf[decodepos++];
			}
			variablename[copy]=0;
			//printf("Variable '%s', type:%i RW:%i element:%i\n",variablename,type,rw,element);
			
			//copy stringname
			strcpy(mydd[element].label,variablename);
			mydd[element].type=type;
			mydd[element].rw=rw;
			mydd[element].data_l=0;
			element=element+1;
			//force update to gtk
			decodepos++;
        
		}while(1);
		element=element+1;
		strcpy(mydd[element].label,"empty");
		mydd[element].type=0;
		mydd[element].rw=0;
		mydd[element].data_l=0;
		//say_hello();
		//variables_window_update();
		//important to use calloc
		
		inject_call((GSourceFunc)variables_window_update, mydd);
	}else if(order==2){
		int elementstodecode=(len-startbyte-3)/4;
		//printf("we have %i elements to decode\n",elementstodecode);
		uint32_t *data=calloc(sizeof(uint32_t),elementstodecode+1);
		int decodepos=startbyte+2;
		int i;
		for(i=0;i<elementstodecode;i++){
			data[i]=(
				(buf[2+i*4]<<(8*3))+
				(buf[3+i*4]<<(8*2))+
				(buf[4+i*4]<<(8*1))+
				(buf[5+i*4]<<(8*0)));
			//printf("%i:%i(%x-%x-%x-%x)\n",i,data[i],buf[2+i*4],buf[3+i*4],buf[4+i*4],buf[5+i*4]);
		}
		inject_call((GSourceFunc)variables_window_update_vars, data);
		//
	}else if(order==3){
		//updating single variable+
		if(len==(3+6)){
			set_single_var_t *singlevar;
			singlevar=malloc(sizeof(set_single_var_t));
			singlevar->addr=(buf[2]<<8)+(buf[3]<<0);
			singlevar->val=(buf[4]<<8*3)+(buf[5]<<8*2)+(buf[6]<<8*1)+(buf[7]<<8*0);
			//senprintf("addr:%i|data:%i\n",singlevar->addr,singlevar->val);
			inject_call((GSourceFunc)variables_window_update_single_var, singlevar);
		}
	}else if(order==0x08){
		//function init reply
		mcu_func_t *my_mcu_func=calloc(sizeof(mcu_func_t),len/4+1);
		int countfunc=0;
		char *msg;
		int decodepos=startbyte+2;
		//print_hex_data(&buf[0],len);
		do{
			my_mcu_func[countfunc].id=buf[decodepos++];
			char *msg=my_mcu_func[countfunc].name;
			int copy=0;
			while(buf[decodepos]!=1){
				if(decodepos>len || decodepos>(FUNC_NAME_LENGTH-1)){
					printf("Order8: error copying stringname\n");
					return;
				}
				msg[copy++]=buf[decodepos++];
			}
			msg[copy]=0; //terminator
			decodepos++;
			//printf("Func %i:%sDecode:%i\n",my_mcu_func[countfunc].id,msg,decodepos);
			if(my_mcu_func[countfunc].id==0) break;
		
			countfunc++;
		}while(countfunc<254);
		//transfer data
		inject_call((GSourceFunc)FuncOnMCU_update, my_mcu_func);
	}else if(order==9){
		//todo: Process this confirmation
		int decodepos=startbyte+2;
		int function=buf[decodepos++];
		int funcstatus=buf[decodepos++];
		
		if(!(funcstatus==1)){
			timeout_reset_of_fail(function);
			func_report_execution_fail(function,funcstatus);
		}else{
			//we can reactivate call
			func_reset_register_callback(function);
		}
	}else if(order==0xFE){
		//We recieved a msg from MCU
		char *msg=malloc(sizeof(char)*1000);
		int decodepos=startbyte+2;
		int copy=0;
		while(buf[decodepos]!=1){
			if(decodepos>len){
				printf("OrderFE:error copying stringname\n");
				return;
			}
			msg[copy++]=buf[decodepos++];
		}
		msg[copy]=0;
		//printf("MCU told '%s'\n",msg);

		inject_call((GSourceFunc)gui_msg_center_add_msg, msg);
	}else if(order==0xFF){
		char *msg=malloc(sizeof(char)*1000);
		strcpy(msg,"MCU send emergency code");
		inject_call((GSourceFunc)gui_msg_center_add_msg, msg);
	}else if(order==0){
		//ping, not yet implemented
	}else{
		char *msg=malloc(sizeof(char)*1000);
		sprintf(msg,"Order 0x%x unkown, ignoring msg.",order);
		inject_call((GSourceFunc)gui_msg_center_add_msg, msg);
		fflush(stdout);
	}
}


void monitor_rs232_disconnect(void){
	monitor_recieve_terminate();
	monitor_master_req_frequently_disable();
	if(_comport>=0){
		RS232_CloseComport(_comport);
	}else{
		printf("Cannot close an not opened comport.\n");
	}
}



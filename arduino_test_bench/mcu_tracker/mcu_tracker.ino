#include <stdint.h>
#define MCU_TRACER_STARTBYTE 0xA5


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

mcu_tracer_t monitorvars[10];

int global_checksum;
uint8_t mcu_tracer_checksum;
int32_t debug1, debug2;
#define MONITOR_ELEMENTS (sizeof(monitorvars)/sizeof(mcu_tracer_t))

void mcu_tracer_fill(void){
  monitorvars[0].type=1;
  monitorvars[0].rw=0;
  monitorvars[0].data_l=&debug1;
  strcpy(monitorvars[0].varname,"Hallo");
  monitorvars[0].data_lmin=INT32_MIN;
  monitorvars[0].data_lmax=INT32_MAX;

  monitorvars[1].type=1;
  monitorvars[1].rw=0;
  monitorvars[1].data_l=&debug2;
  strcpy(monitorvars[1].varname,"Hallo2");
  monitorvars[1].data_lmin=0;
  monitorvars[1].data_lmax=1;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Up and running");
 // Serial.print(monitor_elements,DEC);
  mcu_tracer_fill();

  //DEBUG LED
  pinMode(13, OUTPUT);
}

uint8_t rec_char(void){
      while(!(Serial.available()));
      uint8_t ch;
      ch=Serial.read();
      mcu_tracer_checksum=ch^mcu_tracer_checksum;
      return ch;
}

uint8_t rec_checksum(void){
  while(!(Serial.available()));
  uint8_t ch;
  ch=Serial.read();
  if(mcu_tracer_checksum==ch){
    return 1;
  }else{
    Serial.println("checksum expected:");
    Serial.print(mcu_tracer_checksum,HEX);
    Serial.println("<-");
    return 0; 
  }
}

void mcu_tracer_process(void){
  mcu_tracer_checksum=0;
  while(1){
    if(!(Serial.available())){
      //We have no data, we're done
      return;
    }
    uint8_t startbyte=rec_char();
    if(!(startbyte==MCU_TRACER_STARTBYTE)){
      //we have no startbyte
      mcu_tracer_checksum=0;
      //Serial.println("not a start byte");
      //Serial.print(startbyte,HEX);
      continue;
    }else{
      //Serial.println("Recieved startbyte");
    }
    //Now we synced to the startbyte

    uint8_t order=rec_char();
    //Serial.print("Order:");
    //Serial.print(order,HEX);
    //Serial.print("\n");
    if(order==1){
      //Serial.println("Order one");
      //we have a Init request
      //we need to check mcu_tracer_checksum now
      
      if(rec_checksum()){
        // call the init routine;
        //erial.println("Checksum ok");
        mcu_tracer_init();
      }
    }else if(order==2){
      if(rec_checksum()){
        mcu_tracer_vals();
      }
    }else if(order==3){
      uint32_t data=0;
      uint16_t arraynumber=0;
      uint8_t rec;
      int i;
      //load arrynumber
      for(i=2;i>0;i--){
        arraynumber=(arraynumber<<8);
        rec=rec_char();
        arraynumber=(rec+arraynumber);
      }
      //load data
      for(i=4;i>0;i--){
        data=(data<<8);
        rec=rec_char();
        data=(rec+data);
      }
      if(rec_checksum()){
        mcu_tracer_update(arraynumber,data);
      }
    }else{
      //Serial.println("Order unkown");
      //Serial.print(order,HEX);
    }
  }
}

void mcu_tracer_write_serial(char data){
  global_checksum=global_checksum^data;
  Serial.write(data);
}

void mcu_tracer_write_string(const char* data){
   while( *data != '\0' ){
      mcu_tracer_write_serial(*data++ );
   }
   mcu_tracer_write_serial(1);
}

void mcu_tracer_send_checksum(void){
  Serial.write(global_checksum);
  global_checksum=0;
}
void mcu_tracer_init(void){
  int i;
  mcu_tracer_write_serial(MCU_TRACER_STARTBYTE);
  mcu_tracer_write_serial(1); //Order type 1: Sending init
  for(i=0; i<MONITOR_ELEMENTS; i++){
    //check if we have a terminator =>if, break
    if(monitorvars[i].type==0) break;
    mcu_tracer_write_serial(monitorvars[i].type);
    mcu_tracer_write_serial(monitorvars[i].rw);
    mcu_tracer_write_string(monitorvars[i].varname);
  }
  
  //sending terminating byte
  mcu_tracer_write_serial(0); //last byte
  mcu_tracer_write_serial(0); //read only
  mcu_tracer_write_string("");

  mcu_tracer_send_checksum();
}

//Sends actual values to pc
void mcu_tracer_vals(void){
  mcu_tracer_write_serial(MCU_TRACER_STARTBYTE);
  mcu_tracer_write_serial(2); //order two, we transfer variables
  char i;
  for(i=0; i<MONITOR_ELEMENTS; i++){
    //check if we have a terminator =>if, break
    if(monitorvars[i].type==0) break;
    uint32_t data;
    data=*(monitorvars[i].data_l);
    mcu_tracer_write_serial(data>>(3*8));
    mcu_tracer_write_serial(data>>(2*8));
    mcu_tracer_write_serial(data>>(1*8));
    mcu_tracer_write_serial(data>>(0*8));
  }
  mcu_tracer_send_checksum();
}

//Updates the value in the register
void mcu_tracer_update(uint16_t addr, int32_t val){
  if(addr>MONITOR_ELEMENTS){
    //error, we do not have this addr
    return;
  }

  if(monitorvars[addr].type==1){
    //Integer
    int32_t *toupdate;
    toupdate=monitorvars[addr].data_l; //getting mcu address of variable
    if(val>monitorvars[addr].data_lmax){
      val=monitorvars[addr].data_lmax;
    }
    if(val<monitorvars[addr].data_lmin){
      val=monitorvars[addr].data_lmin;
    }

    //memcpy(monitorvars[addr].data_l,&val,4);

   
    *(monitorvars[addr].data_l)=val;
     /*
    *(monitorvars[addr].data_l+1)=(uint8_t)(val<<8);
    *(monitorvars[addr].data_l+1)=(uint8_t)(val<<8);*/    
  }else{
    Serial.println("type not yet supported");
  }
  mcu_tracer_inform(addr);
}

void mcu_tracer_inform(uint16_t addr){
  if(addr>MONITOR_ELEMENTS) return; //we do not have this.
  int32_t val=*(monitorvars[addr].data_l);

  mcu_tracer_write_serial(MCU_TRACER_STARTBYTE);
  mcu_tracer_write_serial(3); //order 3, we transfer single value
  mcu_tracer_write_serial(addr>>(1*8));
  mcu_tracer_write_serial(addr>>(0*8));
  mcu_tracer_write_serial(val>>(3*8));
  mcu_tracer_write_serial(val>>(2*8));
  mcu_tracer_write_serial(val>>(1*8));
  mcu_tracer_write_serial(val>>(0*8));
  mcu_tracer_send_checksum();
}
void loop() {
  // put your main code here, to run repeatedly:
  mcu_tracer_process();
  if(debug1==0){
    digitalWrite(13, HIGH);
  }else{
    digitalWrite(13, LOW);
  }
  debug2=analogRead(A0);
  /*
  if(Serial.available()){
    Serial.write(Serial.read());
  }*/
}

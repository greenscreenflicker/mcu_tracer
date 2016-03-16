#define MCU_TRACER_STARTBYTE 0xA5
int global_checksum;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Up and running");
  
}


void mcu_tracer_process(void){
  int checksum=MCU_TRACER_STARTBYTE;
  while(1){
    if(!(Serial.available())){
      //We have no data, we're done
      return;
    }
    int startbyte=Serial.read();
    if(!(startbyte==MCU_TRACER_STARTBYTE)){
      //we have no startbyte
      continue;
    }else{
      //Serial.println("Recieved startbyte");
    }
    //Now we synced to the startbyte
    while(!(Serial.available()));
    
    int order=Serial.read();

    checksum=order^checksum;
    if(order==1){
      //Serial.println("Order one");
      //we have a Init request
      //we need to check checksum now
      while(!(Serial.available()));
      int checksum_soll=Serial.read();
      if(checksum==checksum_soll){
        // call the init routine;
        //erial.println("Checksum ok");
        mcu_tracer_init();
      }else{
        //Serial.println("Checksum did not fit, expected");
        //Serial.print(checksum,HEX);
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
  //
  mcu_tracer_write_serial(MCU_TRACER_STARTBYTE);

  //transmit type
  mcu_tracer_write_serial(1); //variable init  

  //now here comes some dummy data, created manually.
  //Replace later with automatic function

  mcu_tracer_write_serial(1); //int
  mcu_tracer_write_serial(1); //read only
  mcu_tracer_write_string("read only");
  mcu_tracer_write_serial(1); //int
  mcu_tracer_write_serial(0); //read write
  mcu_tracer_write_string("read and write");

  
  mcu_tracer_write_serial(0); //last byte
  mcu_tracer_write_serial(0); //read only
  mcu_tracer_write_string("t");

  mcu_tracer_send_checksum();
}

void loop() {
  // put your main code here, to run repeatedly:
  mcu_tracer_process();
  /*
  if(Serial.available()){
    Serial.write(Serial.read());
  }*/
}

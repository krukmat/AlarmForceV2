#define DEBUG_ESP              //comment out to deactivate debug console

#ifdef DEBUG_ESP
  #define pDBGln(x) Serial.println(x)
  #define pDBG(x)   Serial.print(x)
#else 
  #define pDBG(...)
  #define pDBGln(...)
#endif

#include "SerialTransfer.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

SerialTransfer myTransfer;
struct img_meta_data{
  uint16_t counter;
  uint16_t imSize;
  uint16_t numLoops;
  uint16_t sizeLastLoop;
} ImgMetaData;
uint16_t packetCounter=1;
uint16_t bufferPointer=0;
char tempImageBuffer[32000];


void setup() {
  Serial.begin(9600);         // Set the communication speed
  Serial2.begin(962100, SERIAL_8N1); //Receiver_Txd_pin, Receiver_Rxd_pin); // Define and start Receiver serial port
  myTransfer.begin(Serial2);
  
  pinMode(12, INPUT); // Set the GPIO pin as an output
  pinMode(13, OUTPUT);

}

int threshold = 450; //Enter Your threshold value here
int abs_value = 0;

int ledCount = 10; //number of Bargraph LEDs
int Sound_signal;
int sum;
int num_Measure = 128 ; // Set the number of measurements 
int level;
int soundlow = 3575;
int soundmedium = 3400;
String picPrefix ="";
String pic_name;  

void askForPicture(){
  // ESPNow o PIR?
  digitalWrite(13, 1);
  Serial.println("Disparando camara"); 
  digitalWrite(13, 0);
}
void copyToImageBuff(uint16_t dataLenght){
  for(int y=0;y<dataLenght;y++){
    tempImageBuffer[bufferPointer+y] = myTransfer.packet.rxBuff[y+sizeof(ImgMetaData)];
  } 
  bufferPointer+=dataLenght;
  packetCounter++;  

  pDBG("dataLenght: ");
  pDBG(dataLenght);  
  pDBG(", bufferPointer: ");
  pDBGln(bufferPointer);
}

void printBuf(char localBuff){
  pDBG(F("Pixel Values: { "));
  for (uint16_t k=0; k<sizeof(myTransfer.packet.rxBuff); k++){
    pDBG(myTransfer.packet.rxBuff[k]);
    if (k < (sizeof(myTransfer.packet.rxBuff) - 1))
      pDBG(F(", "));
    else
      pDBGln(F(" }"));
  }
}


void receiveData(){
  if(myTransfer.available())  {
    myTransfer.rxObj(ImgMetaData, sizeof(ImgMetaData));
    pDBG("Struct Data: ");
    pDBG(ImgMetaData.counter);
    pDBG(", ");
    pDBG(ImgMetaData.imSize);
    pDBG(", ");  
    pDBG(ImgMetaData.numLoops);
    pDBG(", ");
    pDBG(ImgMetaData.sizeLastLoop);
    pDBG(", PacketCounter: ");
    pDBGln(packetCounter);  

    if(ImgMetaData.counter==1){  
      copyToImageBuff(MAX_PACKET_SIZE-sizeof(ImgMetaData));
    }else{
      if(ImgMetaData.counter==packetCounter){  
        if(packetCounter<ImgMetaData.numLoops){        
          copyToImageBuff(MAX_PACKET_SIZE-sizeof(ImgMetaData));
        }else if(ImgMetaData.counter==packetCounter){
          copyToImageBuff(ImgMetaData.sizeLastLoop);     
        }
      }
    }
 
    if(packetCounter>ImgMetaData.numLoops){  
      pic_name  = picPrefix;
      pic_name += "file.jpg";  
      // TODO: Mandar por LoRa     
      packetCounter=1;
      bufferPointer=0;
      delay(2000);
      //while(1){}
    }  

  }else if(myTransfer.status < 0) { 
    pDBG("ERROR: ");
    if(myTransfer.status == -1)
      pDBGln(F("CRC_ERROR"));
    else if(myTransfer.status == -2)
      pDBGln(F("PAYLOAD_ERROR"));
    else if(myTransfer.status == -3)
      pDBGln(F("STOP_BYTE_ERROR"));
  }
}

void loop() {
  
  sum = 0;
  for ( int i = 0 ; i <num_Measure; i ++)  
  {  
   Sound_signal = analogRead (12);  
    sum =sum + Sound_signal;  
  }  
 
  level = (sum / num_Measure) - 33; // Calculate the average value   
  Serial.println("El nivel es: "+String(level));
  //if (level<=soundlow){
  //  Serial.println("Low"); 
  //} else 
  if(level>soundlow){
    askForPicture();
    delay(45000); //Duerme 10 segundos para esperar la imagen?
  }
  
}

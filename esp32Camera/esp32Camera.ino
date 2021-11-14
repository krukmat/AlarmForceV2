/*************************************************************************************************************************************************
 *  TITLE: This sketch takes an image when motion is detected using a PIR sensor module that connected to pin GPIO 13 
 *  Watch the video to learn more. 
 *  YouTube Video: https://youtu.be/KTRwBBLEsXg
 *  by Tech StudyCell
 *************************************************************************************************************************************************/

/********************************************************************************************************************
 *  Preferences--> Aditional boards Manager URLs : https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *  Board Settings:
 *  Board: "ESP32 Wrover Module"
 *  Upload Speed: "921600"
 *  Flash Frequency: "80MHz"
 *  Flash Mode: "QIO"
 *  Partition Scheme: "Hue APP (3MB No OTA/1MB SPIFFS)"
 *  Core Debug Level: "None"
 *  COM Port: Depends *On Your System*
 *  
 *  GPIO 0 must be connected to GND pin while uploading the sketch
 *  After connecting GPIO 0 to GND pin, press the ESP32 CAM on-board RESET button to put the board in flashing mode
 *********************************************************************************************************************/
 
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "SerialTransfer.h"
#include <EEPROM.h>            // read and write from flash memory
// define the number of bytes you want to access
#define EEPROM_SIZE 1

#define DEBUG_ESP              //comment out to deactivate debug console

#ifdef DEBUG_ESP
  #define pDBGln(x) Serial.println(x)
  #define pDBG(x)   Serial.print(x)
#else 
  #define pDBG(...)
  #define pDBGln(...)
#endif
 
RTC_DATA_ATTR int bootCount = 0;

SerialTransfer myTransfer;
HardwareSerial Comm(1);
struct img_meta_data{
  uint16_t counter;
  uint16_t imSize;
  uint16_t numLoops;
  uint16_t sizeLastLoop;
} ImgMetaData;
const uint16_t PIXELS_PER_packet = MAX_PACKET_SIZE - sizeof(ImgMetaData);

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
 
int pictureNumber = 0;

void printStructBuf(){
  pDBG(F("Internal Struct: { "));
  for (uint16_t k=0; k<sizeof(ImgMetaData); k++){
    pDBG(myTransfer.packet.txBuff[k]);
    if (k<(sizeof(ImgMetaData)-1))
      pDBG(F(", "));
    else
      pDBGln(F(" }"));
  }
}

void printBuf(){
  pDBG(F("Pixel Values: { "));
  for (uint16_t k=8; k<MAX_PACKET_SIZE; k++){
    pDBG(myTransfer.packet.txBuff[k]);
    if (k < (MAX_PACKET_SIZE - 1))
      pDBG(F(", "));
    else
      pDBGln(F(" }"));
  }
  pDBGln();
}

void stuffPixels(const uint8_t * pixelBuff, const uint16_t &bufStartIndex, const uint16_t &txStartIndex, const uint16_t &len){
  uint16_t txi = txStartIndex;
  for (uint16_t i=bufStartIndex; i<(bufStartIndex + len); i++)  {
    myTransfer.packet.txBuff[txi] = pixelBuff[i];
    txi++;
  }
}
void sendPicture(camera_fb_t* fb){
  uint16_t startIndex = 0;
  ImgMetaData.imSize       = fb->len;                             //sizeof(myFb);
  ImgMetaData.numLoops     = (fb->len / PIXELS_PER_packet) + 1;   //(sizeof(myFb)/PIXELS_PER_packet) + 1; 
  ImgMetaData.sizeLastLoop = fb->len % PIXELS_PER_packet;         //(sizeof(myFb)%PIXELS_PER_packet);

  for(ImgMetaData.counter=1; ImgMetaData.counter<=ImgMetaData.numLoops; ImgMetaData.counter++){
    myTransfer.txObj(ImgMetaData, sizeof(ImgMetaData));
    stuffPixels(fb->buf, startIndex, sizeof(ImgMetaData), PIXELS_PER_packet);  
    myTransfer.sendData(MAX_PACKET_SIZE);
     
    pDBGln(F("Sent:"));
    pDBG(F("img.counter: "));      pDBGln((uint16_t)((myTransfer.packet.txBuff[1] << 8) | myTransfer.packet.txBuff[0]));
    pDBG(F("img.imSize: "));       pDBGln((uint16_t)((myTransfer.packet.txBuff[3] << 8) | myTransfer.packet.txBuff[2]));
    pDBG(F("img.numLoops: "));     pDBGln((uint16_t)((myTransfer.packet.txBuff[5] << 8) | myTransfer.packet.txBuff[4]));
    pDBG(F("img.sizeLastLoop: ")); pDBGln((uint16_t)((myTransfer.packet.txBuff[7] << 8) | myTransfer.packet.txBuff[6]));

    startIndex += PIXELS_PER_packet;    
    delay(100);
  }
  esp_camera_fb_return(fb);
  delay(10000);
}
  
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Comm.begin(962100, SERIAL_8N1,15,14);     //, Comm_Txd_pin, Comm_Rxd_pin // Define and start Comm serial port
  myTransfer.begin(Comm);
 
  Serial.setDebugOutput(true);
 
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 10;
  config.fb_count = 2;
 
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

   //set the camera parameters
  sensor_t * s = esp_camera_sensor_get();
  s->set_contrast(s, 2);    //min=-2, max=2
  s->set_brightness(s, 2);  //min=-2, max=2
  s->set_saturation(s, 2);  //min=-2, max=2
  delay(100);               //wait a little for settings to take effect
    
  camera_fb_t * fb = NULL;
 
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    Serial.println("Exiting now"); 
    while(1);   //wait here as something is not right
  }
  // initialize EEPROM with predefined size
  sendPicture(fb);
  
  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);  //GPIO for LED flash
  digitalWrite(4, LOW);  //turn OFF flash LED
  rtc_gpio_hold_en(GPIO_NUM_4);  //make sure flash is held LOW in sleep

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
 
  Serial.println("Going to sleep now");
  delay(3000);
  esp_deep_sleep_start();
  Serial.println("Now in Deep Sleep Mode");
} 
 
void loop() {
 
}

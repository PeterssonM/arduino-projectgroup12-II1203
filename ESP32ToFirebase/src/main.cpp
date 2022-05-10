/*********
  Rui Santos
  Complete instructions at: https://RandomNerdTutorials.com/esp32-cam-save-picture-firebase-storage/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Based on the example provided by the ESP Firebase Client Library
*********/

#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include "SD_MMC.h"
#include <EEPROM.h> 
#include <Firebase_ESP_Client.h>
#include <camera_module_pins.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>

#include <network_cred.h>
#include <firebase_cred.h>

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/data/m.jpg"

boolean motion = true;

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

/*Initiate*/

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void initCamera(){
 // OV2640 camera module
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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  /*if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 20;
    config.fb_count = 2;
  }*/
  else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

void init_sd_card(){
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
}

void init_firebase(){
  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}

/*Action*/

void take_picture(){
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  String path = FILE_PHOTO;
  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();
  esp_camera_fb_return(fb); 
  
  //Turn off cam flash.
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Turn-off the 'brownout detector'
  Serial.println("Hello World!");

  Serial.println("Initiating WiFi...");
  initWiFi();

  init_sd_card();
  Serial.println("Initiating SD-card...");

  Serial.println("Initiating Camera...");
  initCamera();

  Serial.println("Initiating Firebase...");
  init_firebase();

  //esp_deep_sleep_start();
  //Serial.println("This will never be printed");
}
boolean once = true;
void loop() {
  if(motion){
    Serial.println("Snaping a picture...");
    take_picture();
    motion = false;
  }
  delay(10);
  if (Firebase.ready() && once == true){
    Serial.print("Uploading picture... ");

    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, FILE_PHOTO, mem_storage_type_flash, FILE_PHOTO, "image/jpeg")){
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
    }
    else{
      Serial.println(fbdo.errorReason());
    }
    once = false;
  }

  //StorageReference 
}
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"
#include "SPIFFS.h"    
#include "soc/soc.h"         
#include "soc/rtc_cntl_reg.h"  
#include "driver/rtc_io.h"
#include "WiFi.h"
#include "time.h"
#include <Preferences.h>   

Preferences preferences;

#define FLASH_GPIO 4

//FIREBASE
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

#define API_KEY "AIzaSyAVj7INM7guffGsZhXLNSydDBEmYakAQLk"
#define STORAGE_BUCKET_ID "projectgroup12-2f2a2.appspot.com"

#define USER_EMAIL "esp32_ai-thinker@kth.se"
#define USER_PASSWORD "KTH-Link-12"

//define Firebase data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

//WIFI
const char* ssid = "ASUS 2.5GHz";
const char* password = "GEZAgezaGEZA";

#define ESP32_AND_FIREBASE_FILEPATH "/images/"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
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

/*INITIATIONS*/
void init_wifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void init_ntp_client_server(){
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600;
  const int   daylightOffset_sec = 3600;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void init_camera(){
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
  else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  esp_err_t err = esp_camera_init(&config); //Camera init
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

void init_sdmmc(){
  if(!SD_MMC.begin()){
    Serial.println("An Error has occurred while mounting SDMMC");
    ESP.restart();
  }
  else{
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
      Serial.println("No SD Card attached");
      return;
    }
    Serial.println("SDMMC mounted successfully");
  }

}

void init_spiffs(){
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    Serial.println("SPIFFS mounted successfully");
  }
}

void init_firebase(){
  configF.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  configF.token_status_callback = tokenStatusCallback;

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}

/*ACTION, Methods that do stuff other than inits*/

String convert_to_string(char* a, int size){
  int i;
  String s = "";
  for (i = 0; i < size; i++) {
      s = s + a[i];
  }
  return s;
}

String path = "";
String photo = "";
void get_file_name(){
  char temp[6];
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time. Reading from flash to determine file name...");
    unsigned int counter = preferences.getUInt("counter", 0) + 1;
    sprintf(temp, "%u", counter);
    preferences.putUInt("counter", counter);
//    photo = convert_to_string(temp, 1);
    photo = temp[0];
  }
  else{
    char current_time[4];
    strftime(temp, 6, "%R", &timeinfo);
    current_time[0] = temp[0];
    current_time[1] = temp[1];
    current_time[2] = temp[3];
    current_time[3] = temp[4];
    photo = convert_to_string(current_time, sizeof(current_time));
  }
  photo = photo + ".jpg";
}

void flash_on(){
  digitalWrite(FLASH_GPIO, HIGH);
}
void flash_off(){
  digitalWrite(FLASH_GPIO, LOW);
  //rtc_gpio_hold_en(FLASH_GPIO);
}

void take_picture(){
  get_file_name();
  camera_fb_t * fb = NULL;

  //Capture picture
  flash_on();
  delay(1000);
  fb = esp_camera_fb_get();
  delay(500);
  flash_off();
    
  if(!fb){
    Serial.println("Camera Failed to Capture");
    return;
  }
  
  path = ESP32_AND_FIREBASE_FILEPATH + photo;
  
  /* SDMMC
  fs::FS &fs = SD_MMC; 
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); 
    Serial.printf("Saved file to path: %s\n", path.c_str());
    file.close();
  }*/
  
  /* SPIFFS */
  File file = SPIFFS.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); 
    Serial.printf("Saved file to path: %s\n", path.c_str());
    file.close();

    //debugging
    File file = SPIFFS.open(path.c_str());
    Serial.printf("The size of %s is: %d\n", path.c_str(), file.size());
  }
  esp_camera_fb_return(fb); 
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("\n\n\n___KTH-Link___");

  Serial.println("Initiating preferences...");
  preferences.begin("KTH-Link", false); // Note: Namespace name is limited to 15 chars.
  //preferences.clear(); Remove all preferences under the opened namespace
  //preferences.remove("counter"); Or remove the counter key only

  Serial.println("Initiating WiFi...");
  init_wifi();

  Serial.println("Initiating SD-card...");
  //init_SDMMC();
  init_spiffs();
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Turn-off the 'brownout detector'
  Serial.println("Initiating Camera...");
  pinMode(FLASH_GPIO, OUTPUT);
  flash_off();
  init_camera();

  Serial.println("Initiating Firebase...");
  init_firebase();


  Serial.println("Attempting to take a picture...");
  take_picture();
  flash_off();

  if (Firebase.ready()){
    Serial.print("Uploading picture... ");

    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, path, mem_storage_type_flash, path, "image/jpeg")){
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
    }
    else{
      Serial.println(fbdo.errorReason());
    }
  }

  delay(3000);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
  Serial.println("Going to sleep now, Zzz...");
  delay(1000);
  esp_deep_sleep_start();
}

void loop() {}

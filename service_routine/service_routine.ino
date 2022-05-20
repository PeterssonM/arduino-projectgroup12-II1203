#include "SPIFFS.h"
#include <EEPROM.h> 

void list_all_files(){
 
  File root = SPIFFS.open("/");
 
  File file = root.openNextFile();
 
  while(file){
 
      Serial.print("FILE: ");
      Serial.println(file.name());
 
      file = root.openNextFile();
  }
}

void read_eeprom(){
  Serial.printf("Counter variable determining file name is at: %d\n", EEPROM.read(0));  
}

/*
 * Clears flash memory. 
 */
void service_routine(){
  EEPROM.write(0, 0);
  EEPROM.commit();
  if(!SPIFFS.format()){
    Serial.println("Error formating");  
  }  
}
 
void setup() {
  EEPROM.begin(256);
  Serial.begin(115200);

  Serial.println("\n\n\n___Service_routine_for_KTH-Link___");  

  read_eeprom();

  Serial.println("Listing files before format");
  list_all_files();
  
  service_routine();

  read_eeprom();
  
  Serial.println("Listing files after format");
  list_all_files();

  delay(1000);
  Serial.println("Done.");  
  
  
}
 
void loop() {}

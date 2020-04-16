#include "SPIFFS.h"
 
void setup() {
 
   Serial.begin(115200);
 
   if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
   }
 
    File file = SPIFFS.open("/upload.html", FILE_READ);
 
    if(!file){
        Serial.println("There was an error opening the file for reading");
        return;
    }
 
    
  
 
 
    while(file.available()){
 
        Serial.write(file.read());
    }
 
    file.close();
}
 
void loop() {}

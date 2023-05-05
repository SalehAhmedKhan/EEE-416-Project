#include <SoftwareSerial.h>                        
SoftwareSerial esp8266(2,3);  //rx, tx (Connections flipped while connecting to esp-01 module)                 
#define serialCommunicationSpeed 115200               
#define time_out 1000
#define DEBUG true

char sc = '"';

String rec_command;
String rec_data;

void setup()

{
  Serial.begin(serialCommunicationSpeed);           
  esp8266.begin(serialCommunicationSpeed);                                  
}

void loop()                                                         
{
  if(Serial.available())
  {
    rec_command = serialInput(time_out, DEBUG);

    rec_command +="\r\n";

    sendData(rec_command, time_out, DEBUG);
  }

}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    //Serial.println("Sent to ESP: ");
    //Serial.print(command);                                            
    esp8266.print(command);                                          
    long int time = millis();                                      
    while( (time+timeout) > millis())                                 
    {      
      while(esp8266.available())                                      
      {
        char c = esp8266.read();                                     
        response+=c;                                                  
      }  
    }    
    if(debug)                                                        
    {
      //Serial.println("Received from ESP: ");
      Serial.print(response);
    }    
    return response;                                                  
}

String serialInput(const int timeout, boolean debug)
{                                    
    String response = "";                                                                                      
    long int time = millis();                                      
    while( (time+timeout) > millis())                                 
    {      
      while(Serial.available())                                      
      {
        char c = Serial.read();                                     
        response+=c;                                                  
      }  
    }    
    if(debug)                                                        
    {
      //Serial.print(response);
    } 
    return response;
}

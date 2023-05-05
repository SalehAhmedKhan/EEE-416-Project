#include <virtuabotixRTC.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <DMD2.h>

SoftDMD dmd(2, 1);  
virtuabotixRTC myRTC(A0, A1, A2); //
LiquidCrystal lcd(5, 12, 4, 2,3, 10); //
              
#define serialCommunicationSpeed 115200               
#define time_out 1000
#define DEBUG 1

uint8_t en_m_size[12] = {19, 47, 19, 19, 19, 19, 19, 30, 19, 19, 19, 19};
uint8_t bn_m_size[12] = {19, 47, 19, 34, 19, 19, 19, 19, 19, 19, 19, 19};
uint8_t d_size = 19; //8+1+8+2
uint8_t y_size = 37; //2 + (4*8 + 3)

//uint8_t M1 = 12;  //sizeof(disp1) / sizeof(disp1[0]);
uint8_t N1 = 8; //sizeof(disp1[0]) / sizeof(disp1[0][0]);
uint8_t lo_op=0; //selects which column from the disp matrix will appear first
uint8_t second_loop = 0;

//time variables 

uint8_t sync_time[6];

uint8_t tim_e[6]={1, 2, 3, 4, 5, 6};
uint8_t ampm=0; //0 if AM,1 if PM

uint8_t en_date[2]={1, 4};
uint8_t en_year[4]={2, 0, 2, 2};
uint8_t en_m=7; //month

uint8_t bn_date[2] = {3, 0};
uint8_t bn_year[4]={1, 4, 2 , 9};
uint8_t bn_m=3; //month

uint8_t week; 

uint8_t count = 5;
uint8_t scroll = 1;

char sc = '"';
String rec_command;
String rec_data;
//String response = "";
uint8_t prev;

void setup()

{
  dmd.begin();
  Serial.begin(serialCommunicationSpeed);           
  dmd.setBrightness(255);
  //lcd.begin(16, 2);
  //dmd.begin();

  //lcd.clear();
  //lcd.setCursor(0,0);
  //lcd.print("Begin");delay(1000);

  //Serial.println("Loading Previous RTC values...");
  //load_rtc();
  ///delay(100);

  //lcd.setCursor(0,0);
  //lcd.print("Init WIFI");delay(1000);

  initwifi();
  //delay(100);

  //lcd.setCursor(0,0);
  //lcd.print("DONE");delay(1000);

}

void loop()                                                         
{
    //Serial.println("Loading and Showing RTC Time...");
    if(count == 5)
    {
      populate_cells();
      show_time();
      
    }
    else
    {
      dmd.end();
      //lcd.setCursor(0,0);
      //lcd.print("L&S RTC time");delay(1000);
    }

    //load_rtc();
    //populate_cells();
    //delay(100);
    //show_time();

    

//    if(scroll >= 0 && scroll < 5)
//      show_week_day();
//    if(scroll >= 5 && scroll < 10)
//      show_eng_date();
//    if(scroll >= 10 && scroll < 15)
//      show_ban_date();
      
    if(count == 1)
    {
      get_time();
      set_time();
    }
    if(count == 2)
    {
      get_eng_date();
      set_eng_date();
    }
    if(count == 3)
    {
      get_ban_date();
      set_ban_date();
    }
    if(count == 4)
    {
      myRTC.updateTime();
      prev = myRTC.hours;
      count = count + 1;
    }  
    if(count == 5)
    {
      //delay(250); 
    }    
    if((count == 5) && (abs(prev - myRTC.hours) >= 6))
    {
      count = 1;
    }
    
}

void sendData(String command, const int timeout, boolean debug)
{
    rec_data = "";                                          
    Serial.print(command);                                          
    long int time = millis();                                      
    while( (time+timeout) > millis())                                 
    {      
      while(Serial.available())                                      
      {
        char c = Serial.read();                                     
        rec_data+=c;                                                  
      }  
    }    
    if(debug)                                                        
    {
      //lcd.setCursor(0,0);
      //lcd.print(rec_data);
      delay(1000);
    }                                                    
}

void initwifi(void)
{
    //Serial.println("Closing Previous Connection");
    delay(1);
    rec_command = "AT+CIPCLOSE";
    rec_command += "\r\n";
    sendData(rec_command, 2000, 1);
    
    delay(100);
    //Serial.println("Initiating WiFi Module, Connecting to Specified WiFi");
    delay(1);
    
    rec_command = "AT+RST";
    rec_command += "\r\n";
    sendData(rec_command, 1000, 0);

    rec_command = "AT+CWJAP=";
    rec_command += sc;
    rec_command += "Barshan"; //SSID
    rec_command += sc;
    rec_command += ",";
    rec_command += sc;
    rec_command += "#SaL8632"; //PASS
    rec_command += sc;
    rec_command += "\r\n";
    sendData(rec_command, 2000, 1);        
    delay (1000);

    rec_command = "\r\n";
    sendData(rec_command, 1000, 1);
    
}

void get_time(void)
{
    //Serial.println("Getting Online Time...");
    lcd.setCursor(0,0);
    lcd.print("Getting Time...");
    delay(1);

    rec_command = "AT+CIPSTART=";
    rec_command += sc;
    rec_command += "TCP";
    rec_command += sc;
    rec_command += ",";
    rec_command += sc;
    rec_command += "api.thingspeak.com";
    rec_command += sc;
    rec_command += ",80";
    rec_command += "\r\n";
    sendData(rec_command, 1, DEBUG);

    delay(1);

    rec_command = "AT+CIPSEND=90";
    rec_command += "\r\n";
    sendData(rec_command, 1, DEBUG);

    delay(1);

    rec_command = "GET /apps/thinghttp/send_request?api_key=Q1ZBWRBEXXO44K68";
    rec_command += "\r\n";
    rec_command += "Host:api.thingspeak.com";
    rec_command += "\r\n\r\n\r\n\r\n";
    sendData(rec_command, 1000, DEBUG);

    delay(1);
}

void set_time(void)
{
  if(rec_data.indexOf("+IPD,53") > 0)
  {
    //Serial.println("Error 53");
  }
  else if(rec_data.indexOf("+IPD,132") > 0)
  {
    //Serial.println("Error 132");
  }
  else if(rec_data.indexOf("+IPD,") > 0)
    {
      //Serial.println("Setting RTC Time");
    lcd.setCursor(0,0);
    lcd.print("Setting RTC Time");
      //Serial.println(rec_data.substring(rec_data.indexOf("+IPD")));
      
      //Serial.println(rec_data.substring(rec_data.indexOf("+IPD")+20, rec_data.indexOf("+IPD")+32));
      
      //parsing the string
      rec_data = rec_data.substring(rec_data.indexOf("+IPD")+20, rec_data.indexOf("+IPD")+32);
      
      //Serial.println(rec_data.indexOf("M")); //10
      //Serial.println(rec_data.substring(rec_data.indexOf("M")-1, rec_data.indexOf("M")));
      
      if(rec_data.substring(rec_data.indexOf("M")-1, rec_data.indexOf("M")) == "P")
      {
        sync_time[0] = ((rec_data.substring(rec_data.indexOf("M")-10, rec_data.indexOf("M")-8).toInt())/10);
        sync_time[1] = ((rec_data.substring(rec_data.indexOf("M")-10, rec_data.indexOf("M")-8).toInt())%10);
        if(sync_time[0] != 1 && sync_time[1] != 2) // 12 PM Special Case
        {
          sync_time[0] = sync_time[0]+1;
          sync_time[1] = sync_time[1]+1;
        }
      }
      if(rec_data.substring(rec_data.indexOf("M")-1, rec_data.indexOf("M")) == "A")
      {
        sync_time[0] = ((rec_data.substring(rec_data.indexOf("M")-10, rec_data.indexOf("M")-8).toInt())/10);
        sync_time[1] = ((rec_data.substring(rec_data.indexOf("M")-10, rec_data.indexOf("M")-8).toInt())%10);
        if(sync_time[0] == 1 && sync_time[1] == 2) // 12 AM Special Case - 0 hrs in 24 hr format
        {
          sync_time[0] = 0;
          sync_time[1] = 0;
        }
      }

      sync_time[2] = (rec_data.substring(rec_data.indexOf("M")-7, rec_data.indexOf("M")-5).toInt())/10;
      sync_time[3] = (rec_data.substring(rec_data.indexOf("M")-7, rec_data.indexOf("M")-5).toInt())%10;
      
      sync_time[4] = (rec_data.substring(rec_data.indexOf("M")-4, rec_data.indexOf("M")-2).toInt())/10;
      sync_time[5] = (rec_data.substring(rec_data.indexOf("M")-4, rec_data.indexOf("M")-2).toInt())%10;

      myRTC.setDS1302Time((sync_time[4]*10) + sync_time[5], (sync_time[2]*10) + sync_time[3], (sync_time[0]*10) + sync_time[1], week, (en_date[0]*10)+en_date[1], en_m, (en_year[0]*1000)+(en_year[1]*100)+(en_year[2]*10)+en_year[3]);

      rec_data = "";
      count = count+1;
    }
}

void get_eng_date(void)
{
    //Serial.println("Getting Online English Date...");
        lcd.setCursor(0,0);
    lcd.print("Getting Eng Date...");
    delay(1);

    rec_command = "AT+CIPSTART=";
    rec_command += sc;
    rec_command += "TCP";
    rec_command += sc;
    rec_command += ",";
    rec_command += sc;
    rec_command += "api.thingspeak.com";
    rec_command += sc;
    rec_command += ",80";
    rec_command += "\r\n";
    sendData(rec_command, 1, DEBUG);

    delay(1);

    rec_command = "AT+CIPSEND=90";
    rec_command += "\r\n";
    sendData(rec_command, 1, DEBUG);

    delay(1);

    rec_command = "GET /apps/thinghttp/send_request?api_key=6EX7E9O58AOAB9DG";
    rec_command += "\r\n";
    rec_command += "Host:api.thingspeak.com";
    rec_command += "\r\n\r\n\r\n\r\n";
    sendData(rec_command, 1000, DEBUG);

    delay(1);
}

void set_eng_date(void)
{
  if(rec_data.indexOf("+IPD,53") > 0)
  {
    //Serial.println("Error 53");
  }
  else if(rec_data.indexOf("+IPD,132") > 0)
  {
    //Serial.println("Error 132");
  }
  else if(rec_data.indexOf("+IPD,") > 0)
    {
      //Serial.println("Setting RTC English Date");
              lcd.setCursor(0,0);
    lcd.print("Setting RTC Eng Date");
      //Serial.println(rec_data.substring(rec_data.indexOf("+IPD,")));

      //Serial.println(rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1)));

      //parsing the string

      uint8_t dat_e = (rec_data.substring(rec_data.lastIndexOf(",")-2, rec_data.lastIndexOf(","))).toInt();
      en_date[0] = dat_e/10;
      en_date[1] = dat_e%10;

      //Serial.println(rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2));
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "January")
        en_m = 1;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "February")
        en_m = 2;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "March")
        en_m = 3;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "April")
        en_m = 4;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "May")
        en_m = 5;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "June")
        en_m = 6;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "July")
        en_m = 7;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "August")
        en_m = 8;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "September")
        en_m = 9;        
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "October")
        en_m = 10;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "November")
        en_m = 11;
      if((rec_data.substring(rec_data.indexOf(",",rec_data.indexOf(",")+1)+2, rec_data.lastIndexOf(",")-2)) == "December")
        en_m = 12;
                
      uint16_t yea_r = (rec_data.substring(rec_data.lastIndexOf(",")+2, rec_data.lastIndexOf(",")+6)).toInt();
      en_year[0] = yea_r/1000;
      en_year[1] = (yea_r%1000)/100;
      en_year[2] = ((yea_r%1000)%100)/10;
      en_year[3] = ((yea_r%1000)%100)%10;

      //Serial.println(rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1)));
      if((rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1))) == "Saturday")
        week = 1;
      if((rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1))) == "Sunday")
        week = 2;
      if((rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1))) == "Monday")
        week = 3;
      if((rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1))) == "Tuesday")
        week = 4;
      if((rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1))) == "Wednesday")
        week = 5;
      if((rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1))) == "Thursday")
        week = 6;
      if((rec_data.substring(rec_data.indexOf(",")+9, rec_data.indexOf(",",rec_data.indexOf(",")+1))) == "Friday")
        week = 7;

      myRTC.setDS1302Time((sync_time[4]*10) + sync_time[5], (sync_time[2]*10) + sync_time[3], (sync_time[0]*10) + sync_time[1], week, (en_date[0]*10)+en_date[1], en_m, (en_year[0]*1000)+(en_year[1]*100)+(en_year[2]*10)+en_year[3]);
      
      rec_data = "";
      count = count+1;
    }
}

void get_ban_date(void)
{
    //Serial.println("Getting Online Bangla Date...");
                  lcd.setCursor(0,0);
    lcd.print("Getting Ban Date...");
    delay(1);

    rec_command = "AT+CIPSTART=";
    rec_command += sc;
    rec_command += "TCP";
    rec_command += sc;
    rec_command += ",";
    rec_command += sc;
    rec_command += "api.thingspeak.com";
    rec_command += sc;
    rec_command += ",80";
    rec_command += "\r\n";
    sendData(rec_command, 1, DEBUG);

    delay(1);

    rec_command = "AT+CIPSEND=90";
    rec_command += "\r\n";
    sendData(rec_command, 1, DEBUG);

    delay(1);

    rec_command = "GET /apps/thinghttp/send_request?api_key=5MKIPC0USLFNP3UT";
    rec_command += "\r\n";
    rec_command += "Host:api.thingspeak.com";
    rec_command += "\r\n\r\n\r\n\r\n";
    sendData(rec_command, 2000, DEBUG);

    delay(1);
}

void set_ban_date(void)
{
  if(rec_data.indexOf("+IPD,53") > 0)
  {
    //Serial.println("Error 53");
  }
  else if(rec_data.indexOf("+IPD,132") > 0)
  {
    //Serial.println("Error 132");
  }
  else if(rec_data.indexOf("+IPD,") > 0)
    {
      //Serial.println("Setting Bangla Date");
                        lcd.setCursor(0,0);
    lcd.print("Setting Bangla Date...");
      //Serial.println(rec_data.substring(rec_data.indexOf("+IPD")));
      //parsing the string
      
      int dat_e = (rec_data.substring(rec_data.lastIndexOf("-")+1, rec_data.lastIndexOf("-")+4)).toInt();
      bn_date[0] = dat_e/10;
      bn_date[1] = dat_e%10;

      //Serial.println(rec_data.substring(rec_data.indexOf(" ", rec_data.indexOf("-")+2)+1, rec_data.lastIndexOf(",")));
      String mont_h = rec_data.substring(rec_data.indexOf(" ", rec_data.indexOf("-")+2)+1, rec_data.lastIndexOf(","));
      if(mont_h == "Baisakh")
        bn_m = 1;
      if(mont_h == "Joishtho")
        bn_m = 2;
      if(mont_h == "Asadha")
        bn_m = 3;
      if(mont_h == "Srabon")
        bn_m = 4;
      if(mont_h == "Bhadra")
        bn_m = 5;
      if(mont_h == "Ashshin")
        bn_m = 6;
      if(mont_h == "Kartik")
        bn_m = 7;
      if(mont_h == "Ogrohayon")
        bn_m = 8;
      if(mont_h == "Push")
        bn_m = 9;        
      if(mont_h == "Magh")
        bn_m = 10;
      if(mont_h == "Falgun")
        bn_m = 11;
      if(mont_h == "Choitro")
        bn_m = 12;
  
      int yea_r = (rec_data.substring(rec_data.lastIndexOf(",")+2, rec_data.lastIndexOf(",")+6)).toInt();
      bn_year[0] = yea_r/1000;
      bn_year[1] = (yea_r%1000)/100;
      bn_year[2] = ((yea_r%1000)%100)/10;
      bn_year[3] = ((yea_r%1000)%100)%10;

      rec_data = "";
      count = count+1;
    }
}

void load_rtc(void)
{
  myRTC.updateTime();
  //Serial.println(myRTC.hours);
  if(myRTC.hours >= 13)
  {
    tim_e[0] = (myRTC.hours-12)/10;
    tim_e[1] = (myRTC.hours-12)%10;
  }
  else if(myRTC.hours == 0)
  {
    tim_e[0] = 1;
    tim_e[1] = 2;
  }
  else
  {
    tim_e[0] = (myRTC.hours)/10;
    tim_e[1] = (myRTC.hours)%10;
  }

  tim_e[2] = myRTC.minutes/10;
  tim_e[3] = myRTC.minutes%10;

  tim_e[4] = myRTC.seconds/10;
  tim_e[5] = myRTC.seconds%10;

  if(myRTC.hours >= 12)
    ampm = 1; //0 if AM,1 if PM
  else
    ampm = 0; //0 if AM,1 if PM

  week = myRTC.dayofweek;

  en_date[0] = myRTC.dayofmonth/10;
  en_date[1] = myRTC.dayofmonth%10;

  en_m = myRTC.month;
  
  en_year[0] = myRTC.year/1000;
  en_year[1] = (myRTC.year%1000)/100;
  en_year[2] = ((myRTC.year%1000)%100)/10;
  en_year[3] = ((myRTC.year%1000)%100)%10; 
}

void show_time(void)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(tim_e[0]);
  lcd.print(tim_e[1]);
  lcd.print(":");
  lcd.print(tim_e[2]);
  lcd.print(tim_e[3]);
  lcd.print(":");
  lcd.print(tim_e[4]);
  lcd.print(tim_e[5]);
  lcd.print("  ");
  if(ampm == 1)
    lcd.print("PM");
  else
    lcd.print("AM");

  lcd.print(" ");
  
  delay(1);

  scroll = scroll+1;

  if(scroll >= 15)
  {
    scroll = 0;
  }
}
void show_week_day(void)
{
  lcd.setCursor(0, 1);
  lcd.print(week);

  delay(500);
}
void show_eng_date(void)
{
  lcd.setCursor(0, 1);
  lcd.print(en_date[0]);
  lcd.print(en_date[1]);
  lcd.print("/");
  lcd.print(en_m);
  lcd.print("/");
  lcd.print(en_year[0]);
  lcd.print(en_year[1]);
  lcd.print(en_year[2]);
  lcd.print(en_year[3]);
  lcd.print(" ");
  lcd.print("EN");

  delay(500);
}
void show_ban_date(void)
{
  lcd.setCursor(0, 1);
  lcd.print(bn_date[0]);
  lcd.print(bn_date[1]);
  lcd.print("/");
  lcd.print(bn_m);
  lcd.print("/");
  lcd.print(bn_year[0]);
  lcd.print(bn_year[1]);
  lcd.print(bn_year[2]);
  lcd.print(bn_year[3]);
  lcd.print(" ");
  lcd.print("BN");

  delay(500);
}

//matrices for English months
uint64_t en_months( uint8_t en_m, uint8_t i, uint8_t shft ){
  
 uint64_t temp = 0;

 if (en_m==1){
    if      (i==0)  temp = 0b00000000000000000000000000000000000000001111110;
    else if (i==1)  temp = 0b00000000000000000000000000000000000000010000001;
    else if (i==2)  temp = 0b00000000000000000000000000000000000000100000000;
    else if (i==3)  temp = 0b11111111111111111111111111111111111111111111111;
    else if (i==4)  temp = 0b00001110000000100000001001000000101010100000110;
    else if (i==5)  temp = 0b10010001111000100000001000100000101010100001010;
    else if (i==6)  temp = 0b10010000001000100000001000010000101010100010010;
    else if (i==7)  temp = 0b10010111010000100000001000001000101010100100010;
    else if (i==8)  temp = 0b10001001010000100111001000010000101010101000010;
    else if (i==9)  temp = 0b10000001010000100101101000100000101010100100010;
    else if(i==10)  temp = 0b10000010001000100110011001111100101010100001010;
    else if(i==11)  temp = 0b01000100000110100000001000000011101010100000110;
    else if(i==12)  temp = 0b00111000000010100001111000000000101010100000010;
    else if(i==13)  temp = 0b00000000000000000001011000001000000000000100000;
    else if(i==14)  temp = 0b00000000000000000000100100001000000000000100000;
    else if(i==15)  temp = 0b00000000000000000000000010000000000000000000000;
 }

 else if (en_m==2){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==3){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==4){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==5){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==6){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==7){ //JULY
    if      (i==0)  temp = 0b000000000000000000000010000000;
    else if (i==1)  temp = 0b000000000000000000000001111100;
    else if (i==2)  temp = 0b000000000000000000000000000010;
    else if (i==3)  temp = 0b111111111111111111111111111111;
    else if (i==4)  temp = 0b000010110000000000010100010000;
    else if (i==5)  temp = 0b100100001110011111110100011100;
    else if (i==6)  temp = 0b100100100010100010010100110010;
    else if (i==7)  temp = 0b100100110100100000010100000010;
    else if (i==8)  temp = 0b010011010100111000010100000010;
    else if (i==9)  temp = 0b010000010100011000010100111100;
    else if(i==10)  temp = 0b001000100100000000010100001100;
    else if(i==11)  temp = 0b000111100100000000010100000010;
    else if(i==12)  temp = 0b000000001111000000010100000001;
    else if(i==13)  temp = 0b000000001011100000000000000000;
    else if(i==14)  temp = 0b000000001110010000000000000000;
    else if(i==15)  temp = 0b000000000000000000000000000000;
 }

 else if (en_m==8){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==9){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==10){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==11){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==12){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

  temp = temp << shft;
  return temp;
 }

//Function for AMPM data
uint64_t ampm_fn( uint8_t ampm, uint8_t i, uint8_t shft ){
 //ampm = 0 means am, purbahno
 uint64_t temp = 0;

 if (ampm==0){
    if      (i==0)  temp = 0b0000000000000000010000000000000000;
    else if (i==1)  temp = 0b0000000000000001100000000000000000;
    else if (i==2)  temp = 0b0000000000000011000000000000000000;
    else if (i==3)  temp = 0b0111100111111111101111111111100000;
    else if (i==4)  temp = 0b1000010100000010010001000000000000;
    else if (i==5)  temp = 0b1110111100001110010011110000000000;
    else if (i==6)  temp = 0b0011001100110010010110001111000000;
    else if (i==7)  temp = 0b0010000100100010010000001001000000;
    else if (i==8)  temp = 0b0000000100010010010011110011000000;
    else if (i==9)  temp = 0b0000000100001010010000110000000000;
    else if(i==10)  temp = 0b0000000100000110010000001100000000;
    else if(i==11)  temp = 0b0000000100000010010000000110000000;
    else if(i==12)  temp = 0b0000001100000000000000000001011000;
    else if(i==13)  temp = 0b0000010001100000000000000000011000;
    else if(i==14)  temp = 0b0000011110000000000000000000001000;
    else if(i==15)  temp = 0b0000000000000000000000000000010000;
 }

 else{
    if      (i==0)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==1)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==2)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==3)  temp = 0b111111111111001111001111111111101111111111100000;
    else if (i==4)  temp = 0b000011100010010000101000000010010001000000000000;
    else if (i==5)  temp = 0b010010010010011101111000001110010001110000000000;
    else if (i==6)  temp = 0b010011001010000110011000110010010010001110000000;
    else if (i==7)  temp = 0b010000001010000100001001000010010000001001000000;
    else if (i==8)  temp = 0b001000001010000000001000100010010011110011000000;
    else if (i==9)  temp = 0b000100011010000000001000011010010000110000000000;
    else if(i==10)  temp = 0b000011110110000000001001000110010000001100000000;
    else if(i==11)  temp = 0b000000000010000000001000100010010000000010000000;
    else if(i==12)  temp = 0b000000000000000000000000000000000000000001011000;
    else if(i==13)  temp = 0b000000000000000000000000000000000000000000011000;
    else if(i==14)  temp = 0b000000000000000000000000000000000000000000001000;
    else if(i==15)  temp = 0b000000000000000000000000000000000000000000010000;
 }
  temp = temp << shft;
  return temp;
}

//matrices for Bengali months
uint64_t bn_months( uint8_t bn_m, uint8_t i, uint8_t shft ){
  
 uint64_t temp = 0;

 if (bn_m==1){
    if      (i==0)  temp = 0b00000000000000000000000000000000000000001111110;
    else if (i==1)  temp = 0b00000000000000000000000000000000000000010000001;
    else if (i==2)  temp = 0b00000000000000000000000000000000000000100000000;
    else if (i==3)  temp = 0b11111111111111111111111111111111111111111111111;
    else if (i==4)  temp = 0b00001110000000100000001001000000101010100000110;
    else if (i==5)  temp = 0b10010001111000100000001000100000101010100001010;
    else if (i==6)  temp = 0b10010000001000100000001000010000101010100010010;
    else if (i==7)  temp = 0b10010111010000100000001000001000101010100100010;
    else if (i==8)  temp = 0b10001001010000100111001000010000101010101000010;
    else if (i==9)  temp = 0b10000001010000100101101000100000101010100100010;
    else if(i==10)  temp = 0b10000010001000100110011001111100101010100001010;
    else if(i==11)  temp = 0b01000100000110100000001000000011101010100000110;
    else if(i==12)  temp = 0b00111000000010100001111000000000101010100000010;
    else if(i==13)  temp = 0b00000000000000000001011000001000000000000100000;
    else if(i==14)  temp = 0b00000000000000000000100100001000000000000100000;
    else if(i==15)  temp = 0b00000000000000000000000010000000000000000000000;
 }

 else if (bn_m==2){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==3){ //ASHAR
    if      (i==0)  temp = 0b0000000000000000000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000000000000000000;
    else if (i==2)  temp = 0b0000000000000000000000000000000000;
    else if (i==3)  temp = 0b1111111111111011111111110111111111;
    else if (i==4)  temp = 0b0000100000100100110001001001000000;
    else if (i==5)  temp = 0b0100011100100100001001001001000000;
    else if (i==6)  temp = 0b0100111110100100011111001001001100;
    else if (i==7)  temp = 0b0100010010100100100011001001001110;
    else if (i==8)  temp = 0b0010000010100100111001001001000010;
    else if (i==9)  temp = 0b0001000110100100000101001001000010;
    else if(i==10)  temp = 0b0000111101100100000011001001100100;
    else if(i==11)  temp = 0b0000000000100100000001001000111000;
    else if(i==12)  temp = 0b0000000000100000000000000000000000;
    else if(i==13)  temp = 0b0000000000000000000000000000010000;
    else if(i==14)  temp = 0b0000000000000000000000000000010000;
    else if(i==15)  temp = 0b0000000000000000000000000000000000;
 }

 else if (bn_m==4){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==5){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==6){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==7){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==8){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==9){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==10){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==11){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (bn_m==12){
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

  temp = temp << shft;
  return temp;
}

//The function to display digits
uint64_t digits_fn( uint8_t dig_it, uint8_t i, uint8_t shft ){

uint64_t temp=0;

 if (dig_it==0){
    if      (i==2)  temp = 0b00111100;
    else if (i==3)  temp = 0b01111110;
    else if (i==4)  temp = 0b11100110;
    else if (i==5)  temp = 0b11000011;
    else if (i==6)  temp = 0b11000011;
    else if (i==7)  temp = 0b11000011;
    else if (i==8)  temp = 0b11000011;
    else if (i==9)  temp = 0b11000011;
    else if(i==10)  temp = 0b11000011;
    else if(i==11)  temp = 0b11100111;
    else if(i==12)  temp = 0b01111110;
    else if(i==13)  temp = 0b00111100;}
  
 else if (dig_it==1){
    if      (i==2)  temp = 0b11000000;
    else if (i==3)  temp = 0b11100000;
    else if (i==4)  temp = 0b01111000;
    else if (i==5)  temp = 0b00111110;
    else if (i==6)  temp = 0b00001110;
    else if (i==7)  temp = 0b00000111;
    else if (i==8)  temp = 0b00000011;
    else if (i==9)  temp = 0b00000011;
    else if(i==10)  temp = 0b01100011;
    else if(i==11)  temp = 0b11100110;
    else if(i==12)  temp = 0b11111110;
    else if(i==13)  temp = 0b01111100;}
    
 else if (dig_it==2){
    if      (i==2)  temp = 0b11000000; 
    else if (i==3)  temp = 0b11110000;
    else if (i==4)  temp = 0b01111100;
    else if (i==5)  temp = 0b00011110;
    else if (i==6)  temp = 0b00000111;
    else if (i==7)  temp = 0b00000011;
    else if (i==8)  temp = 0b11111111;
    else if (i==9)  temp = 0b11111110;
    else if(i==10)  temp = 0b01110000;
    else if(i==11)  temp = 0b00111000;
    else if(i==12)  temp = 0b00011100;
    else if(i==13)  temp = 0b00001111;}
    
 else if (dig_it==3){
    if      (i==2)  temp = 0b11001100;
    else if (i==3)  temp = 0b11011110;
    else if (i==4)  temp = 0b11011111;
    else if (i==5)  temp = 0b11000011;
    else if (i==6)  temp = 0b11000011;
    else if (i==7)  temp = 0b11000011;
    else if (i==8)  temp = 0b11000011;
    else if (i==9)  temp = 0b11000011;
    else if(i==10)  temp = 0b11000111;
    else if(i==11)  temp = 0b11101110;
    else if(i==12)  temp = 0b01111100;
    else if(i==13)  temp = 0b00111000;}
    
 else if (dig_it==4){
    if      (i==2)  temp = 0b00111100;
    else if (i==3)  temp = 0b01111110;
    else if (i==4)  temp = 0b11000011;
    else if (i==5)  temp = 0b11000011;
    else if (i==6)  temp = 0b01100110;
    else if (i==7)  temp = 0b00111100;
    else if (i==8)  temp = 0b00111100;
    else if (i==9)  temp = 0b01100110;
    else if(i==10)  temp = 0b11000011;
    else if(i==11)  temp = 0b11000011;
    else if(i==12)  temp = 0b01111110;
    else if(i==13)  temp = 0b00111100;}
  
 else if (dig_it==5){ 
    if      (i==2)  temp = 0b00001000;
    else if (i==3)  temp = 0b00011100;
    else if (i==4)  temp = 0b00111111;
    else if (i==5)  temp = 0b01100110;
    else if (i==6)  temp = 0b11001100;
    else if (i==7)  temp = 0b11001100;
    else if (i==8)  temp = 0b11001100;
    else if (i==9)  temp = 0b11001100;
    else if(i==10)  temp = 0b11001100;
    else if(i==11)  temp = 0b11100110;
    else if(i==12)  temp = 0b01111111;
    else if(i==13)  temp = 0b00111100;}

 else if (dig_it==6){
    if      (i==2)  temp = 0b00110000;
    else if (i==3)  temp = 0b00110000;
    else if (i==4)  temp = 0b00110000;
    else if (i==5)  temp = 0b00110000;
    else if (i==6)  temp = 0b00110001;
    else if (i==7)  temp = 0b00111111;
    else if (i==8)  temp = 0b10011111;
    else if (i==9)  temp = 0b11000011;
    else if(i==10)  temp = 0b11000011;
    else if(i==11)  temp = 0b11100011;
    else if(i==12)  temp = 0b01111110;
    else if(i==13)  temp = 0b00111100;}
  
 else if (dig_it==7){
    if      (i==2)  temp = 0b00111100;
    else if (i==3)  temp = 0b01100110;
    else if (i==4)  temp = 0b11000110;
    else if (i==5)  temp = 0b11000110;
    else if (i==6)  temp = 0b11111110;
    else if (i==7)  temp = 0b01111110;
    else if (i==8)  temp = 0b00000110;
    else if (i==9)  temp = 0b00000110;
    else if(i==10)  temp = 0b00000110;
    else if(i==11)  temp = 0b00000110;
    else if(i==12)  temp = 0b00000111;
    else if(i==13)  temp = 0b00000011;}
  
 else if (dig_it==8){
    if      (i==2)  temp = 0b11000000;
    else if (i==3)  temp = 0b11000000;
    else if (i==4)  temp = 0b11000000;
    else if (i==5)  temp = 0b11000000;
    else if (i==6)  temp = 0b11000000;
    else if (i==7)  temp = 0b11000001;
    else if (i==8)  temp = 0b11111111;
    else if (i==9)  temp = 0b11111110;
    else if(i==10)  temp = 0b11000100;
    else if(i==11)  temp = 0b11000100;
    else if(i==12)  temp = 0b11001100;
    else if(i==13)  temp = 0b01111000;}

 else if (dig_it==9){
    if      (i==2)  temp = 0b01100000;
    else if (i==3)  temp = 0b01100000;
    else if (i==4)  temp = 0b00110000;
    else if (i==5)  temp = 0b00011000;
    else if (i==6)  temp = 0b00001100;
    else if (i==7)  temp = 0b00000110;
    else if (i==8)  temp = 0b00000011;
    else if (i==9)  temp = 0b01111011;
    else if(i==10)  temp = 0b11111111;
    else if(i==11)  temp = 0b11000111;
    else if(i==12)  temp = 0b11100110;
    else if(i==13)  temp = 0b01100100;}
 
 else if (dig_it==10){//COMMA
    if      (i==12)  temp = 0b011;
    else if (i==13)  temp = 0b011;
    else if (i==14)  temp = 0b001;
    else if (i==15)  temp = 0b010;}

  temp = temp << shft;
  return temp;
}
//Let there be Light!!!
void populate_cells()
{
  //uint8_t en_m = 7;
  //uint8_t bn_m = 3;
  //uint8_t ampm = 1;
  //ampm size: 34 if ampm=0 (AM), 48 if ampm=1 (PM)
  uint8_t ampm_size = 34 + 14*ampm;

  uint8_t disp_size = ampm_size + d_size + bn_m_size[bn_m] + y_size + 3 + 3 + d_size + en_m_size[en_m] + y_size + 6;
  //uint8_t disp_size = ampm_size + d_size + bn_m_size[bn_m] + y_size + 3 + 4 ; //total columns in the display matrix
  //uint8_t disp_size = d_size + en_m_size[en_m] + y_size + 3 + 4 ; //total columns in the display matrix
  
  //Arrays for display
  uint64_t time_mat = 0;
  uint64_t ampm_mat = 0;
  uint64_t ban_dy_mat = 0; //size 64
  uint64_t ban_m_mat = 0; //size 64
  uint64_t eng_dy_mat = 0; //size 64
  uint64_t eng_m_mat = 0; //size 64

  

  for(int i=0;i<16;i++){
      //AMPM
      ampm_mat = ampm_fn( ampm, i, (64 - ampm_size) );
      
      //MONTHS
      eng_m_mat = en_months( en_m, i, (64 - en_m_size[en_m] - 0));
      ban_m_mat = bn_months( bn_m, i, (64 - bn_m_size[bn_m] - 0));

      eng_dy_mat=0;
      ban_dy_mat=0;
      time_mat=0;

      //TIME DOTS
      if((i==4 || i==5 || i==10 || i==11) && second_loop<6 ){
        time_mat = 1;
        time_mat = time_mat << (64 - 2*N1 - 3);
      }
      
      if (i>1 && i<14){

        //TIME
        time_mat = time_mat | digits_fn( tim_e[0], i, (64 - N1) );
        time_mat = time_mat | digits_fn( tim_e[1], i, (64 - 2*N1 - 1) );
        time_mat = time_mat | digits_fn( tim_e[2], i, (64 - 3*N1 - 4) );
        time_mat = time_mat | digits_fn( tim_e[3], i, (64 - 4*N1 - 5) );

        //BENGALI DATE
        ban_dy_mat =              digits_fn( bn_date[0], i, (64 - (N1 + 0)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_date[1], i, (64 - (N1 + N1 + 1)) );

        //BENGALI YEAR
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[0], i, (64 - (N1 + 2*N1 + 5)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[1], i, (64 - (N1 + 3*N1 + 6)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[2], i, (64 - (N1 + 4*N1 + 7)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[3], i, (64 - (N1 + 5*N1 + 8)) );
        
        //ENGLISH DATE
        eng_dy_mat =              digits_fn( en_date[0], i, (64 - (N1 + 0)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_date[1], i, (64 - (N1 + N1 + 1)) );

        //ENGLISH YEAR
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[0], i, (64 - (N1 + 2*N1 + 5)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[1], i, (64 - (N1 + 3*N1 + 6)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[2], i, (64 - (N1 + 4*N1 + 7)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[3], i, (64 - (N1 + 5*N1 + 8)) );
      }

      //COMMA AFTER BENGALI YEAR
      if(i>10) ban_dy_mat = ban_dy_mat | digits_fn( 10, i, (64 - (N1 + 5*N1 + 8 + 3)) );
        
    
    //The seperating Line
    uint8_t offset = 39;
    dmd.setPixel(offset-1,i,GRAPHICS_ON);
    
    for(int j=0;j<38;j++)
    { 
      //TIME
      if ( (time_mat >> (63 - j)) & 0b1 )
        dmd.setPixel(j,i,GRAPHICS_ON);
      else
        dmd.setPixel(j,i,GRAPHICS_OFF); 

      //////////////SCROLLING DATE AND TIME///////////////
      if (j<25){
      uint8_t col_ind = (j+lo_op)%disp_size;
      uint8_t flag = 0;
      
      if (col_ind < ampm_size)
        flag = (ampm_mat >> (63-col_ind) ) & 0b1;
      
      //BANGLA DATE AND TIME 
      else if (col_ind < (ampm_size + d_size))
        flag = (ban_dy_mat >> (63-col_ind +ampm_size) ) & 0b1;
      
      else if (col_ind < (ampm_size + d_size + bn_m_size[bn_m]))
        flag = (ban_m_mat >> (63-col_ind +ampm_size+d_size) ) & 0b1;
        
      else if (col_ind < (ampm_size + d_size + bn_m_size[bn_m]+y_size+3))
        flag = (ban_dy_mat >> (63-col_ind +ampm_size+bn_m_size[bn_m]) ) & 0b1;

      //ENGLISH DATE AND TIME 
      else if (col_ind < (ampm_size + d_size + bn_m_size[bn_m]+y_size+6+d_size))
        flag = (eng_dy_mat >> (63-col_ind +ampm_size + d_size + bn_m_size[bn_m]+y_size+6) ) & 0b1;

      else if (col_ind < (ampm_size + d_size + bn_m_size[bn_m]+y_size+6+d_size + en_m_size[en_m]))
        flag = (eng_m_mat >> (63-col_ind +ampm_size + d_size + bn_m_size[bn_m]+y_size+6+d_size) ) & 0b1;

      else if (col_ind < (ampm_size + d_size + bn_m_size[bn_m]+y_size+6+d_size + en_m_size[en_m] + y_size))
        flag = (eng_dy_mat >> (63-col_ind +ampm_size + d_size + bn_m_size[bn_m]+y_size+6+en_m_size[en_m]) ) & 0b1;
      
      if (flag)
        dmd.setPixel(j+offset,i,GRAPHICS_ON);
      else
        dmd.setPixel(j+offset,i,GRAPHICS_OFF); 
      }
    }

    
  }
//Now wait and scroll
delay(100);
second_loop = (second_loop + 1)%10;
lo_op = (lo_op+1)%disp_size;
}

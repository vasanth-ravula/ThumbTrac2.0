#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <NTPtimeESP.h>
#include <string>
#include <Wire.h> 
#include <WiFiManager.h>
#include <LiquidCrystal_I2C.h>
#define NB_TRYWIFI 10
#define DEBUG_ON

#define RST_PIN 4
#define SS_PIN 2

std::string read_tag_id = "";
MFRC522 mfrc522(SS_PIN, RST_PIN);
int option;
std::string all_tag_ids = "";
String relationid;
String TAGID;
void enroll_tag(std::string read_tag_id);
void match_tag(std::string read_tag_id);
void delete_employee_id(String TAGID);
String get_employee_id_no(void);
void eeprom_clear(void);
String get_employee_id_no(void);
void eeprom_write(std::string tagid, int total_id_length);
std::string eeprom_read(void);
String get_relation_id(void);
uint8_t menu(void);
//uint8_t lock = 4;
NTPtime NTPch("time.nist.gov");
String date;
strDateTime dateTime;
ESP8266WebServer server(80);
String getValue(String data, char separator, int index);
void storecmddata();
void refreshcmds();
void getcmddata();
void getdatetime(String relid);
char charBuf[200];
char charBuf1[200];
char payload1arr[200];
char relBuf[100];
char user1[100]; // char user1[useraddress.length() + 1];    
char user2[100]; //char user2[adminaddress.length() + 1];
String paramname[100];
String paramvalue[100];
String address[100];
String cmds1[100];
String devicecmd;
String devicecmd1;
String cmdname11[100];
String cmdname12[100];
String enrolluser[100];
String cmds2[100];
String cmdids[100];
//String cmdname[100];
String payload1;
String urldata;
// String payload;
int output;
String reference;
int id;
int addrs;
//String relationid;
String relid;
HTTPClient http;
WiFiClient espClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 19800, 60000); 
LiquidCrystal_I2C lcd(0x27, 16, 2);  
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

void setup()   
{
  Serial.begin(9600);
  EEPROM.begin(4096);
  Serial.print("Startup reason:");
  Serial.println(ESP.getResetReason());
  delay(500);
  SPI.begin(); // Initiate  SPI bus
  mfrc522.PCD_Init();
  //EEPROM.begin(512);
  all_tag_ids = eeprom_read();
  Serial.print("all_tag_ids : ");   
  Serial.println(all_tag_ids.c_str());
  delay(100);
  while (!Serial)
  {
    ;
  }
  WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Waiting to connect…");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", storecmddata);
  server.begin(); //Start the server
  Serial.println("Server listening");
  server.handleClient();
      Wire.begin(5,0);  //Wire.begin(SDA,SCL);
       lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  scrollText(1, "Welcome To Relgo Networks Pvt Ltd.", 250, 16);
  timeClient.begin();
  //pinMode(lock, OUTPUT); 
  pinMode(16,OUTPUT);
 // digitalWrite(lock, HIGH);
}
void loop()
{
  server.handleClient();
  refreshcmds();
 // digitalWrite(lock, HIGH);
  //match_tag(read_tag_id);
}

void refreshcmds()
{
//  digitalWrite(lock, HIGH);
  EEPROM.get(0, charBuf);
  Serial.println("charbuf : ");
  Serial.println(charBuf);
  String charbuf1 = charBuf;
  
  for (int i = 0; i < charbuf1.length(); i++)
  {
    cmds2[i] = getValue(charBuf, '$', i);
  }
  for (int j = 0; j < cmds2[j].length(); j++)
  {
    cmdids[j] = getValue(cmds2[j], ';', 1);
  }
  // for (int k = 0; k < cmds2[k].length(); k++)
  // {
  //   cmdname[k] = getValue(cmds2[k], ';', 0);
  // }
  Serial.println("cmdid1 : " + cmdids[0]);
  Serial.println("cmdid2 : " + cmdids[1]);
  Serial.println("cmdid3 : " + cmdids[2]); 
  Serial.println("cmdid4 : " + cmdids[3]); 
  // Serial.println("cmdname1 : " + cmdname[0]);
  // Serial.println("cmdname2 : " + cmdname[1]);
  // Serial.println("cmdname3 : " + cmdname[2]); 
  // Serial.println("cmdname4 : " + cmdname[3]); 

  	char deleteid[100];
    	cmdids[2].toCharArray(deleteid,sizeof(deleteid));
 String delete1 = "";
  for (int i = 0; i < cmdids[2].length(); i++)
  {
    
    if (!isspace(deleteid[i]))
    {
      delete1 += deleteid[i];
    }
  }
  cmdids[2]=delete1;

  Serial.println("cmdid[1]: "+cmdids[2]);
  for (int a = 0; a < 3; a++)
  {
    urldata = "http://server.example.com:5320/Rfid.aspx?commandrid=";
    urldata.concat(cmdids[a]);
    urldata.concat("&get=yes");
    Serial.println(urldata);
    if(a == 0){
    while (1)
    {
      String payload;
      http.begin(urldata);
      //Specify request destination
      // http.addHeader("Content-Type", "text/plain");
      int httpCode = http.GET(); //Send the request
      if (httpCode > 0)
      {                             //Check the returning code
        payload = http.getString(); //Get the request response payload
        //Serial.println(payload);
      }
      payload1 = getValue(payload, '<', 0);
      Serial.println(payload1);
      payload1.toCharArray(payload1arr, sizeof(payload1arr));
      for (int i = 0; i < payload1.length(); i++)
      {
        enrolluser[i] = getValue(payload1arr, '&', i);
      }
      Serial.println("enroll user");
      enrolluser[0] = getValue(enrolluser[0], '?', 1);
      Serial.println(" command history id : " + enrolluser[0]);
      Serial.println(" command id : " + enrolluser[1]);
      enrolluser[2] = getValue(enrolluser[2], '~', 0);
      Serial.println("parameter data : " + enrolluser[2]); //Parameterdata=Useraddress;user@example.com$adminaddress;user@example.com~
      for (int i = 0; i < enrolluser[2].length(); i++)
      {
        address[i] = getValue(enrolluser[2], '$', i);
      }
      if (enrolluser[2] == "")
      {
        for (int i = 0; i < address[i].length(); i++)
        {
          address[i] = "";
        }
      }
      for (uint8_t j = 0; j < address[j].length(); j++)
      {
        paramname[j] = getValue(address[j], ';', 0);
      }

      paramname[0] = getValue(paramname[0], '=', 1);

      for (int k = 0; k < address[k].length(); k++)
      {
        paramvalue[k] = getValue(address[k], ';', 1);
      }

      Serial.println("paramname : " + paramname[0]);
      Serial.println("paramname : " + paramname[1]);
      Serial.println("paramvalue : " + paramvalue[0]);
      //Serial.println("paramvalue : " + paramvalue[1]);
      paramvalue[1] = getValue(paramvalue[1], '~', 0);
      Serial.println("paramvalue : " + paramvalue[1]);
      String s = paramvalue[0];
      String d = paramvalue[1];
      char s1[50];
      char d1[50];
      s.toCharArray(s1, sizeof(s1));
      d.toCharArray(d1, sizeof(d1));
      String dinfouser = "";
      for (int i = 0; i < s.length(); i++)
      {
         if (!isspace(s1[i]))
        {
          dinfouser += s1[i];
        }
      }
      String dinfoadmin = "";
      for (int i = 0; i < d.length(); i++)
      {

        if (!isspace(d1[i]))
        {
          dinfoadmin += d1[i];
        }
      }
      String data = "http://server.example.com:5320/rfid.aspx?Target=";
      data.concat(dinfouser);
      data.concat("&Source=");
      data.concat(dinfoadmin);
      Serial.println(data);
      //Declare an object of class HTTPClient
      

      http.begin(data);           //Specify request destination
                                  // http.addHeader("Content-Type", "text/plain");
      int httpCode1 = http.GET(); //Send the request
      //Serial.println("httpcode : " + httpCode);
      if (httpCode > 0)
      {                                     //Check the returning code
        String payload2 = http.getString(); //Get the request response payload
        relationid = getValue(payload2, '<', 0);
        Serial.println("relation id : " + relationid);
      }

      if (enrolluser[2] == "")
      {
        for (int i = 0; i < 10; i++)
        {
             match_tag(read_tag_id);
             delay(50);        
        }
      }
      else
      {
        enroll_tag(read_tag_id);
        Serial.println("enrolled successfully");
        //Serial.println(output);
          relationid.toCharArray(relBuf, sizeof(relBuf));
          // addrs = 100 + (id * 60);
          // EEPROM.put(addrs, relBuf);
          // EEPROM.commit();
          Serial.println("fingerprint id : " + relationid);
          String Comhisid  = getValue(enrolluser[0], '=', 1);
          String commandid = getValue(enrolluser[1], '=', 1);
          Serial.println("command id : " + commandid); 
          Serial.println("command id : " + Comhisid );
          String data3 = "http://server.example.com:5320/Rfid.aspx?commandrid=";
          data3.concat(commandid);
          data3.concat("&recored=delete&commandhistoryid=");
          data3.concat(Comhisid);
          data3.concat("&output=Completed");
          Serial.println(data3);
          http.begin(data3);
          int httpCode2 = http.GET(); //Send the request
          Serial.println("httpcode : " + httpCode2);
                Serial.println(payload1);

          payload1="";
                Serial.println(payload1);

          for (int a = 0; a < 10; a++)
          {
           match_tag(read_tag_id);
            delay(50);
          }
        
      }
      break;
    }
  }
  if(a==2){
      while (1)
    {
      String payload;
      http.begin(urldata);
      //Specify request destination
      // http.addHeader("Content-Type", "text/plain");
      int httpCode = http.GET(); //Send the request

      if (httpCode > 0)
      {                             //Check the returning code
        payload = http.getString(); //Get the request response payload
        //Serial.println(payload);
      }
      payload1 = getValue(payload, '<', 0);
      Serial.println(payload1);
      payload1.toCharArray(payload1arr, sizeof(payload1arr));
      for (int i = 0; i < payload1.length(); i++)
      {
        enrolluser[i] = getValue(payload1arr, '&', i);
      }
      Serial.println("enroll user");
      enrolluser[0] = getValue(enrolluser[0], '?', 1);
      Serial.println(" command history id : " + enrolluser[0]);
      Serial.println(" command id : " + enrolluser[1]);
      enrolluser[2] = getValue(enrolluser[2], '~', 0);
      Serial.println("parameter data : " + enrolluser[2]); //Parameterdata=Useraddress;user@example.com$adminaddress;user@example.com~

      for (int i = 0; i < enrolluser[2].length(); i++)
      {
        address[i] = getValue(enrolluser[2], '$', i);
      }
      if (enrolluser[2] == "")
      {
        for (int i = 0; i < address[i].length(); i++)
        {
          address[i] = "";
        }
      }
      for (uint8_t j = 0; j < address[j].length(); j++)
      {
        paramname[j] = getValue(address[j], ';', 0);
      }

      paramname[0] = getValue(paramname[0], '=', 1);

      for (int k = 0; k < address[k].length(); k++)
      {
        paramvalue[k] = getValue(address[k], ';', 1);
      }

      Serial.println("paramname : " + paramname[0]);
      Serial.println("paramname : " + paramname[1]);
      Serial.println("paramvalue : " + paramvalue[0]);
      //Serial.println("paramvalue : " + paramvalue[1]);

      paramvalue[1] = getValue(paramvalue[1], '~', 0);
      Serial.println("paramvalue : " + paramvalue[1]);
      String s = paramvalue[0];
      String d = paramvalue[1];
      char s1[50];
      char d1[50];
      s.toCharArray(s1, sizeof(s1));
      d.toCharArray(d1, sizeof(d1));
      String dinfouser = "";
      for (int i = 0; i < s.length(); i++)
      {

        if (!isspace(s1[i]))
        {
          dinfouser += s1[i];
        }
      }
      String dinfoadmin = "";
      for (int i = 0; i < d.length(); i++)
      {

        if (!isspace(d1[i]))
        {
          dinfoadmin += d1[i];
        }
      }
      String data = "http://server.example.com:5320/rfid.aspx?Target=";
      data.concat(dinfouser);
      data.concat("&Source=");
      data.concat(dinfoadmin);
      Serial.println(data);
      //Declare an object of class HTTPClient

      http.begin(data);           //Specify request destination
                                  // http.addHeader("Content-Type", "text/plain");
      int httpCode1 = http.GET(); //Send the request
      //Serial.println("httpcode : " + httpCode);
      if (httpCode > 0)
      {                                     //Check the returning code
        String payload2 = http.getString(); //Get the request response payload
        relationid = getValue(payload2, '<', 0);
        Serial.println("relation id : " + relationid);
      }

      if (enrolluser[2] == "")
      {
        for (int i = 0; i < 10; i++)
        {
             match_tag(read_tag_id);
             delay(50);
         
          
        }
      }
      else
      { 
        String employee_id;
        //relation_id=employee_id;
        delete_employee_id (TAGID);
        Serial.println("deleted sucessfully");
        //Serial.println(output);
          relationid.toCharArray(relBuf, sizeof(relBuf));
          // addrs = 100 + (id * 60);
          // EEPROM.put(addrs, relBuf);
          // EEPROM.commit();
          Serial.println("fingerprint id : " + relationid);
          String Comhisid = getValue(enrolluser[0], '=', 1);
          String commandid = getValue(enrolluser[1], '=', 1);
          Serial.println("command id : " + commandid);
          Serial.println("command id : " + Comhisid);
          String data3 = "http://server.example.com:5320/Rfid.aspx?commandrid=";
          data3.concat(commandid);
          data3.concat("&recored=delete&commandhistoryid=");
          data3.concat(Comhisid);
          data3.concat("&output=Completed");
          Serial.println(data3);
          http.begin(data3);
          int httpCode2 = http.GET(); //Send the request
          Serial.println("httpcode : " + httpCode2);
                Serial.println(payload1);

          payload1="";
                Serial.println(payload1);

          for (int a = 0; a < 10; a++)
          {
           match_tag(read_tag_id);
            delay(50);
          }
        
      }
      break;
    }
  }
  }
}

void storecmddata()
{
   // digitalWrite(lock, HIGH);
  String message = "Number of args received:";
  message += server.args(); //Get number of parameters
  message += "\n";          //Add a new line

  String firstquery = server.argName(0);
  devicecmd1=server.arg(0);
  Serial.println(devicecmd1);
  devicecmd = server.arg(1);
  Serial.println(devicecmd);
  devicecmd+="$name;"+devicecmd1;
  devicecmd.toCharArray(charBuf, sizeof(charBuf));
  devicecmd1.toCharArray(charBuf1, sizeof(charBuf1));
  Serial.println(charBuf);
  Serial.println(charBuf1);
  EEPROM.put(0, charBuf);
  EEPROM.commit();
  Serial.println("stored");
  for (int i = 0; i < devicecmd.length(); i++)
  {
    cmds1[i] = getValue(charBuf, '$', i);
  }
  for (int j = 0; j < cmds1[j].length(); j++)
  {
    cmdname11[j] = getValue(cmds1[j], ';', 0);
  }
  for (int j = 0; j < cmds1[j].length(); j++)
  {
    cmdname12[j] = getValue(cmds1[j], ';', 1);
  }
  Serial.println("cmdname12 : " + cmdname12[0]);
  Serial.println("cmdname12 : " + cmdname12[1]);
  Serial.println("cmdname12 : " + cmdname12[2]);
  Serial.println("cmdname12 : " + cmdname12[3]);
  Serial.println("cmdname11 : " + cmdname11[0]);
  Serial.println("cmdname11 : " + cmdname11[1]);
  Serial.println("cmdname11 : " + cmdname11[2]);
  Serial.println("cmdname12 : " + cmdname11[3]);
  for (int a = 0; a <3; a++)
  {
    urldata = "http://server.example.com:5320/Rfid.aspx?commandrid=";
    urldata.concat(cmdname12[a]);
    urldata.concat("&get=yes");
    if(a==0){
          while (1)
    {
      String payload;
      http.begin(urldata);
      //Specify request destination
      // http.addHeader("Content-Type", "text/plain");
      int httpCode = http.GET(); //Send the request

      if (httpCode > 0)
      {                             //Check the returning code
        payload = http.getString(); //Get the request response payload
        //Serial.println(payload);
      }

      payload1 = getValue(payload, '<', 0);
      Serial.println(payload1);
      payload1.toCharArray(payload1arr, sizeof(payload1arr));
      for (int i = 0; i < payload1.length(); i++)
      {
        enrolluser[i] = getValue(payload1arr, '&', i);
      }

      Serial.println("enroll user");
      enrolluser[0] = getValue(enrolluser[0], '?', 1);
      Serial.println(" command history id : " + enrolluser[0]);
      Serial.println(" command id : " + enrolluser[1]);
      enrolluser[2] = getValue(enrolluser[2], '~', 0);
      Serial.println("parameter data : " + enrolluser[2]); //Parameterdata=Useraddress;user@example.com$adminaddress;user@example.com~

      for (int i = 0; i < enrolluser[2].length(); i++)
      {
        address[i] = getValue(enrolluser[2], '$', i);
      }
      if (enrolluser[2] == "")
      {
        for (int i = 0; i < address[i].length(); i++)
        {
          address[i] = "";
        }
      }
      for (uint8_t j = 0; j < address[j].length(); j++)
      {
        paramname[j] = getValue(address[j], ';', 0);
      }

      paramname[0] = getValue(paramname[0], '=', 1);

      for (int k = 0; k < address[k].length(); k++)
      {
        paramvalue[k] = getValue(address[k], ';', 1);
      }

      Serial.println("paramname : " + paramname[0]);
      Serial.println("paramname : " + paramname[1]);
      Serial.println("paramvalue : " + paramvalue[0]);
      //Serial.println("paramvalue : " + paramvalue[1]);

      paramvalue[1] = getValue(paramvalue[1], '~', 0);
      Serial.println("paramvalue : " + paramvalue[1]);
      String s = paramvalue[0];
      String d = paramvalue[1];
      char s1[50];
      char d1[50];
      s.toCharArray(s1, sizeof(s1));
      d.toCharArray(d1, sizeof(d1));
      String dinfouser = "";
      for (int i = 0; i < s.length(); i++)
      {

        if (!isspace(s1[i]))
        {
          dinfouser += s1[i];
        }
      }
      String dinfoadmin = "";
      for (int i = 0; i < d.length(); i++)
      {

        if (!isspace(d1[i]))
        {
          dinfoadmin += d1[i];
        }
      }
      String data = "http://server.example.com:5320/rfid.aspx?Target=";
      data.concat(dinfouser);
      data.concat("&Source=");
      data.concat(dinfoadmin);
      Serial.println(data);   
      //Declare an object of class HTTPClient

      http.begin(data);           //Specify request destination
                                  // http.addHeader("Content-Type", "text/plain");
      int httpCode1 = http.GET(); //Send the request
      //Serial.println("httpcode : " + httpCode);
      if (httpCode > 0)
      {                                     //Check the returning code
        String payload2 = http.getString(); //Get the request response payload
        relationid = getValue(payload2, '<', 0);
        Serial.println("relation id : " + relationid);
      }

      if (enrolluser[2] == "")
      {
        for (int i = 0; i < 2; i++)
        {
         match_tag(read_tag_id);
          delay(50);
        }
      }
      else
      {
         enroll_tag( read_tag_id);
        
        //Serial.println(output);
          relationid.toCharArray(relBuf, sizeof(relBuf));
          // addrs = 100 + (id * 60);
          // EEPROM.put(addrs, relBuf);
          // EEPROM.commit();
          Serial.println("fingerprint id : " + relationid);
          String Comhisid = getValue(enrolluser[0], '=', 1);
          String commandid = getValue(enrolluser[1], '=', 1);
          Serial.println("command id : " + commandid);
          Serial.println("command id : " + Comhisid);
          String data3 = "http://server.example.com:5320/Rfid.aspx?commandrid=";
          data3.concat(commandid);
          data3.concat("&recored=delete&commandhistoryid=");
          data3.concat(Comhisid);
          data3.concat("&output=Completed");
          Serial.println(data3);
          http.begin(data3);
          int httpCode2 = http.GET(); //Send the request
          Serial.println("httpcode : " + httpCode2);
                Serial.println(payload1);

          payload1="";
                Serial.println(payload1);

          for (int a = 0; a < 10; a++)
          {
            match_tag(read_tag_id);
            delay(50);
          }
        
      }
      break;
    }
    }
    if(a==2){
      while (1)
    {
      String payload;
      http.begin(urldata);
      //Specify request destination
      // http.addHeader("Content-Type", "text/plain");
      int httpCode = http.GET(); //Send the request
      if (httpCode > 0)
      {                             //Check the returning code
        payload = http.getString(); //Get the request response payload
        //Serial.println(payload);
      }
      payload1 = getValue(payload, '<', 0);
      Serial.println(payload1);
      payload1.toCharArray(payload1arr, sizeof(payload1arr));
      for (int i = 0; i < payload1.length(); i++)
      {
        enrolluser[i] = getValue(payload1arr, '&', i);
      }
      Serial.println("enroll user");
      enrolluser[0] = getValue(enrolluser[0], '?', 1);
      Serial.println(" command history id : " + enrolluser[0]);
      Serial.println(" command id : " + enrolluser[1]);
      enrolluser[2] = getValue(enrolluser[2], '~', 0);
      Serial.println("parameter data : " + enrolluser[2]); //Parameterdata=Useraddress;user@example.com$adminaddress;user@example.com~

      for (int i = 0; i < enrolluser[2].length(); i++)
      {
        address[i] = getValue(enrolluser[2], '$', i);
      }
      if (enrolluser[2] == "")
      {
        for (int i = 0; i < address[i].length(); i++)
        {
          address[i] = "";
        }
      }
      for (uint8_t j = 0; j < address[j].length(); j++)
      {
        paramname[j] = getValue(address[j], ';', 0);
      }

      paramname[0] = getValue(paramname[0], '=', 1);

      for (int k = 0; k < address[k].length(); k++)
      {
        paramvalue[k] = getValue(address[k], ';', 1);
      }

      Serial.println("paramname : " + paramname[0]);
      Serial.println("paramname : " + paramname[1]);
      Serial.println("paramvalue : " + paramvalue[0]);
      //Serial.println("paramvalue : " + paramvalue[1]);

      paramvalue[1] = getValue(paramvalue[1], '~', 0);
      Serial.println("paramvalue : " + paramvalue[1]);
      String s = paramvalue[0];
      String d = paramvalue[1];
      char s1[50];
      char d1[50];
      s.toCharArray(s1, sizeof(s1));
      d.toCharArray(d1, sizeof(d1));
      String dinfouser = "";
      for (int i = 0; i < s.length(); i++)
      {

        if (!isspace(s1[i]))
        {
          dinfouser += s1[i];
        }
      }
      String dinfoadmin = "";
      for (int i = 0; i < d.length(); i++)
      {

        if (!isspace(d1[i]))
        {
          dinfoadmin += d1[i];
        }
      }
      String data = "http://server.example.com:5320/rfid.aspx?Target=";
      data.concat(dinfouser);
      data.concat("&Source=");
      data.concat(dinfoadmin);
      Serial.println(data);
      //Declare an object of class HTTPClient

      http.begin(data);           //Specify request destination
                                  // http.addHeader("Content-Type", "text/plain");
      int httpCode1 = http.GET(); //Send the request
      //Serial.println("httpcode : " + httpCode);
      if (httpCode > 0)
      {                                     //Check the returning code
        String payload2 = http.getString(); //Get the request response payload
        relationid = getValue(payload2, '<', 0);
        Serial.println("relation id : " + relationid);
      }

      if (enrolluser[2] == "")
      {
        for (int i = 0; i < 2; i++)
        {
         match_tag(read_tag_id);
          delay(50);
        }
      }
      else
      {
         //enroll_tag( read_tag_id);
           delete_employee_id (TAGID);
        //Serial.println(output);
          relationid.toCharArray(relBuf, sizeof(relBuf));
          // addrs = 100 + (id * 60);
          // EEPROM.put(addrs, relBuf);
          // EEPROM.commit();
          Serial.println("relation id : " + relationid);
          String Comhisid = getValue(enrolluser[0], '=', 1);
          String commandid = getValue(enrolluser[1], '=', 1);
          Serial.println("command id : " + commandid);
          Serial.println("command id : " + Comhisid);
          String data3 = "http://server.example.com:5320/Rfid.aspx?commandrid=";
          data3.concat(commandid);
          data3.concat("&recored=delete&commandhistoryid=");
          data3.concat(Comhisid);
          data3.concat("&output=Completed");
          Serial.println(data3);
          http.begin(data3);
          int httpCode2 = http.GET(); //Send the request
          Serial.println("httpcode : " + httpCode2);
                Serial.println(payload1);

          payload1="";
                Serial.println(payload1);

          for (int a = 0; a < 10; a++)
          {
            match_tag(read_tag_id);
            delay(50);
          }
        
      }
      break;
    }
    }
  
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

 void enroll_tag(std::string read_tag_id)
    {
      lcd.print("enrolling Plz Wait");
      LABLE_ALREADY_PRESENT:
      String TAGID = ""; 
    read_tag_id  = "";
    String office_id_no_and_Tag_id = "";
    String employee_id="";
    //String employee_id="";

      Serial.println("please scan new id card to add");
        while ( ! mfrc522.PICC_IsNewCardPresent()) 
        {
          delay(100);
        }
        // Select one of the cards
        while( ! mfrc522.PICC_ReadCardSerial()) 
        {
          delay(100);
        }

        //Show UID on serial monitor
        Serial.println();
        Serial.print(" UID tag :");

        for (byte i = 0; i < mfrc522.uid.size; i++) 
        {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          TAGID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
          TAGID.concat(String(mfrc522.uid.uidByte[i], HEX));
        }

        TAGID.toUpperCase();
        read_tag_id = TAGID.c_str();
        employee_id = relationid;
        Serial.println();
       // Serial.printf("\nget_id_no --->%d\n",employee_id);

        office_id_no_and_Tag_id = office_id_no_and_Tag_id + (TAGID + employee_id + ',' + '#');

        Serial.print("office_id_no_and_Tag_id : ");
        Serial.println(office_id_no_and_Tag_id);    
        Serial.print("all_tag_ids.find(read_tag_id) : ");
        Serial.println(all_tag_ids.find(read_tag_id));
        if(all_tag_ids.find(read_tag_id)!=-1)
        {
          Serial.println(" \n***Already present in database**** ");
          Serial.println();
          TAGID = "";
          read_tag_id = "";
          Serial.println("please scan new card");
          goto LABLE_ALREADY_PRESENT;
        }

        eeprom_write(office_id_no_and_Tag_id.c_str(),all_tag_ids.length());
        TAGID = "";
        payload1="";
              Serial.println(payload1);

        all_tag_ids = eeprom_read();  
        Serial.print("all_tag_ids : ");
        Serial.println(all_tag_ids.c_str());        
        lcd.clear();    
      
      }

 void match_tag(std::string read_tag_id)
  {
     lcd.print("place RFID Tag");
     String TAGID = "";
     read_tag_id = "";
 
    //
    Serial.println("please scan new id card to check");
   
        delay(1000);
        while ( ! mfrc522.PICC_IsNewCardPresent()) 
        {
          Serial.println("Nocard");
          lcd.clear();
          return;
          //delay(100);
        }
        // Select one of the cards
        while( ! mfrc522.PICC_ReadCardSerial()) 
        {
          delay(100);
        }
        
        //Show UID on serial monitor
        Serial.println();
        Serial.print(" UID tag :");

        for (byte i = 0; i < mfrc522.uid.size; i++) 
        {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          TAGID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
          TAGID.concat(String(mfrc522.uid.uidByte[i], HEX));
        }

        TAGID.toUpperCase();
        read_tag_id = TAGID.c_str();
  
  if(all_tag_ids.find(read_tag_id) !=-1)
  {
    int found = all_tag_ids.find(read_tag_id);
    int reference = found;
    std::string relation_id = "";
     std::string employee_id = "";
    String relid;

    Serial.printf("refererence index ==%d\n",reference);
    if(reference >18)
      while(reference--)
      {
        if(all_tag_ids[reference] == ',')
        {
          reference++;
          for(;reference<found;reference++)
            employee_id += all_tag_ids[reference]; 
          employee_id = employee_id + " " + read_tag_id + " " ;
          break; 
        }
        if(reference ==  found)
          break;
      }
    else
    {
      int k = 0;
      while(reference != k )
      { 
        employee_id += all_tag_ids[k++]; 
      }
      employee_id = employee_id + " " + read_tag_id + " " ;
      relation_id = employee_id;
    }
    employee_id[employee_id.length()-1] = ',';
    employee_id = employee_id + "$";
    reference = found+8;


    while(all_tag_ids[reference]!=',')
    {
      relid = relid+all_tag_ids[reference++];
    }
    
    Serial.println(" Access Granted ");
   Serial.println(" read_tag_id");
   Serial.println(relid);
      getdatetime(relid); 
    
  }
  else
  {
    Serial.println(" Access Denied ");
    lcd.clear();
    lcd.print("UnAuthorized");
    delay(2000);
    lcd.clear();
    //Serial.print(" UID tag :");
    String TAGID="";
    TAGID = ""; 
    delay(3000);
  } 
}
void eeprom_clear(void)
{
  int addr = 0;
  Serial.println("Clearing EEPROM START");
  for (int i = 0; i < 1024; i++)
  {
    EEPROM.write(addr + i, '#');
    delay(5);
  }
  Serial.println("Clearing EEPROM END");
  EEPROM.commit();
}

void eeprom_write(std::string tagid,int total_id_length)
{
  int addr = total_id_length-1 ;

  if(addr<=0)
  {
    addr=0;
  }
  else
  {
    addr = total_id_length-1;
  }
  if(EEPROM.read(0)=='#')
    addr=0;
  Serial.println("EEPROM WRITING START");

  for(int i=0;i<tagid.length();i++)
  {
    EEPROM.write(addr+i, tagid[i]);
    delay(5);
  }
  EEPROM.commit();
  Serial.println("EEPROM WRITING END");
}
std::string eeprom_read(void)
{
  char read_data;
  int addr = 0;
  std::string all_id_cards = "";
  Serial.println("EEPROM READING START");

  do
  {
    if(addr == 1200)
    {
      Serial.println("Memory Full");
      break;
    }
    read_data = EEPROM.read(addr);
    Serial.print(read_data);
    all_id_cards +=read_data; 
    delay(5);  
  }
  while(EEPROM.read(addr++) !='#');

  Serial.println();
  Serial.println("EEPROM READ END");      
  return all_id_cards;
}
void delete_employee_id(String TAGID)
{
      //String TAGID;
        //relation_id=employee_id;
         Serial.println("please scan  id card to delete");
        delay(1000);
        while ( ! mfrc522.PICC_IsNewCardPresent()) 
        {
          Serial.println("Nocard");
         // return;
          delay(1000);
        }
        // Select one of the cards
        while( ! mfrc522.PICC_ReadCardSerial()) 
        {
          delay(100);
        }
        
        //Show UID on serial monitor
        Serial.println();
        Serial.print(" UID tag :");

        for (byte i = 0; i < mfrc522.uid.size; i++) 
        {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          TAGID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
          TAGID.concat(String(mfrc522.uid.uidByte[i], HEX));
        }

        TAGID.toUpperCase();
        read_tag_id = TAGID.c_str();
  
  /*if(all_tag_ids.find(read_tag_id) !=-1)
  {
    int found = all_tag_ids.find(read_tag_id);
    int reference = found;
    std::string relation_id = "";
     std::string employee_id = "";
    String relid;

    Serial.printf("refererence index ==%d\n",reference);
    if(reference >18)
      while(reference--)
      {
        if(all_tag_ids[reference] == ',')
        {
          reference++;
          for(;reference<found;reference++)
            employee_id += all_tag_ids[reference]; 
          employee_id = employee_id + " " + read_tag_id + " " ;
          break; 
        }
        if(reference ==  found)
          break;
      }
    else
    {
      int k = 0;
      while(reference != k )
      { 
        employee_id += all_tag_ids[k++]; 
      }
      employee_id = employee_id + " " + read_tag_id + " " ;
      relation_id = employee_id;
    }
  }*/
  String temp_employee_id  = TAGID;
  String temp = "";
  int found = all_tag_ids.find(temp_employee_id.c_str());

  for(int k =0;k<all_tag_ids.length();k++)
  {
    if(k==found)
    {
      for(;k<all_tag_ids.length();k++)
      {
        if(all_tag_ids[k]==',')
          break;
        Serial.printf("in remove %c\n",all_tag_ids[k]);    
      }
      k++;
    }
    temp = temp + all_tag_ids[k];
    //Serial.println(temp);
  }
  Serial.printf("before deltion -----> %s\n",all_tag_ids.c_str());
  Serial.printf("After temp deltion -----> %s\n",temp.c_str());
  all_tag_ids = "";
  eeprom_clear();  
  for(int l=0;l<temp.length();l++)
  {
    all_tag_ids = all_tag_ids + temp[l]; 
    EEPROM.write(l,temp[l]);
    delay(5);     
  }
  payload1="";
        Serial.println(payload1);

  Serial.printf("After deltion all_tag_ids -----> %s\n",all_tag_ids.c_str());
}    
uint8_t menu(void)
{
  Serial.println(F("\nChoose an option:"));
  Serial.println(F("0 - Cancel"));
  Serial.println(F("1 - Enrolling employee ID"));
  Serial.println(F("2 - Read All employee data base"));
  Serial.println(F("3 - Delete Employee ID"));
  Serial.println(F("4 - Clear all login and logout data"));
  Serial.println(F("5 - Show all login and logout data"));
  Serial.println(F("6 - Clear All employee ID data"));
  Serial.flush();
  //waits while the user does not start data
  while (!Serial.available())
  {
  };
  //retrieves the chosen option
  uint8_t op = (int)Serial.read();
  Serial.printf(" op In menu = %d \n", op);

  //remove all characters after option (as \n per example)
  while (Serial.available())
  {
    if (Serial.read() == '\n')
      break;
    Serial.read();
  }
  return (op - 48); //subtract 48 from read value, 48 is the zero from ascii table
  }

String get_employee_id_no(void)
{
  String id = "";
  Serial.print(F("\nEnter Employee id number :"));
  Serial.flush();
  while (id == 0)
  {
    while (!Serial.available())
      ;
    id = Serial.readString();
  }
  while (Serial.available())
  {
    if (Serial.read() == '\n')
      break;
    Serial.read();
  }
  return id;
}

String get_relation_id(void)
{
  String id = "";
  Serial.print(F("\nEnter relation id number :"));
  Serial.flush();
  while (id == 0)
  {
    while (!Serial.available())
      ;
    id = Serial.readString();
  }
  while (Serial.available())
  {
    if (Serial.read() == '\n')
      break;
    Serial.read();
  }
  return id;
}

void getdatetime(String relid)

{

  String useraddress, adminaddress;
  char rid[100];
	char matchrid[100];
 // uint16_t id1 = finger.fingerID;
  // int addrs1 = 100 + (id1 * 60);
  // EEPROM.get(addrs1, relBuf);

  String relationid1 = String(relid);
  Serial.println("new one : " + relationid1);
  //char rid[relationid1.length() + 1];
  relationid1.toCharArray(rid, sizeof(rid));
	 cmdids[1].toCharArray(matchrid,sizeof(matchrid));
  // char devname[100];
  // cmdname[3].toCharArray(devname,sizeof(devname));
  // String devicename = "";
  // for (int i = 0; i < cmdname[3].length(); i++)
  // {
    
  //   if (!isspace(devname[i]))
  //   {
  //     devicename += devname[i];
  //   }
  // }
  // cmdname[3]=devicename;
  // String devicename1="";
  // char devname1[100];
  //  http.begin("http://server.example.com:5320:5320/Rfid.aspx?userrid=" + cmdname[3] + "&value=target");
  // int httpcode02 = http.GET(); //Send the request
  //                              // Serial.println(code);
  // if (httpcode02 > 0)
  // { //Check the returning code
  //   useraddress = http.getString();
  //   devicename1 = getValue(devicename1, '@', 0);    //Get the request response payload
  //   Serial.println("devicename : " + devicename1); // Serial.println("Adminaddress : "+adminaddress);
  // }

  // devicename1.toCharArray(devname1, sizeof(devname1));
  // String dinfo5 = "";
  // for (int j = 0; j < devicename1.length(); j++)
  // {

  //   if (!isspace(devname1[j]))
  //   {
  //     dinfo5 += devname1[j];
  //   }
  // }
 // http://server.example.com:5320/rfid.aspx?userrid=00000000-0000-0000-0000-000000000000&value=targetname;
  String dinfo3 = "";
  for (int i = 0; i < relationid1.length();i++)
  {

    if (!isspace(rid[i]))
    {
      dinfo3 += rid[i];
    }
  }
  // String dinfo4=dinfo3;
  http.begin("http://server.example.com:5320/Rfid.aspx?userrid="+dinfo3+"&value=target");
  int httpcode1 = http.GET(); //Send the request
                               // Serial.println(code);
  if (httpcode1 > 0)
  { //Check the returning code
    useraddress = http.getString();
    useraddress = getValue(useraddress, '<', 0);    //Get the request response payload
    Serial.println("useraddress : " + useraddress); // Serial.println("Adminaddress : "+adminaddress);
  }

  useraddress.toCharArray(user1, sizeof(user1));
  String dinfo1 = "";
  for (int j = 0; j < useraddress.length(); j++)
  {

    if (!isspace(user1[j]))
    {
      dinfo1 += user1[j];
    }
  }

  http.begin("http://server.example.com:5320/Rfid.aspx?userrid="+dinfo3+"&value=source");
  int httpcode = http.GET(); //Send the request
  // Serial.println(code);
  if (httpcode > 0)
  { //Check the returning code
    adminaddress = http.getString();
    adminaddress = getValue(adminaddress, '<', 0); //Get the request response payload
    Serial.println("admin:" + adminaddress);       // Serial.println("Adminaddress : "+adminaddress);
  }
  adminaddress.toCharArray(user2, sizeof(user2));

  String dinfo2 = "";
  for (int k = 0; k < adminaddress.length(); k++)
  {

    if (!isspace(user2[k]))
    {
      dinfo2 += user2[k];
    }
  }

  Serial.println(dinfo1);
  Serial.println(dinfo2);
  Serial.println(dinfo3);
 // Serial.println(dinfo4);
  EEPROM.get(3800, charBuf);
  Serial.println("charbuf : ");
  Serial.println(charBuf);
  String charbuf2 = charBuf;
  for (int i = 0; i < charbuf2.length(); i++)
  {
    cmds2[i] = getValue(charBuf, '$', i);
  }
  for (int j = 0; j < cmds2[j].length(); j++)
  {
    cmdids[j] = getValue(cmds2[j], ';', 1);
  }
// for (int k = 0; k < cmds2[k].length(); k++)
//   {
//     cmdname[k] = getValue(cmds2[k], ';', 0);
//   }
  String matchid = "";
  for (int i = 0; i < cmdids[1].length(); i++)
  {

    if (!isspace(matchrid[i]))
    {
      matchid += matchrid[i];
    }
  }
  //Serial.println("devicename : " + cmdname[3]); 
  Serial.println("cmdid[1]: "+cmdids[1]);

 
  timeClient.update();

  String time = timeClient.getFormattedTime();
  Serial.println(time);
  dateTime = NTPch.getNTPtime(5.5, 0);

  if (dateTime.valid)
  {

    String month = String(dateTime.month);
    String day = String(dateTime.day);
    String year = String(dateTime.year);
    date = month + "/" + day + "/" + year;
    
  }
  
Serial.print("Date : ");
    Serial.println(date);
  if(date!= NULL && date != ""&& time != NULL && time!="" && dinfo1 !=NULL && dinfo1 !="" && dinfo2 != NULL && dinfo2 !=""){
      digitalWrite(16, HIGH);
        
    //digitalWrite(lock, LOW);
    lcd.clear();
        lcd.print("please wait");

  String data1 = "http://server.example.com:5320/Rfid.aspx?commandid=";
	data1.concat(matchid);  
  data1.concat("&Parameterdata=Date;");
  data1.concat(date);
  data1.concat("$Time;");
  data1.concat(time);
  // data1.concat("$devicename;");
  // data1.concat(dinfo5);
  data1.concat("$Location;PlotNo191,TagoreTowers,Phase-IIGuttalabegumpeta,KavuriHills,Madhapur,Hyderabad,500033,India$Useraddress;");
  data1.concat(dinfo1);
  data1.concat("&userrelationid=");
  data1.concat(dinfo3);
  data1.concat("&Useraddress=");
  data1.concat(dinfo2);
  Serial.println(data1);
  http.begin(data1);
  int httpCode = http.GET(); //Send the request
  Serial.println(httpCode);
  if(httpCode == -11){
         lcd.clear();
    // delay (10000);
   scrollText(1, dinfo1, 250, 16);
    lcd.print("Attdence Added");
    lcd.print(dinfo1);
    delay(3000);
    digitalWrite(16, LOW);
   // digitalWrite(lock, HIGH);
    lcd.clear();
  }
  else
  {
         lcd.clear();
          lcd.print("match once again");
          //   digitalWrite(redlight,HIGH);
             delay(1000);
          //   digitalWrite(redlight,LOW);
             lcd.clear();
              if(WiFi.status() != WL_CONNECTED){
                lcd.print("Wifi Disconnected");
                lcd.clear();
  //    if (!wifiManager.autoConnect("AutoConnectAP")) {
  // if (!wifiManager.startConfigPortal("AutoConnectAP")) {
  //    Serial.println("failed to connect and hit timeout");
  //    delay(3000);
  //    //reset and try again, or maybe put it to deep sleep
  //    ESP.reset();
  //    delay(8000);
  // }
  // }
   }
    lcd.clear();
  }
  
  }
  }
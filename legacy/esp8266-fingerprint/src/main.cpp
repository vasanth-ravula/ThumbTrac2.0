#include <Arduino.h>
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
#include <stdlib.h>
#include <stdio.h>
#include <Wire.h> 
#include <WiFiManager.h>
#include <LiquidCrystal_I2C.h>
#define NB_TRYWIFI 10
#define DEBUG_ON
std::string all_tag_ids = "";
NTPtime NTPch("time.nist.gov");
String date;
strDateTime dateTime;
ESP8266WebServer server(80);
String getValue(String data, char separator, int index);
void storecmddata();
void refreshcmds();
void getcmddata();
void matchfingerprint();
void getEnrollUser(int id);
void getdatetime();
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
String devicecmd1;
String devicecmd;
String cmdname11[100];
String cmdname12[100];
String enrolluser[100];
String cmds2[100];
String cmdids[100];
String payload1;
String urldata;
String payload;
int getFingerprintID();
uint8_t deleteFingerprint(int id);
uint8_t getFingerprintEnroll(int id);
int id;
uint8_t greenlight = 14 ;
uint8_t doorlock = 13;
uint8_t buzzer = 12;
uint8_t redlight = 16;//uint8_t doorlock = 13;

int addrs;
String relationid;
SoftwareSerial mySerial(5, 4);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
HTTPClient http;
WiFiClient espClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 19800, 60000);
String eeprom_read();
     WiFiManager wifiManager;

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
  // pinMode(16,OUTPUT);
  //   pinMode(0,OUTPUT);
  //pinMode(doorlock,OUTPUT);
  EEPROM.begin(4096);
  Serial.print("Startup reason:");
  Serial.println(ESP.getResetReason());
  // int addr = 0;
	// Serial.println("Clearing EEPROM START");  
	// for(int i = 0; i <4096; i++)
	// {
	// 	EEPROM.write(addr+i, '#');
	// 	delay(5);
	// }
	// Serial.println("Clearing EEPROM END");  
	// EEPROM.commit();

  eeprom_read();
  //all_tag_ids = eeprom_read();
  while (!Serial)
  {
    ;
  }
  // WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
  // WiFi.mode(WIFI_STA);
  if (!wifiManager.autoConnect("AutoConnectAP")) {
  if (!wifiManager.startConfigPortal("AutoConnectAP")) {
     Serial.println("failed to connect and hit timeout");
     delay(3000);
     //reset and try again, or maybe put it to deep sleep
     ESP.reset();
     delay(8000);
  }
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Waiting to connect…");
     lcd.print("Connecting Wifi..");
  }
  server.handleClient();

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", storecmddata);
  server.begin(); //Start the server
  Serial.println("Server listening");
  server.handleClient();
    Wire.begin(2,0);  //Wire.begin(SDA,SCL);
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
//    scrollText(1, "Welcome To Relgo Networks Pvt Ltd.", 250, 16);
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
      ;
  }
    timeClient.begin();
    pinMode(buzzer,OUTPUT);
    pinMode(greenlight,OUTPUT);
    pinMode(redlight,OUTPUT);
    pinMode(doorlock,OUTPUT);
//  digitalWrite(doorlock,HIGH);
}

// uint8_t readnumber(void)
// {
//   uint8_t num = 0;

//   while (num == 0)
//   {
//     while (!Serial.available());
//     num = Serial.parseInt();
//   }
//   return num;
// }
void loop()
{
  server.handleClient();
  server.on("/", storecmddata);
  refreshcmds();
  matchfingerprint();
  digitalWrite(doorlock,LOW);
 }
void refreshcmds()
{
    server.on("/", storecmddata);

  digitalWrite(doorlock,LOW);
  server.begin();
   server.handleClient();
  EEPROM.get(0, charBuf);
  char mid[100];
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
 Serial.println("cmdid1 : " + cmdids[0]);
 Serial.println("cmdid2 : " + cmdids[1]);
 Serial.println("cmdid3 : " + cmdids[2]); 
   cmdids[0].toCharArray(mid,sizeof(mid));
  String dinfo5 = "";
  for (int i = 0; i < cmdids[0].length(); i++)
  {
    if (!isspace(mid[i]))
    {
      dinfo5 += mid[i];
     
    }
  }
   cmdids[0]=dinfo5;
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
  Serial.println("cmdid[1]: "+cmdids[0]);
  while (1)
  {
       server.handleClient();  
  for (int a = 0; a <3; a++)
  {
    urldata = "http://server.example.com:5320/rfid.aspx?commandrid=";
    urldata.concat(cmdids[a]);
    urldata.concat("&get=yes");
    Serial.println(urldata);
    if(a==0)
    {
        while (1)
    {
      String payload; 
      http.begin(urldata);
      //Specify request destination
      // http.addHeader("Content-Type", "text/plain");
      int httpCode = http.GET(); //Send the request
      if (httpCode > 0)
      {                              //Check the returning code
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
      Serial.println("paramname : " + paramname[2]);
      Serial.println("paramvalue : " + paramvalue[0]);
      //Serial.println("paramvalue : " + paramvalue[1]);
      paramvalue[1] = getValue(paramvalue[1], '~', 0);
      Serial.println("paramvalue : " + paramvalue[1]);
      Serial.println("paramvalue : " + paramvalue[2]);
      String s = paramvalue[0];
      String d = paramvalue[1];
      String k = paramvalue[2];

      char s1[50];
      char d1[50];
      char k1[50];
      s.toCharArray(s1, sizeof(s1));
      d.toCharArray(d1, sizeof(d1));
      k.toCharArray(k1,sizeof(k1));
      Serial.printf(k1);
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
       data.concat("&Relationtype=Employee");
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
        for (int i = 0; i < 30; i++)
        {
          matchfingerprint();
             delay(50);        
        }
      }
      else
      {
        lcd.clear();
        lcd.print("Enrolling.....");  
         String empid12 = "";
      for (int i = 0; i < k.length(); i++)
      {
         if (!isspace(k1[i]))
        {
          empid12 += k1[i];
        }
      }
      Serial.println("empid ="+empid12);
            Serial.printf(k1);
               int dec=0;
        int len = strlen(k1);
	    for(int i=0; i<len; i++)
      {
	        	dec = dec * 10 + ( k1[i] - '0' );
            Serial.println("dec="+dec);
      	}
        Serial.println("output dec="+dec);
        id=dec;
         digitalWrite(buzzer,HIGH);
         delay(5000);
        digitalWrite(buzzer,LOW);
                  lcd.clear();

        Serial.println("id : " + id);
        if (id == 0)
        { // ID #0 not allowed, try again!
          return;
        }
        Serial.print("Enrolling ID #");
        Serial.println(id);
        uint8_t output = getFingerprintEnroll(id);
        Serial.println(output);
        
        if (output == 9)
        {
          relationid.toCharArray(relBuf, sizeof(relBuf));
           addrs = 100 + (id * 60);
          EEPROM.put(addrs, relBuf);
          EEPROM.commit();
          Serial.println("fingerprint id : " + relationid);
          String Comhisid = getValue(enrolluser[0], '=', 1);
          String commandid = getValue(enrolluser[1], '=', 1);
          Serial.println("command id : " + commandid);
          Serial.println("command id : " + Comhisid);
          String data3 = "http://server.example.com:5320/rfid.aspx?commandrid=";
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
                lcd.clear();
                 lcd.print("Enrolled Successfully..");
            digitalWrite(greenlight,HIGH);
             delay(2000);
             digitalWrite(greenlight,LOW);
             lcd.clear(); 
          for (int a = 0; a < 30; a++)
          {
          matchfingerprint();
            delay(50);
          }
        
    }
    else{
      lcd.clear();
      lcd.print("Enrolled Falied"); 
             digitalWrite(redlight,HIGH);
             delay(2000);
             digitalWrite(redlight,LOW);
             lcd.clear();
      lcd.print("Match Again"); 
      delay(1000);
      lcd.clear();
    }
      }
      
  }
  break;
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
      data.concat("&Relationtype=Employee");
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
        for(int i = 0; i < 30; i++)
        {
          matchfingerprint();
             delay(50);
         
          
        }
      }
      else
      { 
        String employee_id;
        //relation_id=employee_id;
      //  delete_employee_id (TAGID);
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
          String data3 = "http://server.example.com:5320/rfid.aspx?commandrid=";
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

          for (int a = 0; a < 30; a++)
          {
          matchfingerprint();
            delay(50);
          }
        }
      
      break;
    }
    }
  }
  continue;
   }
}
void storecmddata()
{

  String message = "Number of args received:";
  message += server.args(); //Get number of parameters
  message += "\n";          //Add a new line

  String firstquery = server.argName(0);
  devicecmd1=server.arg(0);
  Serial.println(devicecmd1);
  devicecmd = server.arg(1) + "\n";
  Serial.println(devicecmd);
  devicecmd.toCharArray(charBuf, sizeof(charBuf));
  devicecmd1.toCharArray(charBuf1, sizeof(charBuf1));
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
while(1){
  for (int a = 0; a < 3; a++)
  {
    urldata = "http://server.example.com:5320/rfid.aspx?commandrid=";
    urldata.concat(cmdname12[a]);
    urldata.concat("&get=yes");
    if(a==0)
    {
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
      Serial.println("paramvalue : " + paramvalue[2]);
      String s = paramvalue[0];
      String d = paramvalue[1];
      String k = paramvalue[2];
      char s1[50];
      char d1[50];
      char k1[50];
      s.toCharArray(s1, sizeof(s1));
      d.toCharArray(d1, sizeof(d1));
      k.toCharArray(k1,sizeof(k1));
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
      data.concat("&Relationtype=Employee");
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
        for (int i = 0; i < 30; i++)
        {
          matchfingerprint();
             delay(50);        
        }
      }
      else
      {
         String empid12 = "";
      for (int i = 0; i < k.length(); i++)
      {
         if (!isspace(k1[i]))
        {
          empid12 += k1[i];
        }
      }
      Serial.println("empid ="+empid12);
            Serial.printf(k1);
               int dec=0;
        int len = strlen(k1);
	    for(int i=0; i<len; i++)
      {
	        	dec = dec * 10 + ( k1[i] - '0' );
            Serial.println("dec="+dec);
      	}
        Serial.println("output dec="+dec);
        id=dec;  
        digitalWrite(buzzer ,HIGH);
        delay(5000);
        digitalWrite(buzzer,LOW);
       // enroll_tag(read_tag_id);
      //  Serial.println("enrolled sucessfully");
        //Serial.println(output);
        // id = readnumber();
        // Serial.println("id : " + id);

        if (id == 0)
        { // ID #0 not allowed, try again!
          return;
        }
        Serial.print("Enrolling ID #");
        Serial.println(id);

        uint8_t output = getFingerprintEnroll(id);
        Serial.println(output);
        
        if (output == 9)
        {
          relationid.toCharArray(relBuf, sizeof(relBuf));
           addrs = 100 + (id * 60);
          EEPROM.put(addrs, relBuf);
          EEPROM.commit();
         // relationid.toCharArray(relBuf, sizeof(relBuf));
          // addrs = 100 + (id * 60);
          // EEPROM.put(addrs, relBuf);
          // EEPROM.commit();
          Serial.println("fingerprint id : " + relationid);
          String Comhisid = getValue(enrolluser[0], '=', 1);
          String commandid = getValue(enrolluser[1], '=', 1);
          Serial.println("command id : " + commandid);
          Serial.println("command id : " + Comhisid);
          String data3 = "http://server.example.com:5320/rfid.aspx?commandrid=";
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
              digitalWrite(greenlight,HIGH);
               delay(2000);
               digitalWrite(greenlight,LOW);
          for (int a = 0; a < 30; a++)
          {
          matchfingerprint();
            delay(50);
          }
        
      }
      else{
             digitalWrite(redlight,HIGH);
             delay(2000);
             digitalWrite(redlight,LOW);
      }
      
    }    
    break;
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
      data.concat("&Relationtype=Employee");
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
        for(int i = 0; i < 30; i++)
        {
          matchfingerprint();
             delay(50);
          
        }
      }
      else
      { 
        String employee_id;
        //relation_id=employee_id;
      //  delete_employee_id (TAGID);
        deleteFingerprint(id);
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
          String data3 = "http://server.example.com:5320/rfid.aspx?commandrid=";
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

          for (int a = 0; a < 30; a++)
          {
          matchfingerprint();
            delay(50);
          }
        }
      }
      break;
    }
  }
    // while (1)
    // {
    //   http.begin(urldata);
    //   //Specify request destination
    //   // http.addHeader("Content-Type", "text/plain");
    //   int httpCode = http.GET(); //Send the request

    //   if (httpCode > 0)
    //   {                             //Check the returning code
    //     payload = http.getString(); //Get the request response payload
    //     //Serial.println(payload);
    //   }

    //   payload1 = getValue(payload, '<', 0);
    //   Serial.println(payload1);
    //   payload1.toCharArray(payload1arr, sizeof(payload1arr));
    //   for (int i = 0; i < payload1.length(); i++)
    //   {
    //     enrolluser[i] = getValue(payload1arr, '&', i);
    //   }

    //   Serial.println("enroll user");
    //   enrolluser[0] = getValue(enrolluser[0], '?', 1);
    //   Serial.println(" command history id : " + enrolluser[0]);
    //   Serial.println(" command id : " + enrolluser[1]);
    //   enrolluser[2] = getValue(enrolluser[2], '~', 0);
    //   Serial.println("parameter data : " + enrolluser[2]); //Parameterdata=Useraddress;user@example.com$adminaddress;user@example.com~

    //   for (int i = 0; i < enrolluser[2].length(); i++)
    //   {
    //     address[i] = getValue(enrolluser[2], '$', i);
    //   }
    //   if (enrolluser[2] == "")
    //   {
    //     for (int i = 0; i < address[i].length(); i++)
    //     {
    //       address[i] = "";
    //     }
    //   }
    //   for (uint8_t j = 0; j < address[j].length(); j++)
    //   {
    //     paramname[j] = getValue(address[j], ';', 0);
    //   }

    //   paramname[0] = getValue(paramname[0], '=', 1);

    //   for (int k = 0; k < address[k].length(); k++)
    //   {
    //     paramvalue[k] = getValue(address[k], ';', 1);
    //   }

    //   Serial.println("paramname : " + paramname[0]);
    //   Serial.println("paramname : " + paramname[1]);
    //   Serial.println("paramvalue : " + paramvalue[0]);
    //   //Serial.println("paramvalue : " + paramvalue[1]);

    //   paramvalue[1] = getValue(paramvalue[1], '~', 0);
    //   Serial.println("paramvalue : " + paramvalue[1]);
    //   String s = paramvalue[0];
    //   String d = paramvalue[1];
    //   char s1[50];
    //   char d1[50];
    //   s.toCharArray(s1, sizeof(s1));
    //   d.toCharArray(d1, sizeof(d1));
    //   String dinfouser = "";
    //   for (int i = 0; i < s.length(); i++)
    //   {

    //     if (!isspace(s1[i]))
    //     {
    //       dinfouser += s1[i];
    //     }
    //   }
    //   String dinfoadmin = "";
    //   for (int i = 0; i < d.length(); i++)
    //   {

    //     if (!isspace(d1[i]))
    //     {
    //       dinfoadmin += d1[i];
    //     }
    //   }
    //   String data = "http://server.example.com:5320/rfid.aspx?Target=";
    //   data.concat(dinfouser);
    //   data.concat("&Source=");
    //   data.concat(dinfoadmin);
    //   Serial.println(data);
    //   //Declare an object of class HTTPClient

    //   http.begin(data);           //Specify request destination
    //                               // http.addHeader("Content-Type", "text/plain");
    //   int httpCode1 = http.GET(); //Send the request
    //   //Serial.println("httpcode : " + httpCode);
    //   if (httpCode > 0)
    //   {                                     //Check the returning code
    //     String payload2 = http.getString(); //Get the request response payload
    //     relationid = getValue(payload2, '<', 0);
    //     Serial.println("relation id : " + relationid);
    //   }

    //   if (enrolluser[2] == "")
    //   {
    //     for (int i = 0; i < 50; i++)
    //     {
    //       matchfingerprint();
    //       delay(50);
    //     }
    //   }
    //   else
    //   {
    //     id = readnumber();
    //     Serial.println("id : " + id);

    //     if (id == 0)
    //     { // ID #0 not allowed, try again!
    //       return;
    //     }
    //     Serial.print("Enrolling ID #");
    //     Serial.println(id);

    //     uint8_t output = getFingerprintEnroll(id);
    //     Serial.println(output);
    //     if (output == 9)
    //     {
    //       relationid.toCharArray(relBuf, sizeof(relBuf));
    //       addrs = 100 + (id * 60);
    //       EEPROM.put(addrs, relBuf);
    //       EEPROM.commit();
    //       Serial.println("fingerprint id : " + relationid);
    //       String Comhisid = getValue(enrolluser[0], '=', 1);
    //       String commandid = getValue(enrolluser[1], '=', 1);
    //       Serial.println("command id : " + commandid);
    //       Serial.println("command id : " + Comhisid);
    //       String data3 = "http://server.example.com:5320/rfid.aspx?commandrid=";
    //       data3.concat(commandid);
    //       data3.concat("&recored=delete&commandhistoryid=");
    //       data3.concat(Comhisid);
    //       data3.concat("&output=Completed");
    //       Serial.println(data3);
    //       http.begin(data3);
    //       int httpCode2 = http.GET(); //Send the request
    //       Serial.println("httpcode : " + httpCode2);

    //       for (int a = 0; a < 50; a++)
    //       {
    //         matchfingerprint();
    //         delay(50);
    //       }
    //     }
    //   }
    // }
  }
}
}
String eeprom_read(void)
{
  char read_data;
  int addr = 0;
  String all_id_cards = "";
  Serial.println("EEPROM READING START");

  do
  {
    if (addr == 1024)
    {
      Serial.println("Memory Full");
      break;
    }
    read_data = EEPROM.read(addr);
    Serial.print(read_data);
    all_id_cards += read_data;
    delay(5);
  } while (EEPROM.read(addr++) != '#');

  Serial.println();
  Serial.println("EEPROM READ END");
  return all_id_cards;
}
void matchfingerprint()
{
  getFingerprintID();
  delay(50);
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
uint8_t deleteFingerprint(int id) {
  uint8_t p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return p;
  }   
}
uint8_t getFingerprintEnroll(int id)
{
  uint8_t p = -1;
  Serial.println("Waiting for valid finger to enroll");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  lcd.clear();
  lcd.print("Remove Finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.print("place same Finger");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
    lcd.clear();
    lcd.print("fingerprints matched");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    lcd.clear();
    lcd.print("Fingerprints not matched");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }
}

int getFingerprintID()
{
  uint8_t p = finger.getImage();
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image taken");
    break;
  case FINGERPRINT_NOFINGER:
    Serial.println("No finger detected");
      lcd.print("place the finger");
      delay(1000);
      lcd.clear();
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_IMAGEFAIL:
    Serial.println("Imaging error");
    return p;
  default:
    Serial.println("Unknown error");
    return p;  
  }

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK)
  { 
    Serial.println("Found a print match!");
    //int tsk=finger.fingerI
    getdatetime();
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Did not find a match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  // Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

void getdatetime()
{
  String useraddress, adminaddress;
  char rid[100];
  char matchrid[100];
  uint16_t id1 = finger.fingerID;
  int addrs1 = 100 + (id1 * 60);
  EEPROM.get(addrs1, relBuf);
  String relationid1 = relBuf;
  Serial.println("new one : " + relationid1); 
  //char rid[relationid1.length() + 1];
  relationid1.toCharArray(rid, sizeof(rid));
  cmdids[1].toCharArray(matchrid,sizeof(matchrid));
  String dinfo3 = "";
  for (int i = 0; i < relationid1.length(); i++)
  {
    if (!isspace(rid[i]))
    {
      dinfo3 += rid[i];
    }
  }
  http.begin("http://server.example.com:5320/rfid.aspx?userrid=" + dinfo3 + "&value=target");
  int httpcode01 = http.GET(); //Send the request  
  if (httpcode01 > 0)
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

  http.begin("http://server.example.com:5320/rfid.aspx?userrid=" + dinfo3 + "&value=source");
  int Httpcode = http.GET(); //Send the request
  // Serial.println(code);
  if (Httpcode > 0)
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
  
  EEPROM.get(0, charBuf);
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

  String matchid = "";
  for (int i = 0; i < cmdids[1].length(); i++)
  {

    if (!isspace(matchrid[i]))
    {
      matchid += matchrid[i];
    }
  }
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
  digitalWrite(greenlight,HIGH);
  lcd.print("please wait");
   digitalWrite(doorlock,HIGH);
     digitalWrite(buzzer,HIGH);
   lcd.clear();
  String data1 = "http://server.example.com:5320/rfid.aspx?commandid=";
	data1.concat(matchid);
  data1.concat("&Parameterdata=Date;");
  data1.concat(date);
  data1.concat("$Time;");
  data1.concat(time);
  data1.concat("$Location;Banglore$Useraddress;");
  data1.concat(dinfo1);
  data1.concat("&userrelationid=");
  data1.concat(dinfo3);
  data1.concat("&Useraddress=");
  data1.concat(dinfo2);
  Serial.println(data1);
  http.begin(data1);
  int httpCode = http.GET(); //Send the request
  Serial.println(httpCode);
 //digitalWrite(doorlock,HIGH);
  digitalWrite(buzzer,HIGH);
  delay(1000);
  digitalWrite(buzzer,LOW);
  lcd.print("Attdence Added Successfully");
  digitalWrite(buzzer,HIGH);
 delay(1000);
  digitalWrite(buzzer,LOW);
digitalWrite(greenlight,LOW);
digitalWrite(doorlock,LOW); 
lcd.clear(); // if (httpCode > 0)
  // {                                     //Check the returning code
  //   String responce = http.getString(); //Get the request response payload
  //   Serial.println("final responce : " + responce);
  // }
  //  String commandid="";
  //  String Userrid="";
  //  String Useraddress = "";//http clent request pass userrid and value=Target
  //  String adminaddress = "";//http clent request pass userrid and value=Source
  //  String taime="";
  //  String Date="";
}else{
  
             lcd.print("match once again");
             digitalWrite(redlight,HIGH);
             delay(2000);
             digitalWrite(redlight,LOW);
             lcd.clear();
              if(WiFi.status() != WL_CONNECTED){
                lcd.print("Wifi Disconnected");
                lcd.clear();
     if (!wifiManager.autoConnect("AutoConnectAP")) {
  if (!wifiManager.startConfigPortal("AutoConnectAP")) {
     Serial.println("failed to connect and hit timeout");
     delay(3000);
     //reset and try again, or maybe put it to deep sleep
     ESP.reset();
     delay(8000);
  }
  }
   }
}
}
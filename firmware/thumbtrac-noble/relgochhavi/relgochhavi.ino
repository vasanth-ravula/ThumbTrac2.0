#define VERSION_INFO "1.2.000"  // Released to Public
#include <AsyncElegantOTA.h>
#include <Hash.h>
#include <elegantWebpage.h>
#include "soc/rtc_wdt.h"
#include <LiquidCrystal.h>
#include "HTTPClient.h"
#include <SPIFFS.h>
#include "Arduino.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <List.hpp>
#include <string.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <UrlEncode.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SPIFFS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"
#include "sntp.h"
#include <esp_task_wdt.h>

const char logentries_html[] = R"logpage(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Log Entries</title>
</head>
<body>
<h2>Download Log entries as CSV</h2>
<p>Click the link below to download the log entries as a CSV file:</p>
)logpage";

const char index_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>:: Relgo Thumb Trac ::</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style type="text/css">
        html {
            font-family: Arial, Helvetica, sans-serif;
            text-align: center;
        }
        body {
            margin: 0;
        }
        .topnav {
            overflow: hidden;
            background-color: #FF7F27;
        }
            .topnav h1 {
                font-size: 36px;
                color: #000;
                font-weight: 400;
                margin: 10px 0px;
            }
            .topnav h3 {
                font-size: 20px;
                color: #337ab7;
                margin: 0px 0px 15px 0px;
                font-weight: normal;
            }
        .content {
            padding: 30px;
            max-width: 800px;
            margin: 0 auto;
        }
        .card {
            background-color: #F8F7F9;
            /*box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);*/
            padding-top: 20px;
            padding-bottom: 20px;
            border-radius: 5px;
            border: solid 1px #d5d5d5;
            margin-left:200px;
            text-align:left;
            padding-left:50px;
        }
        .card h5 {
            font-size: 17px;
            padding: 0px;
            margin: 10px 0px;
            font-weight: normal;
        }
        input[type=text] {
            padding: 6px 12px;
            font-size: 14px;
            color: #555;
            background-color: #fff;
            background-image: none;
            border: 1px solid #ccc;
            border-radius: 4px;
            width: 75%;
        }
        .state {
            font-size: 1.5rem;
            color: #8c8c8c;
            font-weight: bold;
        }
    </style>
</head>

<body>
    <div class="home">
        <div class="topnav">
            <h1>Relgo Thumb Trac Administration</h1>
            <h3>Press command key for using your device.</h3>
        </div>
        <div class="content">
            <div class="card">
                <h5><strong>c</strong> : Configure Device  </h5>  
                <h5><strong>r</strong> : Register Device  </h5>
                <h5><strong>e</strong> : Enroll User (works with single location only) </h5>
                <h5><strong>i</strong> : Get Device Information  </h5>
                <h5><strong>t</strong> : Get Time log entries (works with single location only)  </h5>
                <h5><strong>d</strong> : Delete log entries   </h5><br/>
                <h5><strong>w</strong> : Wipe Device   </h5><br/>
                <label for="fname">Input :</label>
                <input type="text" id="fname" name="fname" onkeyup="commandevent()" autocomplete="off" placeholder="Enter Input Value"><br />
                (Press Enter)
            </div>
        </div>
    </div>
    <script>
        function commandevent() {
            console.log("commandevent");
            // Number 13 is the "Enter" key on the keyboard
            if (event.keyCode == 13) {
                // Cancel the default action, if needed
                event.preventDefault();
                new_value = document.getElementById("fname").value;
                switch (new_value) {
                    case 'c':
                        window.location.assign("./configurePage");
                        break;
                    case 'r':
                        window.location.assign("./deviceMode?register");
                        break;
                    case 'e':
                        window.location.assign("./enrollUser");
                        break;
                    case 'i':
                        window.location.assign("./deviceMode?info");
                        break;
                    case 't':
                        window.location.assign("./deviceMode?logentries");
                        break;
                    case 'd':
                        window.location.assign("./resetDevice?logentries");
                        break;
                    case 'w':
                        window.location.assign("./resetDevice?wipe&templates&logentries");
                        break;
                    default:
                        window.location.assign("./deviceMode");
                        break;
                }
                document.getElementById("fname").value = "";
            }
        }
    </script>
</body>
</html>
)rawliteral";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
long gmtOffset_sec = -16 * 3600;
int daylightOffset_sec = -15 * 3600;

//const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

const char *time_zone = "PST8PDT,M3.2.0,M11.1.0";  // TimeZone rule for Pacific including daylight adjustment rules (optional)


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16

extern "C" {
#include "bep_host_if.h"
#include "com_common.h"
#include "platform.h"
#include "fpc_bep_types.h"
#include "fpc_hcp_common.h"
#include "fpc_com_result.h"
#include "fpc_hcp.h"
#include "platform.h"
#include "com_common.h"
#include "bep_host_if.h"
}

#include <SPI.h>
#define FORMAT_SPIFFS_IF_FAILED true
#define SCK_PIN 14
#define MISO_PIN 12
#define MOSI_PIN 13
#define SS_PIN 15
#define VIBRATE 33
#define LED_1 26
#define LED_2 27
#define TEMPLATE_SIZE 2048

SPIClass SPI_T(HSPI);

static const int spiClk = 10000000;
// Set your Static IP address
IPAddress local_IP(10, 1, 10, 1);
// Set your Gateway IP address
IPAddress gateway(10, 1, 1, 1);
String localIP = "Not Set";
String localGateway = "Not Set";
String apIP = "Not Set";
String relgoServer = "";
String serviceType = "MultiLocation";
String relgoAdmin = "";
String deviceName = "Relgo";
bool wifiConnected = false;

IPAddress subnet(255, 255, 252, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

String ssid = "";
String password = "";
String location = "";
String timezone = "PST/PDT";
uint16_t template_counter = 0;
bool getCommandsFromRelgo = true;
bool enrollInprogress = false;

WiFiClient wifiClient;
HTTPClient http;

uint16_t waitForFinger(String);

// Structure save device file.
struct relationTemplates {
  uint16_t templateId;
  unsigned char relationId[16];  // Guid
  //byte template[512]; // FP template
};
#define MAX_PENDING_COMMANDS 100
#define ALREADY_ENROLLED -2
#define MAX_DEVICE_COMMANDS 20
#define MAX_DEVICES 10


// Pending commands saved as the requests for devices
struct PendingCommand {
  unsigned long requesttime;
  String deviceRelationId;
  int historyId;
  String command;
  String parameters;
} pendingCommand[MAX_PENDING_COMMANDS];

List<struct PendingCommand> listPendingCommands;
List<int> commandHistory;
static int pendingCommandsCount = 0;


struct DeviceInfo {
  String deviceRelationId;
  String deviceCommands;
  String serviceUrl;
} deviceInfo[MAX_DEVICES];
int deviceCount = 0;

bool is_new_message = false;
String final_message;
bool ledState = 0;
const int ledPin = 2;

const char config_html[]  = R"configpage(
<!DOCTYPE HTML>
<html>
<head>
    <title>:: Relgo Thumb Trac ::</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        html {
            font-family: Arial, Helvetica, sans-serif;
            text-align: left;
            margin: 0px auto;
        }

        body {
            margin: 0;
        }

        .topnav {
            overflow: hidden;
            background-color: #FF7F27;
            text-align: center;
        }

            .topnav h1 {
                font-size: 36px;
                color: #000;
                font-weight: 400;
                margin: 10px 0px;
            }

            .topnav h3 {
                font-size: 20px;
                color: #337ab7;
                margin: 0px 0px 15px 0px;
                font-weight: normal;
            }

        .content {
            padding: 30px;
            max-width: 800px;
            margin: 0 auto;
        }

        .card {
            background-color: #F8F7F9;
            box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
            padding-top: 20px;
            padding-bottom: 20px;
        }

        input[type=text] {
            padding: 6px 12px;
            font-size: 14px;
            color: #555;
            background-color: #fff;
            background-image: none;
            border: 1px solid #ccc;
            border-radius: 4px;
            width: 300px;
        }

        .button {
            background-color: #337ab7;
            color: #fff;
            padding: 7px 15px 9px 15px;
            cursor: pointer;
            border: 0px;
            letter-spacing: 1.2px;
            margin-left:200px;
        }

        .state {
            font-size: 1.5rem;
            color: #8c8c8c;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="home">
        <div class="topnav">
            <h1>Configure Device</h1>
            <h3>Configure your device</h3>
        </div>
        <div class="content">
            <div class="card">
                <label for="devicename" style="display: inline-block; width: 200px; text-align: right;">Device Name </label>
                <input type="text" id="devicename" name="devicename" autocomplete="off" placeholder="Lobby"><br>
                <br>
                <label for="ssid" style="display: inline-block; width: 200px; text-align: right;">Wifi SSID </label>
                <input type="text" id="ssid" name="ssid" autocomplete="off" placeholder="Your Wifi User"><br>
                <br>
                <label for="password" style="display: inline-block; width: 200px; text-align: right;">Wifi Password </label>
                <input type="password" id="password" name="password" autocomplete="off" placeholder="Your Wifi Password"><br>
                <br>
                <label for="location" style="display: inline-block; width: 200px; text-align: right;">Location </label>
                <input type="text" id="location" name="location" autocomplete="off" width="200px" placeholder="Lobby(no-spaces)"><br>
                <br>
                <label for="timezone" style="display: inline-block; width: 200px; text-align: right;">Device Timezone:</label>
                <select id="timezoneselect" name="timezoneselect" onchange='checkvalue(this.value)'>
                    <option value="PST/PDT" selected>PST/PDT</option>
                    <option value="EST/EDT">EST/EDT</option>
                    <option value="CST/CDT">CST/CDT</option>
                    <option value="MST/MDT">MST/CDT</option>
                    <option value="IST">IST</option>
                    <option value="OTH">OTH</option>
                </select>
                <input type="text" id="timezone" name="timezone" style="display: none; margin-left: 275px; margin-top: 15px;" autocomplete="off" placeholder="gmt offset in hours (5.5 or -8) if OTH is selected "><br>
                <br />
                <label for="servicetype" style="display: inline-block; width: 200px; text-align: right;">Service Type</label>
                <select id="servicetype" name="servicetype" onchange='servicelocation(this.value)'>
                    <option value="SingleLocation">No Relgo Service</option>
                    <option value="MultiLocation" selected>With Relgo Service</option>
                </select><br />
                <br />
                <div id="div_multilocations" style="display: inline; padding-top: 15px;">
                    <label for="account" style="display: inline-block; width: 200px; text-align: right;">Service Account </label>
                    <input type="text" id="account" name="account" autocomplete="off" placeholder="Service Admin Account works with  multi locations"><br>
                    <br>
                    <label for="serviceurl" style="display: inline-block; width: 200px; text-align: right;">Service Url </label>
                    <input type="text" id="serviceurl" name="serviceurl" autocomplete="off" placeholder="Service Url works with  multi locations"><br>
                    <br>
                </div>
                <button class="button" onclick="configurecommand()">Configure</button>
            </div>
        </div>
    </div>
    <script>
        function configurecommand() {
            var configcommand = "./configureDevice?";
            var name = document.getElementById('devicename').value;
            var ssid = document.getElementById('ssid').value;
            var password = document.getElementById('password').value;
            var location = document.getElementById('location').value;
            var tzselect = document.getElementById('timezoneselect').value;
            var tz;
            if (tzselect != "OTH")
                tz = tzselect;
            else
                tz = document.getElementById('timezoneselect').value;
            var servicetype = document.getElementById('servicetype').value;
            var account = document.getElementById('account').value;
            var serviceurl = document.getElementById('serviceurl').value;
            var configcommand = "./configureDevice?" + "name=" + name + "&ssid=" + ssid + "&password=" + password + "&location=" + location + "&timezone=" + tz + "&servicetype=" + servicetype + "&admin=" + account + "&server=" + serviceurl;
            console.log(configcommand);
            window.location.assign(configcommand);
        }

        function checkvalue(val) {
            if (val === "OTH")
                document.getElementById('timezone').style.display = 'block';
            else
                document.getElementById('timezone').style.display = 'none';
        }

        function servicelocation(val) {
            if (val === "SingleLocation")
                document.getElementById('div_multilocations').style.display = 'none';
            else
                document.getElementById('div_multilocations').style.display = 'block';
        }

    </script>
</body>
</html>

)configpage";

// forward declarations
unsigned int enroll(String relationId, String deviceRelationId, String empCode);
uint16_t getTemplateForRelationId(String relationId);
void newListner(String deviceRelationId, String listnerCommand, String listnerName, String listnerSchedule);


void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}


String readFile(fs::FS &fs, const char *path) {
  //    Serial.printf("Reading file: %s\r\n", path);

  String readContent;
  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return readContent;
  }

  while (file.available()) {
    char ch = file.read();
    readContent = readContent + String(ch);
  }
  file.close();
  return readContent;
}
int readBuf(fs::FS &fs, const char *path, uint8_t *buf) {
  //    Serial.printf("Reading file: %s\r\n", path);

  int readlen = -1;
  File file = fs.open(path);
  if (file) {
    int len = file.available();
    Serial.println("filelen:" + String(len));
    readlen = file.readBytes((char *)buf, len);
    if (buf == NULL)
      Serial.println("readbuf NULL");
    else
      Serial.println((char *)buf);
  }
  file.close();
  return readlen;
}


void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written:" + String(message));
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
void writeBuf(fs::FS &fs, const char *path, const uint8_t *buf, uint16_t size) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.write(buf, size)) {
    Serial.print("File written:" + String(size));
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");

    return;
  }
  if (file.print(message)) {
    Serial.println("Append:" + String(message));
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}


void SpiInit(void) {
  pinMode(SS_PIN, OUTPUT);
  SPI_T.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);  //spi initialization  //CLK,MISO,MOIS,SS
}

void SpiWriteBurstReg(const uint8_t *buffer, uint16_t num) {
  SPI_T.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SS_PIN, LOW);
  for (int i = 0; i < num; i++) {
    SPI_T.transfer(buffer[i]);
  }
  digitalWrite(SS_PIN, HIGH);
  SPI_T.endTransaction();
}


void SpiReadBurstReg(uint8_t *buffer, uint16_t num) {
  if (buffer == NULL)
    Serial.println("SpiReadBurstReg Buf NULL");

  SPI_T.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  if (buffer == NULL)
    Serial.println("SpiReadBurstReg Buf NULL");
  digitalWrite(SS_PIN, LOW);
  for (int i = 0; i < num; i++) {
    buffer[i] = SPI_T.transfer(0x00);
    //    Serial.print(buffer[i], HEX);
  }
  digitalWrite(SS_PIN, HIGH);
  SPI_T.endTransaction();
}
char *port = NULL;
int speed = 1000;
int timeout = 5;
//uint8_t buffer[512];
uint8_t spiBuffer[512];
uint16_t spiSize[2] = { 256, 256 };
fpc_com_chain_t hcp_chain;

void IRAM_ATTR isr() {
  printf("\n ISR ");
}

void new_function(uint16_t size, const uint8_t *data, uint32_t timeout, void *session) {
  //  printf("new_function::size %d \n", size);
  //  for (int i = 0; i < size; i++)
  //  {
  //    printf("%02X \n", data[i]);
  //  }
  SpiWriteBurstReg(data, size);
}


void recv_function(uint16_t size, uint8_t *data_d, uint32_t timeout, void *session) {
  uint16_t size_d = size;
  //  Serial.println("fn:recv_function");
  //  Serial.println(size);
  //  delay(10);
  int i = 0;
  uint32_t timeout_millis = millis();
  bool is_timeout = true;

  while (1) {
    if (digitalRead(2) == 1) break;
  }
  SpiReadBurstReg(data_d, size);
}
void send_ws_message(char *cr) {
  //  Serial.println("fn::send_ws_message");
  String final_msg = String(cr);
  Serial.println(final_msg);
  //   ws.textAll(final_msg);
}

bool historyExist(int historyId) {
  int *cmdsArray = commandHistory.toArray();
  for (int i = 0; i < commandHistory.getSize(); i++) {
    if (cmdsArray[i] == historyId) {
      free(cmdsArray);
      return true;
    }
  }
  free(cmdsArray);
  return false;
}

AsyncWebServer server(80);
void notFound(AsyncWebServerRequest *request) {
  Serial.print("Not Found");
  request->send(404, "text/plain", "Not found");
}

void initWiFi() {
  WiFi.mode(WIFI_MODE_STA);

  // Read ssid (ssid + ":" + password + ":" + location + ":" + timezone + ":" + localIP + ":" + localGateway);
  String savedSsid = readFile(SPIFFS, "/ssid.txt");
  int loc1 = 0;
  int loc2 = 0;
  if (savedSsid.length() > 0) {
    loc2 = savedSsid.indexOf("^");
    ssid = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    password = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    location = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    timezone = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    localIP = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    localGateway = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    serviceType = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    relgoServer = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    relgoAdmin = savedSsid.substring(loc1, loc2);
    loc1 = loc2 + 1;
    loc2 = savedSsid.indexOf("^", loc1);
    deviceName = savedSsid.substring(loc1, loc2);
    WiFi.begin(ssid, password);
    gateway.fromString(localGateway);

    // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    //   Serial.println("STA Failed to configure");
    // }
  }
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    WiFi.begin(ssid, password);
    retry++;
    delay(1000);
    if (retry > 3) break;
  }

  if (WiFi.status() == WL_CONNECTED) {
    localIP = WiFi.localIP().toString();
    Serial.println(localIP);
    local_IP.fromString(localIP);
    wifiConnected = true;
    Serial.println("wifi Connected");
  } else {
    wifiConnected = false;
    Serial.println("wifi not Connected");
    WiFi.mode(WIFI_AP_STA);
    //Serial.print("AP IP:");
    WiFi.softAP("RelgoWifi", "");
    // Print ESP AP IP Address
    apIP = WiFi.softAPIP().toString();
    Serial.println("ServerStarted" + apIP);
  }
  server.begin();  //Start the server
  Serial.print("Started:");
  Serial.println(localIP);

  Serial.println(ssid);
  Serial.println(password);
  Serial.println(location);
  Serial.println(timezone);
  localIP = WiFi.localIP().toString();
  Serial.println(localIP);
  //  Serial.println(gateway);
  Serial.println(relgoServer);
  Serial.println(relgoAdmin);

  // Set http client timeout to 10000msec. (10secs). RegisterDevice is taking 6secs.
  http.setTimeout(60000);
  // Also WDT timer event also set to 10secs.
  esp_task_wdt_init(60, false);

  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);

  if (timezone == "PST/PDT") {
    gmtOffset_sec = -8 * 3600;
    daylightOffset_sec = -7 * 3600;
  } else if (timezone == "EST/EDT") {
    gmtOffset_sec = -5 * 3600;
    daylightOffset_sec = -4 * 3600;
  } else if (timezone == "CST/CDT") {
    gmtOffset_sec = -6 * 3600;
    daylightOffset_sec = -5 * 3600;
  } else if (timezone == "MST/MDT") {
    gmtOffset_sec = -7 * 3600;
    daylightOffset_sec = -6 * 3600;
  } else if (timezone == "IST") {
    gmtOffset_sec = 5.5 * 3600;
    daylightOffset_sec = 5.5 * 3600;
  } else {
    gmtOffset_sec = 0;
    daylightOffset_sec = 0;
  }
  Serial.println(timezone + "PST offset:" + String(gmtOffset_sec) + "PDT offset:" + String(daylightOffset_sec));
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}
void serial_delay() {
  //  Serial.println("fn::serial_delay");
  //  while (!Serial2.available()) {};
}

void initFP() {
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(SS_PIN, OUTPUT);

  SpiInit();
  delay(100);
  speed = 10000;

  if (!platform_com_init(port, &speed, timeout)) {
    WRITE("Com initialization failed\n");
    exit(1);
  }
  //  send_data_t();
  assign_function((void *)new_function);
  assign2_function((void *)recv_function);
  assign3_function((void *)serial_delay);
  assign_callback_forws((void *)send_ws_message);

  init_com_chain(&hcp_chain, spiBuffer, spiSize, NULL);
}

// Device Listners
// Sa
#define MAX_LISTNER_LENGTH 4
struct ListnerDef {
  String listnerName;
  String listnerCommand;
  String listnerDeviceRid;
  String listnerSchedule;
} listners[MAX_LISTNER_LENGTH];
int listnerCount = 0;

void displayMessage(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 4);
  display.print(F("U:"));
  display.print(template_counter);
  display.print(("   "));
  if (wifiConnected)
    display.println(localIP);
  else
    display.println("Relgo WiFi");
  display.setTextSize(2);
  display.print("\n\r");
  display.println(msg);
  display.setTextSize(1);
  display.print("\n\r");
  display.display();
  delay(1000);
}

String processor(const String &var) {
  Serial.println(var);
  if (var == "STATE") {
    if (ledState) {
      return "ON";
    } else {
      return "OFF";
    }
  }
}

void setup() {
  Serial.begin(115200);
  //SPIFFS initilization begin
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  initWiFi();
  initFP();

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  pinMode(VIBRATE, OUTPUT);

  pinMode(25, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  digitalWrite(25, HIGH);
  delay(10);
  digitalWrite(25, LOW);
  delay(10);
  digitalWrite(VIBRATE, LOW);

  Serial.begin(115200);
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);

  //last read count for templates. TemplateId needs to be unique
  String counter = readFile(SPIFFS, "/counter.txt");
  if (counter.length() == 0)
    template_counter = 0;
  else
    template_counter = counter.toInt();
  //  Serial.println("Template Count:" + template_counter);

  // Setup devices
  // http://server.example.com:2023/?devicerid=00000000-0000-0000-0000-000000000000&devicecommandrid=match;00000000-0000-0000-0000-000000000000$enroll;854789-edfg8-df546d-8df5d&ServiceUrl=http://server.example.com:9011/rfid.aspx
  int dc = 0;
  String dinfo = readFile(SPIFFS, "/devices.txt");
  // First time devices will not be there. Register current device if configuration is set
  //  Serial.println(dinfo);
  int loc1 = 0, loc2 = 0;
  while (loc1 != -1) {
    loc1 = dinfo.indexOf("$DeviceRelationId:", loc1);
    if (loc1 == -1) break;
    loc2 = dinfo.indexOf("$ServiceUrl:", loc1);
    deviceInfo[dc].deviceRelationId = dinfo.substring(loc1 + sizeof("$DeviceRelationId:") - 1, loc2);
    //    Serial.println("Device:" + deviceInfo[dc].deviceRelationId);
    loc1 = loc2;
    loc2 = dinfo.indexOf("$DeviceCommandId:", loc1);
    if (loc1 != -1)
      deviceInfo[dc].serviceUrl = String(dinfo.substring(loc1 + sizeof("$ServiceUrl:") - 1, loc2));
    deviceInfo[dc].serviceUrl.trim();
    loc1 = loc2;
    loc2 = dinfo.indexOf("$DeviceRelationId:", loc2);
    if (loc2 != -1) {  // There are more than one device relations
      deviceInfo[dc].deviceCommands = String(dinfo.substring(loc1 + sizeof("$DeviceCommandId:") - 1, loc2));
    } else {
      deviceInfo[dc].deviceCommands = String(dinfo.substring(loc1 + sizeof("$DeviceCommandId:") - 1));
    }
    loc1 = loc2;
    //    Serial.println("Device:" + String(dc) + ":" + deviceInfo[dc].deviceRelationId + ":" + deviceInfo[dc].serviceUrl + ":" + deviceInfo[dc].deviceCommands);
    dc++;
  }
  deviceCount = dc;  // Set device count from storage

  // Setup listners
  listnerCount = 0;
  int lc = 0;
  loc1 = 0;
  loc2 = 0;
  int idx1 = 0, idx2 = 0;
  String linfo = readFile(SPIFFS, "/listners.txt");
  String nextline;
  char buf[1024];
  //  Serial.println("Listner:" + linfo);
  linfo.trim();
  while (1) {
    //    Serial.println(lc);
    idx2 = linfo.indexOf("\r\n", idx1);
    if (idx2 != -1)
      nextline = linfo.substring(idx1, idx2);
    else {
      nextline = linfo.substring(idx1);
    }
    if (nextline == NULL) break;
    Serial.println("listner:" + linfo);

    idx1 = idx2;
    // Parse newline
    loc1 = 0;
    loc2 = linfo.indexOf(" ", loc1);
    if (loc2 == -1) break;
    listners[lc].listnerName = linfo.substring(loc1, loc2);
    //  Serial.print("listner name:" + listners[lc].listnerName);
    loc1 = loc2 + 1;
    loc2 = linfo.indexOf(" ", loc1);
    listners[lc].listnerCommand = linfo.substring(loc1, loc2);
    //  Serial.print(" command:" + listners[lc].listnerCommand);
    loc1 = loc2 + 1;
    loc2 = linfo.indexOf(" ", loc1);
    listners[lc].listnerDeviceRid = linfo.substring(loc1, loc2);
    //  Serial.println(" device:" + listners[lc].listnerDeviceRid);
    loc1 = loc2 + 1;
    loc2 = linfo.indexOf(" ", loc1);
    listners[lc].listnerSchedule = linfo.substring(loc1, loc2);

    lc++;
  }
  listnerCount = lc;
  Serial.println("Device Count:" + String(deviceCount));
  Serial.println("Listner Count:" + String(listnerCount));
  Serial.println("Setup Complete");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // start OTA
  AsyncElegantOTA.begin(&server);  // Start AsyncElegantOTA

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/configurePage", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", config_html, processor);
  });
}

bool listnersOff = false;

// Get device type for supported sensors by this device
String devicetypesSupported = "FingerPrint;NFC;BlueTooth;Display";

void getDeviceType(AsyncWebServerRequest *request) {
  Serial.println("getDeviceType:" + devicetypesSupported);
  request->send(200, "text/plain", devicetypesSupported.c_str());
}

// Add new device relation to read commands templates

void configureDevice(AsyncWebServerRequest *request) {

  Serial.println("Configure Device");

  if (request->hasParam("help")) {
    request->send(200, "text/plain", "<Access PointIP>/configureDevice ssid=<ssid> password=<password> location=<location> timezone=<PST/PDT> localip=<Device local IP> gateway=<gateway IP Address> server=<RelgoIP> admin=<YourRelgoAdminaddress>");
  }
  if (request->hasParam("name")) {
    deviceName = request->getParam("name")->value();
  }
  if (request->hasParam("ssid")) {
    ssid = request->getParam("ssid")->value();
  }
  if (request->hasParam("password")) {
    password = request->getParam("password")->value();
    Serial.println(password);
  }
  if (request->hasParam("location")) {
    location = request->getParam("location")->value();
  }
  if (request->hasParam("timezone")) {
    timezone = request->getParam("timezone")->value();
  }
  if (request->hasParam("localip")) {
    localIP = request->getParam("localip")->value();
    local_IP.fromString(localIP);
  }
  if (request->hasParam("gateway")) {
    localGateway = request->getParam("gateway")->value();
    gateway.fromString(localGateway);
  }
  if (request->hasParam("servicetype")) {
    serviceType = request->getParam("servicetype")->value();
  }
  if (request->hasParam("server")) {
    relgoServer = request->getParam("server")->value();
  }
  if (request->hasParam("admin")) {
    relgoAdmin = request->getParam("admin")->value();
  }
  Serial.println("Configuring to Wifi");

  Serial.println("Save Configuration");

  // Save ssid and password
  String savessid = String(ssid + "^" + password + "^" + location + "^" + timezone + "^" + localIP + "^" + localGateway + "^" + serviceType + "^" + relgoServer + "^" + relgoAdmin + "^" + deviceName);

  writeFile(SPIFFS, "/ssid.txt", savessid.c_str());  //writes template count to SPIFFS
  request->send(200, "text/plain", "Configure Device Succefull.");

  Serial.println("Configure Device Succefull.");
  delay(1000);
  // ESP.restart();
}

void deleteTemplate(uint16_t templateId);

// Get enrolled template
int getSavedTemplate(uint16_t templateId, uint8_t *data) {
  //fpc_bep_result_t bep_get_template(fpc_com_chain_t *chain, uint16_t template_id, uint8_t *data, uint32_t size)
  Serial.println("getSavedTemplate");
  data = (uint8_t *)malloc(TEMPLATE_SIZE);
  //  fpc_bep_result_t res = bep_get_template(&hcp_chain, data, TEMPLATE_SIZE);
  fpc_bep_result_t res = bep_get_storage_template(&hcp_chain, data, TEMPLATE_SIZE);
  if (res == FPC_BEP_RESULT_OK) {
    //int tsize = hcp_chain.arg.size;
    writeBuf(SPIFFS, "/templatebuf.txt", data, TEMPLATE_SIZE);  //writes template count to SPIFFS
    return true;
  }
  Serial.println("bep_get_template:" + String(res));
  return false;
}


// Get saved template for a template id
void getTemplate(AsyncWebServerRequest *request) {
  uint8_t *buf = NULL;
  uint16_t templateid;
  if (request->hasParam("templateId")) {
    templateid = (uint16_t)request->getParam("templateId")->value().toInt();
    int len = getSavedTemplate(templateid, buf);
    if (len > 0) {
      writeBuf(SPIFFS, "/templatebuf.txt", buf, len);  //writes template count to SPIFFS
    }
  }

  request->send(200, "text/plain", "Saved Template:" + String(templateid));
  free(buf);
}
bool putSavedTemplate(uint16_t templateId, uint8_t *data) {
  //  fpc_bep_result_t res = bep_put_template(&hcp_chain, templateId, data, TEMPLATE_SIZE);
  fpc_bep_result_t res = bep_put_storage_template(&hcp_chain, templateId, data, TEMPLATE_SIZE);
  if (res == FPC_BEP_RESULT_OK) {
    Serial.println("Template put." + String(templateId));
    return true;
  }
  Serial.println("putSavedTemplate:" + String(res));
  return false;
}

void putTemplate(AsyncWebServerRequest *request) {
  uint8_t *buf = (uint8_t *)malloc(2048);
  uint16_t templateid;
  if (request->hasParam("templateId")) {
    templateid = (uint16_t)request->getParam("templateId")->value().toInt();
  }

  int len = readBuf(SPIFFS, "/templatebuf.txt", buf);  //Real template data

  Serial.println("*******readlen:" + String(len));
  bool f = putSavedTemplate(templateid, buf);

  request->send(200, "text/plain", "Put Template:" + String(templateid) + String(f));
}

// Process commands from queue and Execute the command in sequence.
//http://server.example.com:2023/executeCommand?devicerelationid=00000000-0000-0000-0000-000000000000&commandhistoryid=202516&
//commandrelid=854789-edfg8-df546d-8df5d&Parameterdata=userrelationid;75f8d88f49c048a6a343ec29f4e7fe01$ecode;1024~
// Return results
void processCommand(struct PendingCommand pc) {
  Serial.println("Processing:" + pc.command);
  if (pc.command == NULL)
    return;

  int di = -1;
  // &devicecommandrid=match;00000000-0000-0000-0000-000000000000$enroll;854789-edfg8-df546d-8df5d
  struct DeviceCommands {
    String commandName;
    String commandRelationId;
  } deviceCommands[MAX_DEVICE_COMMANDS];

  for (int i = 0; i < deviceCount; i++) {
    //  Serial.println("pcdid:" + deviceInfo[i].deviceRelationId + ":" + pc.deviceRelationId);
    if (pc.deviceRelationId == deviceInfo[i].deviceRelationId) {
      di = i;
      int dc = 0;
      int idx1 = 0, idx2 = 0;
      while (1) {
        idx2 = deviceInfo[di].deviceCommands.indexOf(";", idx1);
        deviceCommands[dc].commandName = deviceInfo[di].deviceCommands.substring(idx1, idx2);
        idx1 = idx2 + 1;
        idx2 = deviceInfo[di].deviceCommands.indexOf("$", idx1);
        deviceCommands[dc].commandRelationId = deviceInfo[di].deviceCommands.substring(idx1, idx2);
        //        Serial.println("dc:" + deviceCommands[dc].commandName + ":" + deviceCommands[dc].commandRelationId);
        dc++;
        idx1 = deviceInfo[di].deviceCommands.indexOf("$", idx2);
        if (idx1 == -1) break;
      }
      break;  // break out from searching matching device
    }
  }
  //Serial.println("Exec:"+pc.command);
  // Compare pc command to device command and invoke corresponding method
  struct DeviceCommands execcmd;  // current command
  for (int i = 0; i < MAX_DEVICE_COMMANDS; i++) {
    //  Serial.println("DC:"+deviceCommands[i].commandRelationId);
    if (pc.command == deviceCommands[i].commandRelationId) {
      execcmd = deviceCommands[i];
      break;
    }
  }
  //  Serial.println(execcmd.commandName);
  // Current command is enroll
  if ((execcmd.commandName.indexOf("Enroll") != -1) || (execcmd.commandName.indexOf("enroll") != -1)) {
    // Turn offlistners
    listnersOff = true;
    int idx1 = pc.parameters.indexOf("userrelationid;") + sizeof("userrelationid;") - 1;
    int idx2 = pc.parameters.indexOf("$");
    String relationId = pc.parameters.substring(idx1, idx2);
    // Serial.println("params:" + pc.parameters);
    // Serial.println("rid:" + relationId);
    // Serial.println("did:" + pc.deviceRelationId);
    // Parse employee code
    idx1 = pc.parameters.indexOf("ecode;") + sizeof("ecode;") - 1;
    idx2 = pc.parameters.indexOf("~");
    String empcode = pc.parameters.substring(idx1, idx2);
    String enrolmsg = String("Enrol ") + empcode;
    //Show employee id on the device.
    displayMessage(enrolmsg);
    delay(1000);

    // enroll the user corresponding to relationId
    int templateid = enroll(relationId, pc.deviceRelationId, empcode);
    //    delay(1000);

    // Get template
    //    uint8_t *templatebuffer;
    Serial.println(String(templateid));
    // if (templateid != -1)
    //   bool bSavedTemplateAvailable = getSavedTemplate(templateid, templatebuffer);

    // Return the response
    //http://server.example.com:9011/rfid.aspx?devicerelationid=00000000-0000-0000-0000-000000000000&
    // userrelationid;00000000-0000-0000-0000-000000000000&historyid=202512&recorded=enroll&output=<templateid>

    String urldata = deviceInfo[di].serviceUrl;
    urldata.trim();
    urldata.concat("?devicerelationid=");
    urldata.concat(deviceInfo[di].deviceRelationId);
    urldata.concat("&userrelationid=");
    urldata.concat(relationId);
    urldata.concat("&historyid=");
    urldata.concat(pc.historyId);
    urldata.concat("&recorded=");
    if (templateid > 0) {
      urldata.concat("Enroll");
      urldata.concat("&output=");
      urldata.concat(templateid);
    } else {
      urldata.concat("fail");
      urldata.concat("&output=");
    }
    Serial.println(urldata);

    http.begin(urldata);
    int httpCode = http.GET();         //Send the request
    if (httpCode > 0) {                //Check the returning code
      String resp = http.getString();  //Get the request response payload
      delay(1000);                     // Give it a delay so that, we don't get timeout exception or reset
    }
    http.end();
    //    free(templatebuffer);
    // Start listners
    listnersOff = false;
  } else if (execcmd.commandName.indexOf("Delete") != -1) {
    int idx1 = pc.parameters.indexOf("userrelationid;") + sizeof("userrelationid;");
    int idx2 = pc.parameters.indexOf("$") - 1;
    String relationId = pc.parameters.substring(idx1, idx2);

    uint16_t templateid = getTemplateForRelationId(relationId);
    deleteTemplate(templateid);

  } else if (execcmd.commandName.indexOf("addListner") != -1) {
    Serial.println("Command addListner:" + pc.parameters);
    int idx1 = pc.parameters.indexOf("listnerName;") + sizeof("listnerName;") - 1;
    int idx2 = pc.parameters.indexOf("$", idx1);
    String listnerName = pc.parameters.substring(idx1, idx2);
    //Serial.println("name:" + listnerName);
    idx1 = pc.parameters.indexOf("listnerCommand;", idx2) + sizeof("listnerCommand;") - 1;
    idx2 = pc.parameters.indexOf("$", idx1);
    String listnerCommand = pc.parameters.substring(idx1, idx2);
    //Serial.println("command:" + listnerCommand);
    idx1 = pc.parameters.indexOf("listnerSchedule;", idx2) + sizeof("listnerSchedule;") - 1;
    idx2 = pc.parameters.indexOf("~");
    String listnerSchedule = pc.parameters.substring(idx1, idx2);
    //Serial.println("schedule:" + listnerSchedule);

    newListner(deviceInfo[di].deviceRelationId, listnerCommand, listnerName, listnerSchedule);
    return;

  } else  // If I receive any other command, send not supported response
  {
    int idx1 = pc.parameters.indexOf("userrelationid;") + sizeof("userrelationid;");
    int idx2 = pc.parameters.indexOf("$") - 1;
    String relationId = pc.parameters.substring(idx1, idx2);
    // Parse employee code
    idx1 = pc.parameters.indexOf("ecode;") + sizeof("ecode;") - 1;
    idx2 = pc.parameters.indexOf("~");
    String empcode = pc.parameters.substring(idx1, idx2);
    String enrolmsg = String("Invalid ") + empcode;
    //Show employee id on the device.
    displayMessage(enrolmsg);


    // Return the response
    //http://server.example.com:9011/rfid.aspx?devicerelationid=00000000-0000-0000-0000-000000000000&
    // userrelationid;00000000-0000-0000-0000-000000000000&historyid=202512&recorded=fail&output=Invalid

    String urldata = deviceInfo[di].serviceUrl;
    urldata.concat("?devicerelationid=");
    urldata.concat(deviceInfo[di].deviceRelationId);
    urldata.concat("&userrelationid=");
    urldata.concat(relationId);
    urldata.concat("&historyid=");
    urldata.concat(pc.historyId);
    urldata.concat("&recorded=");
    urldata.concat("invalid");
    urldata.concat("&output=");

    Serial.println(urldata);

    http.begin(urldata);
    int httpCode = http.GET();
    if (httpCode > 0) {
      http.getString();
      delay(1000);
    }
    http.end();
  }
}

// When the device comes up, it will register for the configured server
// the call returns device command for this device
void registerDevice(String rserver, String radmin, String locate) {
  Serial.println("registerDevice");
  //String urldata = "http://server.example.com:9011/rfid.aspx?purpose=registerdevice&administrator=user@example.com&location=BonneyLake&devicetype=Fingerprint&devicename=ShawTaxesDevice66";
  String drid;
  String payload;
  int retry = 0;

  String urldata = rserver + "/rfid.aspx?purpose=registerdevice&administrator=" + radmin + "&location=" + locate + "&devicetype=Fingerprint&devicename=" + deviceName;
  Serial.println(urldata);
  http.begin(urldata);
  int httpCode = http.GET();
  if (httpCode > 0) {
    payload = http.getString();
  } else
    Serial.println("Error http Code:" + String(httpCode));
  http.end();
  int idx = payload.indexOf("<!DOCTYPE", 0);
  drid = payload.substring(0, idx);
  Serial.println("Response:" + drid);
  if (drid.length() > 0) {
    // Returns add device command
    //  devicerid=00000000-0000-0000-0000-000000000000&
    //  devicecommandrid=match;00000000-0000-0000-0000-000000000000$getenrolluser;854789-edfg8-df546d-8df5d&ServiceUrl=http://server.example.com:9011/rfid.aspx
    int idx1 = drid.indexOf("devicerid=", 0) + sizeof("devicerid=") - 1;
    int idx2 = drid.indexOf("&devicecommandrid=", 0);
    String deviceRelationId = drid.substring(idx1, idx2);
    idx1 = idx2 + sizeof("&devicecommandrid=") - 1;
    idx2 = drid.indexOf("&ServiceUrl=", idx1);
    String deviceCommands = drid.substring(idx1, idx2);
    idx1 = idx2 + sizeof("&ServiceUrl=") - 1;
    String serviceUrl = drid.substring(idx1);
    //  Serial.println(serviceUrl + deviceRelationId + deviceCommands);
    deviceInfo[deviceCount].deviceRelationId = deviceRelationId;
    deviceInfo[deviceCount].serviceUrl = serviceUrl;
    deviceInfo[deviceCount].deviceCommands = deviceCommands;
    deviceCount++;

    String di = "$DeviceRelationId:" + deviceRelationId + "$ServiceUrl:" + serviceUrl + "$DeviceCommandId:" + deviceCommands;  // Save device relation d and commands associated
    appendFile(SPIFFS, "/devices.txt", di.c_str());
  }
}

// Add new device relation to read commands templates
// Structure to hold device information for all device data. It can hold upto 10 devices with 6 commands each.
void addDevice(AsyncWebServerRequest *request) {
  Serial.print("Add Device");
  String deviceRelationId;
  String isUpdate = "false";
  size_t len = 0;
  if (request->hasParam("devicerid")) {
    deviceRelationId = request->getParam("devicerid")->value();
  } else {
    request->send(200, "text/plain", "Device RelationId is required");
    return;
  }
  if (request->hasParam("update")) {
    isUpdate = request->getParam("update")->value();
  }
  //	http://server.example.com:2023?devicerid=00000000-0000-0000-0000-000000000000&
  // devicecommandrid=match;00000000-0000-0000-0000-000000000000$getenrolluser;854789-edfg8-df546d-8df5d&ServiceUrl=http://server.example.com:9011/rfid.aspx

  String deviceCommands;
  String serviceUrl;
  // Get device commands and their relationids for the device. These are used to call back
  if (request->hasParam("devicecommandrid")) {
    deviceCommands = request->getParam("devicecommandrid")->value();
  }
  Serial.println(deviceCommands);
  if (request->hasParam("ServiceUrl")) {
    serviceUrl = request->getParam("ServiceUrl")->value();
  }
  Serial.println(serviceUrl);

  int di = -1;
  // If same device is already present, do not allow add. If update is true, update the parameters
  for (int i = 0; i < deviceCount; i++) {
    if (deviceInfo[i].deviceRelationId == deviceRelationId)
      // device already exists
      di = i;
  }
  if (di != -1) {
    if (isUpdate == "true") {                                                                                                           // update device
      String di = "$UpdatedDeviceRelationId:" + deviceRelationId + "$ServiceUrl:" + serviceUrl + "$DeviceCommandId:" + deviceCommands;  // Save device relation d and commands associated
      appendFile(SPIFFS, "/devices.txt", di.c_str());                                                                                   //writes device Id's to SPIFFS. se these device relation to get commands
      deviceInfo[deviceCount].deviceRelationId = deviceRelationId;
      deviceInfo[deviceCount].serviceUrl = serviceUrl;
      deviceInfo[deviceCount].deviceCommands = deviceCommands;
      deviceCount++;
    }
  } else {
    // Add new device
    String di = "$DeviceRelationId:" + deviceRelationId + "$ServiceUrl:" + serviceUrl + "$DeviceCommandId:" + deviceCommands;  // Save device relation d and commands associated
    appendFile(SPIFFS, "/devices.txt", di.c_str());                                                                            //writes device Id's to SPIFFS. se these device relation to get commands
    deviceInfo[deviceCount].deviceRelationId = deviceRelationId;
    deviceInfo[deviceCount].serviceUrl = serviceUrl;
    deviceInfo[deviceCount].deviceCommands = deviceCommands;

    deviceCount++;
  }

  request->send(200, "text/plain", "Addded Device Successfully:" + deviceRelationId);
}
// Creates new listner for the device. Typically when applications require asynchronous input from the device, it issues this command
// This code establishes a listener from specific input semsor and passon
// ahttp://server.example.com:2023/addListner?devicerid=00000000-0000-0000-0000-000000000000&devicecommandrid=match;00000000-0000-0000-0000-000000000000&listenername="Attendance"
//

void newListner(String deviceRelationId, String listnerCommand, String listnerName, String listnerSchedule) {
  Serial.println(deviceRelationId);
  Serial.println(listnerName);
  Serial.println(listnerCommand);
  Serial.println(listnerSchedule);

  bool alreadyExist = false;
  int listnerIndex = listnerCount;

  for (int i = 0; i < listnerCount; i++) {
    // if listnername already exists, update same listner
    if (listners[i].listnerName.equals(listnerName)) {
      alreadyExist = true;
      listnerIndex = i;
    }
  }
  if ((listnerCommand.indexOf("Match") != -1) || (listnerCommand.indexOf("match") != -1))
  // 0 -FP match, 1 RFID match
  {
    listners[listnerIndex].listnerName = listnerName;
    listners[listnerIndex].listnerCommand = listnerCommand;
    listners[listnerIndex].listnerDeviceRid = deviceRelationId;
    listners[listnerIndex].listnerSchedule = listnerSchedule;

    // Save listners
    String di = String(listners[listnerIndex].listnerName + String(" ") + listners[listnerIndex].listnerCommand + String(" ") + listners[listnerIndex].listnerDeviceRid + String(" ") + listners[listnerIndex].listnerSchedule + String("\r\n"));
    appendFile(SPIFFS, "/listners.txt", di.c_str());  //writes device Id's to SPIFFS. se these device relation to get commands
                                                      //    Serial.println(di);
    listnerCount++;
  }
  listnersOff = false;
}

void addListner(AsyncWebServerRequest *request) {
  String deviceRelationId;
  String listnerCommand;
  String listnerName;
  String listnerSchedule;

  Serial.println("addListner");
  if (request->hasParam("devicerid")) {
    deviceRelationId = request->getParam("devicerid")->value();
  }
  if (request->hasParam("devicecommandrid")) {
    listnerCommand = request->getParam("devicecommandrid")->value();  //
  }
  if (request->hasParam("listnername")) {
    listnerName = request->getParam("listnername")->value();
  }
  if (request->hasParam("listnerschedule")) {
    listnerSchedule = request->getParam("listnerschedule")->value();
  }

  newListner(deviceRelationId, listnerCommand, listnerName, listnerSchedule);
  request->send(200, "text/plain", "Added Listner Successfully:" + listnerName);
}

// Execute command: Add a new command to execution queue. Used for simulating the exeution.
//http://<ServiceUrl>/Rfid.aspx?commandrid=00000000-0000-0000-0000-000000000000&get=yes

void executeCommand(AsyncWebServerRequest *request) {
  //  String query   = "get rfid.aspx?commandrid=00000000-0000-0000-0000-000000000000&get=yes";
  //http://server.example.com:2023/enrolluser?devicerelationid=00000000-0000-0000-0000-000000000000&historyid=202512&commandrelid=00000000-0000-0000-0000-000000000000&Parameterdata=userrelationid;00000000-0000-0000-0000-000000000000$ecode;1021
  // Parse pending command
  //http://server.example.com/rfid.aspx?commandhistoryid=201752&commandid=00000000-0000-0000-0000-000000000000&Parameterdata=User;user@example.com$admin;user@example.com$ecode;1021~

  struct PendingCommand *pencom = &pendingCommand[0];

  if (request->hasParam("commandhistoryid")) {
    pencom->historyId = request->getParam("commandhistoryid")->value().toInt();
  }
  if (request->hasParam("devicerelationid")) {
    pencom->deviceRelationId = request->getParam("devicerelationid")->value();
  }
  if (request->hasParam("commandrelid")) {
    pencom->command = request->getParam("commandrelid")->value();
  }
  if (request->hasParam("Parameterdata")) {
    pencom->parameters = request->getParam("Parameterdata")->value();
  }
  listPendingCommands.add(*pencom);

  request->send(200, "text/plain", pencom->command);
}
// Get all pending commands for the device
//Sample Request :
//http://server.example.com:9011/RFID.aspx?devicerid=00000000-0000-0000-0000-000000000000&getpendingcommands=true
// Parse all pending commands
// http://server.example.com:9011/rfid.aspx?devicerelationid=00000000-0000-0000-0000-000000000000
// &historyid=202512&commandrelid=00000000-0000-0000-0000-000000000000 &Parameterdata=userrelationid;00000000-0000-0000-0000-000000000000$ecode;1021
// &historyid=202523&commandrelid=00000000-0000-0000-0000-000000000000 &Parameterdata=userrelationid;00000000-0000-0000-0000-000000000000$ecode;1021

void getDeviceCommands(int deviceIndex) {
  //String query   = "get rfid.aspx?commandrid=00000000-0000-0000-0000-000000000000&get=yes";
  //Serial.println("getDeviceCommands");
  struct DeviceInfo di = deviceInfo[deviceIndex];
  di.serviceUrl.trim();
  String urldata = di.serviceUrl;
  urldata.concat("?devicerid=");
  urldata.concat(di.deviceRelationId);
  urldata.concat("&getpendingcommands=true");
  //Serial.println(urldata);
  String payload;
  http.begin(urldata);
  int httpCode = http.GET();     //Send the request
  if (httpCode > 0) {            //Check the returning code
    payload = http.getString();  //Get the request response payload
                                 //    Serial.println("Payload Length:" + String(payload.length()));
    delay(1000);                 // Give it a delay so that, we don't get timeout exception or reset
  }
  http.end();
  String payload1;
  int pl = payload.indexOf('<', 0);  // We are gettingviewstate also along with response. If so trim the content
  if (pl != -1) {
    payload1 = payload.substring(0, pl);
  } else
    payload1 = payload;

  payload1.trim();
  if (payload1.length() == 0) return;
  //  Serial.println("Payload Length:" + String(payload1.length()));
  Serial.print("Command:");
  Serial.print(payload1);

  bool moreCommands = true;
  int idx1 = 0, idx2 = 0;
  // Loop until all commands are saved.
  int pc = 0;
  struct PendingCommand *pencom;
  while (1) {
    idx1 = payload1.indexOf("historyid=", idx1);
    if (idx1 == -1) break;
    pencom = &pendingCommand[pc];
    idx1 += sizeof("historyid=") - 1;
    idx2 = payload1.indexOf("&", idx1);
    pencom->historyId = payload1.substring(idx1, idx2).toInt();
    pencom->deviceRelationId = di.deviceRelationId;
    idx1 = payload1.indexOf("commandrelid=", idx2) + sizeof("commandrelid=") - 1;
    idx2 = payload1.indexOf("&", idx1);
    pencom->command = payload1.substring(idx1, idx2);
    idx1 = payload1.indexOf("Parameterdata=", idx2);
    idx2 = payload1.indexOf("~", idx1);
    if (idx2 != -1)
      pencom->parameters = payload1.substring(idx1 + sizeof("Parameterdata=") - 1, idx2);
    else
      pencom->parameters = payload1.substring(idx1 + sizeof("Parameterdata=") - 1);
    //    Serial.println(pencom->historyId);
    if (!historyExist(pencom->historyId)) {
      listPendingCommands.add(*pencom);
      commandHistory.add(pencom->historyId);
    } else
      continue;
    pendingCommandsCount = listPendingCommands.getSize();
    //    Serial.println(String(pencom->historyId) + " " + String(pencom->command));
    pc++;
    if (pc >= MAX_PENDING_COMMANDS) { pc = 0; }
  }
}

struct templateEntry {
  uint16_t currentTemplate;
  String currentDeviceId;
  String currentRelationId;
  String employeeCode;
} currentScan;

void getRelationIdForTemplate(uint16_t template_id) {
  String content = readFile(SPIFFS, "/templates.txt");
  //  Serial.println(content);
  int idx1 = 0, idx2 = 0;
  String nextline;
  while (idx2 < content.length()) {
    int li1 = 0, li2 = 0;
    idx2 = content.indexOf("\r\n", idx1);
    if (idx2 != -1)
      nextline = content.substring(idx1, idx2);
    else
      nextline = content.substring(idx1);

    if (nextline == NULL) break;
    idx1 = idx2 + 2;  // go to next line
    nextline.trim();
    //      Serial.println(nextline);
    li2 = nextline.indexOf(" ", li1);
    currentScan.currentDeviceId = nextline.substring(li1, li2);
    //    Serial.println(currentScan.currentDeviceId);
    li1 = li2 + 1;
    li2 = nextline.indexOf(" ", li1);
    currentScan.currentTemplate = nextline.substring(li1, li2).toInt();
    //    Serial.println(currentScan.currentTemplate);
    li1 = li2 + 1;
    li2 = nextline.indexOf(" ", li1);
    currentScan.currentRelationId = nextline.substring(li1, li2);
    //    Serial.println(currentScan.currentRelationId);

    if (currentScan.currentTemplate == template_id)
      break;
  }
}

uint16_t getTemplateForRelationId(String relationId) {
  String content = readFile(SPIFFS, "/templates.txt");
  //  Serial.println(content);
  int idx1 = 0, idx2 = 0;
  String nextline;
  while (idx2 < content.length()) {
    int li1 = 0, li2 = 0;
    idx2 = content.indexOf("\r\n", idx1);
    if (idx2 != -1)
      nextline = content.substring(idx1, idx2);
    else
      nextline = content.substring(idx1);

    if (nextline == NULL) break;
    idx1 = idx2 + 2;  // go to next line
    nextline.trim();
    //    Serial.println(nextline);
    li2 = nextline.indexOf(" ", li1);
    currentScan.currentDeviceId = nextline.substring(li1, li2);
    //    Serial.println(currentScan.currentDeviceId);
    li1 = li2 + 1;
    li2 = nextline.indexOf(" ", li1);
    currentScan.currentTemplate = nextline.substring(li1, li2).toInt();
    //    Serial.println(currentScan.currentTemplate);
    li1 = li2 + 1;
    li2 = nextline.indexOf(" ", li1);
    currentScan.currentRelationId = nextline.substring(li1, li2);
    //    Serial.println(currentScan.currentRelationId);

    if (currentScan.currentRelationId == relationId)
      break;
  }
  return currentScan.currentTemplate;
}


void getTemplateIds(AsyncWebServerRequest *request) {
  Serial.println("getTemplateIds");
  uint16_t template_ids[250];
  uint32_t template_count;
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;

  res = bep_get_template_count(&hcp_chain, &template_count);
  if (res == FPC_BEP_RESULT_OK) {
    Serial.println("Template Count:" + String(template_count));
  } else
    request->send(200, "text/plain", "No Templates Found");

  //template_ids = (uint16_t*)malloc(template_count);
  res = bep_get_template_ids(&hcp_chain, template_ids, template_count);
  if (res == FPC_BEP_RESULT_OK) {
    for (int i = 0; i < template_count; i++)
      Serial.print(String(template_ids[i]));
    request->send(200, "text/plain", "Template Ids:" + String(template_ids[0]));
  } else
    request->send(200, "text/plain", "get template ids");
}

// enroll remotely by passing template for the relation id
// Save the template for relationid so that enroll user can be provisioned to any location
unsigned int remoteEnroll(String relationId, String deviceRelationId) {
}
void enrollmessages(char *msg) {
  Serial.println("RC:" + String(msg));
  displayMessage(msg);
}

// Call back to display Enroll messages for three times
void (*new_ws_callback)(char *ch);

// Enroll new user for the device
unsigned int enroll(String relationId, String deviceRelationId, String empCode) {
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;
  new_ws_callback = &enrollmessages;

  uint16_t template_id = -1;
  bool match = false;
  listnersOff = true;
  delay(1000);
  //  res = bep_identify_finger(&hcp_chain, &template_id, &match);
  //  if (res == FPC_BEP_RESULT_OK && match) return ALREADY_ENROLLED;  // already enrolled

  // Enroll Finger
  res = bep_enroll_finger(&hcp_chain);
  if (res == FPC_BEP_RESULT_OK) {
    Serial.println("Enroll Succeeded");
  } else {
    Serial.print("Enroll Failed");
    Serial.println(res);
    return -1;
  }
  // Save Template
  template_counter++;

  res = bep_save_template(&hcp_chain, template_counter);
  if (res == FPC_BEP_RESULT_OK) {
    Serial.println("Template saved with ID." + String(template_counter));
  }
  String enrollsuccess = "Success:"+String(template_counter);
  displayMessage(enrollsuccess);

  String logentry = String(deviceRelationId + " " + template_counter + " " + relationId + " " + empCode + "\r\n");
  appendFile(SPIFFS, "/templates.txt", logentry.c_str());
  Serial.println(logentry);
  writeFile(SPIFFS, "/counter.txt", String(template_counter).c_str());
  listnersOff = false;
  return template_counter;
}
void enrollUser(AsyncWebServerRequest *request) {
  String relationId;
  String deviceRelationId;
  String empcode;
  //turn off reading finger print
  if (serviceType != "SingleLocation") {
      request->send(200, "text/plain", "Local enroll not allowed for Multi Location Service configuration." );
      return;
  }

  enrollInprogress = true;

  if (request->hasParam("relationId")) {
    relationId = request->getParam("Parameterdata")->value();
  }
  if (request->hasParam("devicerelationid")) {
    deviceRelationId = request->getParam("devicerelationid")->value();
  }
  if (request->hasParam("empcode")) {
    empcode = request->getParam("empcode")->value();
  }
  delay(1000);
  int tid = enroll(deviceRelationId, relationId, empcode);
  if (tid != -1)
    request->send(200, "text/plain", "Enroll Successfull. Template Id:" + String(tid));
  else
    request->send(200, "text/plain", "Enroll Failed. Error: " + String(tid));

  // Display welcome message
  delay(500);
  if (deviceName.length() == 0)
    displayMessage("Configure");
  else
    displayMessage(deviceName);

  enrollInprogress = false;
}

void matchFinger(AsyncWebServerRequest *request) {
  uint16_t template_id;
  bool match;
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;

  Serial.println("matchFinger.");

  res = bep_identify_finger(&hcp_chain, &template_id, &match);
  if (res == FPC_BEP_RESULT_OK) {
    if (match) {
      char ch[100];
      sprintf(ch, "Match with template id: %d\n", template_id);
      request->send(200, "text/plain", "Matched with ID." + String(template_id));
    } else {
      request->send(200, "text/plain", "No match Found.");
    }
  }
}

// Set device mode scan and command processing
void deviceMode(AsyncWebServerRequest *request) {
  String modeval;

  if (request->hasParam("listnersOff")) {
    modeval = request->getParam("listnersOff")->value();
    if (modeval == "true") listnersOff = true;
    else listnersOff = false;
  }

  if (request->hasParam("commandsoff")) {
    listnersOff = true;
    getCommandsFromRelgo = false;
  }

  if (request->hasParam("restart")) {
    ESP.restart();
    request->send(200, "text/plain", "Success");
  }

  if (request->hasParam("register")) {
    Serial.println("register");
    if (deviceCount == 0) {
      relgoServer.trim();
      relgoAdmin.trim();
      if (!(relgoServer.length() == 0) && !(relgoAdmin.length() == 0))
        registerDevice(relgoServer, relgoAdmin, location);
    }
  }
  if (request->hasParam("info")) {
    Serial.println("info");
    String savedSsid = readFile(SPIFFS, "/ssid.txt");
    String dinfo = readFile(SPIFFS, "/devices.txt");
    String linfo = readFile(SPIFFS, "/listners.txt");
    String content = readFile(SPIFFS, "/templates.txt");
    Serial.println(VERSION_INFO);
    Serial.println(savedSsid);
    Serial.println(dinfo);
    Serial.println(linfo);
    Serial.println(content);
    request->send(200, "text/plain",  "SSID:" + savedSsid + "\n Devices:" + dinfo + "\n Listners:" + linfo + "\n templates:" + content);
    return;
  }
  if (request->hasParam("logentries")) {
    Serial.println("logentries");
    String logentries = readFile(SPIFFS, "/logentries.txt");
    char downloadhtml[1000];
    sprintf(downloadhtml, "%s%s%s%s", logentries_html, "<a href=\"data:text/csv;charset=ascii,",logentries.c_str(), "\" download=\"data.csv\">Download CSV</a></body></html>");
//    request->send(200, "text/plain", logentries);
    request->send(200, "text/html", downloadhtml);
    return;
  }

  Serial.print("Devices Count:" + String(deviceCount));
  request->send(200, "text/plain", "Success");
}

void deleteTemplate(uint16_t templateId) {
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;

  res = bep_delete_template(&hcp_chain, templateId);
  if (res == FPC_BEP_RESULT_OK)
    Serial.println("Templated deleted!");
}
// Reset the device all device relations, templates
void resetDevice(AsyncWebServerRequest *request) {
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;

  if (request->hasParam("logentries")) {
    writeFile(SPIFFS, "/logentries.txt", "");  //writes template count to SPIFFS
  }
  if (request->hasParam("templates")) {
    res = bep_delete_template(&hcp_chain, REMOVE_ID_ALL_TEMPLATES);
    if (res == FPC_BEP_RESULT_OK)
      Serial.println("All templated deleted!");
    template_counter = 0;
    writeFile(SPIFFS, "/counter.txt", "0");    //writes template count to SPIFFS
    writeFile(SPIFFS, "/templates.txt", " ");  //writes template count to SPIFFS
  }
  if (request->hasParam("all")) {
    writeFile(SPIFFS, "/devices.txt", " ");      //writes template count to SPIFFS
    writeFile(SPIFFS, "/listners.txt", " ");     //writes template count to SPIFFS
    writeFile(SPIFFS, "/templatebuf.txt", " ");  //writes template count to SPIFFS
  }
  if (request->hasParam("wipe")) {
    writeFile(SPIFFS, "/devices.txt", " ");      //writes template count to SPIFFS
    writeFile(SPIFFS, "/listners.txt", " ");     //writes template count to SPIFFS
    writeFile(SPIFFS, "/ssid.txt", " ");         //writes template count to SPIFFS
    writeFile(SPIFFS, "/templatebuf.txt", " ");  //writes template count to SPIFFS
  }

  request->send(200, "text/plain", "Success");
}
// match method finger print device
//Request Format :
uint16_t waitForFinger(String listnerName) {
  uint16_t template_id;
  bool match;
  fpc_bep_result_t res = FPC_BEP_RESULT_OK;
  uint16_t timeout;
  uint16_t sleep_counter;
  struct ListnerDef currentListner;

  if (listnerName.length() != 0) {
    for (int i = 0; i < listnerCount; i++) {
      if (listners[i].listnerName == listnerName) {
        currentListner = listners[i];
        break;
      }
    }
  }
  timeout = 5000;
  sleep_counter = 30;
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "Date:%m/%d/%Y,Time:%H:%M", &timeinfo);
  String asString(timeStringBuff);
  char timeStringBuff1[50];
  strftime(timeStringBuff1, sizeof(timeStringBuff1), "%HH:%MM", &timeinfo);
  // Serial.println(millis());
  res = bep_sensor_wait_for_finger(&hcp_chain, timeout, sleep_counter);
  //Serial.println(millis());
  if (res == FPC_BEP_RESULT_OK) {
    res = bep_identify_finger(&hcp_chain, &template_id, &match);
    if (match) {
      Serial.println("Match:" + String(template_id));
      displayMessage("Approved:"+String(template_id));
      delay(500);
      displayMessage(String(timeStringBuff1));
      delay(1000);
      if (deviceName.length() == 0)
        displayMessage("Configure");
      else
        displayMessage(deviceName);

      // if serviceType is SingleLocation, save the record and return
      if (serviceType == "SingleLocation") {
        String logentry = "Template Id=" + String(template_id) + ",Location=" + location + "," + asString + "\r\n";
        appendFile(SPIFFS, "/logentries.txt", logentry.c_str());
        Serial.println(logentry);
        return template_id;
      }
      // If Multi location service continue
      getRelationIdForTemplate(template_id);
      Serial.println(currentScan.currentRelationId);
      // Invoke attendance entry
      for (int i = 0; i < deviceCount; i++) {
        if (deviceInfo[i].deviceRelationId.equalsIgnoreCase(currentScan.currentDeviceId)) {
          //http://server.example.com:9011/rfid.aspx?devicerelationid=00000000-0000-0000-0000-000000000000&commandrelationid=00000000-0000-0000-0000-000000000000
          //&listnername=Attendance&Parameterdata=Date;01/01/2023$Time;12:00$Location;Banglore$userrelationid;00000000-0000-0000-0000-000000000000
          String urldata = deviceInfo[i].serviceUrl;
          urldata.concat("?devicerelationid=");
          urldata.concat(deviceInfo[i].deviceRelationId);

          urldata.concat("&commandrelationid=");
          //          String cid = currentListner.listnerCommand.substring(currentListner.listnerCommand.indexOf("match") + sizeof("match"), currentListner.listnerCommand.length());
          int idx1 = deviceInfo[i].deviceCommands.indexOf(currentListner.listnerCommand) + currentListner.listnerCommand.length() + 1;  //match;
          String cid = deviceInfo[i].deviceCommands.substring(idx1, idx1 + 36);
          urldata.concat(cid);
          urldata.concat("&listnername=");
          urldata.concat(listnerName);
          urldata.concat("&Parameterdata=");
          urldata.concat(asString + "$Location;" + location);
          urldata.concat("$userrelationid;");
          currentScan.currentRelationId.trim();
          urldata.concat(currentScan.currentRelationId);
          Serial.println(urldata);
          int retry = 0;
          if (http.connected()) {
            //            Serial.print("http connected");
            delay(10);
          }
          http.begin(urldata);
          int httpCode = http.GET();
          if (httpCode > 0) {
            String payload = http.getString();
            delay(200);  // Give it a delay so that, we don't get timeout exception or reset
                         //            Serial.println("Length:" + String(payload.length()) + "http Code:" + String(httpCode)+"payload:"+payload);
            String resp = http.errorToString(httpCode);
            //            Serial.println("Response:"+payload);
          } else
            Serial.println("Error http Code:" + String(httpCode));

          http.end();
        }
      }

      return template_id;
    }
    return -2;
  }
  return -1;
}
// the loop function runs over and over again forever
void loop() {
  AsyncElegantOTA.loop();
  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>

  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   showAdminCommands(request);
  // });
  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/addDevice", HTTP_GET, [](AsyncWebServerRequest *request) {
    addDevice(request);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/addListner", HTTP_GET, [](AsyncWebServerRequest *request) {
    addListner(request);
  });

  server.on("/matchFinger", HTTP_GET, [](AsyncWebServerRequest *request) {
    matchFinger(request);
  });

  server.on("/deviceMode", HTTP_GET, [](AsyncWebServerRequest *request) {
    deviceMode(request);
  });

  server.on("/resetDevice", HTTP_GET, [](AsyncWebServerRequest *request) {
    resetDevice(request);
  });

  server.on("/enrollUser", HTTP_GET, [](AsyncWebServerRequest *request) {
    enrollUser(request);
  });

  server.on("/getTemplateIds", HTTP_GET, [](AsyncWebServerRequest *request) {
    getTemplateIds(request);
  });

  server.on("/getTemplate", HTTP_GET, [](AsyncWebServerRequest *request) {
    getTemplate(request);
  });
  server.on("/putTemplate", HTTP_GET, [](AsyncWebServerRequest *request) {
    putTemplate(request);
  });

  server.on("/getDeviceType", HTTP_GET, [](AsyncWebServerRequest *request) {
    getDeviceType(request);
  });

  server.on("/configureDevice", HTTP_GET, [](AsyncWebServerRequest *request) {
    configureDevice(request);
  });

  server.on("/executeCommand", HTTP_GET, [](AsyncWebServerRequest *request) {
    executeCommand(request);
  });


  int dc = 0;
  uint16_t template_id = 0;
  deviceName.trim();
  if (deviceName.length() == 0)
    displayMessage("Configure");
  else
    displayMessage(deviceName);


  while (1) {
    //    delay(1000);
    // If serviceType is SingleLocation, which means they will not use service, instead use admin screen to download
    if (serviceType == "SingleLocation") {
      if (!enrollInprogress) {
        template_id = waitForFinger("");
        if (template_id != 65535) {
          if (template_id == 65534) {
            displayMessage("Failed");
            if (deviceName.length() == 0)
              displayMessage("Configure");
            else
              displayMessage(deviceName);
          }
        }

        delay(1000);
        continue;  // Go back to while loop
      }
    }
    // For each device get commands  andexecute those commands
    if (dc < deviceCount && getCommandsFromRelgo) {
      getDeviceCommands(dc++);
      if (dc >= deviceCount) {
        //        getCommandsFromRelgo = false;  // read command only once from all devices when this flag is set
        dc = 0;
      }
    }

    // If there is scan of finger, detect corresponding relationid for the template, and send back against device.
    for (int i = 0; i < listnerCount; i++) {
      if (listnersOff) break;
      if ((listners[i].listnerCommand.indexOf("Match") != -1) || (listners[i].listnerCommand.indexOf("match") != -1)) {
        displayMessage(listners[i].listnerName);
        template_id = waitForFinger(listners[i].listnerName);
        if (template_id != 65535) {
          if (template_id == 65534)
            displayMessage("Failed");
          // else
          //   displayMessage("Approved");
          //        delay(1000);
        }
      }
    }
    for (int i = 0; i < listPendingCommands.getSize(); i++) {
      //      Serial.println("Processing Command:" + String(listPendingCommands[i].historyId));
      //      Serial.println("Params:" + listPendingCommands[i].parameters);
      processCommand(listPendingCommands[i]);
      listPendingCommands.remove(i);
      //      Serial.println(String(i) + " " + String(listPendingCommands.getSize()));
      delay(500);
    }

    pendingCommandsCount = 0;
  }
}
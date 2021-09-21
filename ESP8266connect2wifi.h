
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>


#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#ifndef STASSID
#define STASSID "wifiSSID"
#define STAPSK  "wifiPW"
#endif

#ifndef APSSID
#define APSSID "AccessPointName"
#define APPSK  ""   //mindestens 6 Zeichen!!!! (max.32)
#endif



//const char* ssid     = STASSID;
//const char* password = STAPSK;

const char* ssid_accessPoint = APSSID;
const char* password_accessPoint = APPSK;

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";

const char* PARAM_SSID = "inputSSID";
const char* PARAM_PW = "inputPW";

time_t t;

int IPnum1[3];
int IPnum2[3];
int IPnum3[3];
int IPnum4[3];

int enable_num1 = 7;
int enable_num2 = 0;
int enable_num3 = 23;
int enable_num4 = 0;

int timeout = 0;
bool is_connected = true;

WiFiUDP ntpUDP;

int status = WL_IDLE_STATUS;

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>website-titel</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);
    }
  </script></head><body bgcolor=00BFFF>
  <font size = "4"> <B>Please enter the name and password of your WIFI Network</B></font><br><br><br>
  <form action="/get" target="hidden-form">
    <font size = "2"> WIFI-Network Name (current Network: %inputSSID%):</font><br> <input type="text" name="inputSSID"><br><br>
    <font size = "2">Password (current password %inputPW%):</font><br> <input type="text" name="inputPW"><br><br>
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";

const char index_html_2[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>connected</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body bgcolor=00BFFF>
  <font size = "4"> <B>You are connected</B></font><br><br><br>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";



void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}



void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Replaces placeholder with stored values
String processor(const String& var){
  //Serial.println(var);
  if(var == "inputSSID"){
    return readFile(SPIFFS, "/inputSSID.txt");
  }
  else if(var == "inputPW"){
    return readFile(SPIFFS, "/inputPW.txt");
  }
  return String();
}

void connect2wifiorstartAccesPoint() {

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.
  //ESP.wdtDisable();
  Serial.begin(9600);

  SPIFFS.begin();
  String str1 = String(processor("inputSSID"));
  String str2 = String(processor("inputPW"));
  
  const char* ssid = str1.c_str();
  const char* password = str2.c_str();


  //Serial.print(ssid);
  //Serial.print(password);


  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && is_connected == true) {
    delay(500);
    timeout++;
    Serial.print(".");
    if(timeout > 20){
      is_connected = false;
    }
  }

    if(is_connected == true){
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html_2, processor);
  });
  server.onNotFound(notFound);
  server.begin();
  
    }
    else{
      if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
      }
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(ssid_accessPoint, password_accessPoint);
      //WiFi.begin(ssid_accessPoint, password_accessPoint);
      //IPAddress myIP = WiFi.softAPIP();
      IPAddress myIP(192,168,4,1);
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      ESP.wdtFeed();
      delay(0);
      yield();      
      
      //server.on("/", handleRoot);
      
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_SSID)) {
      inputMessage = request->getParam(PARAM_SSID)->value();
      writeFile(SPIFFS, "/inputSSID.txt", inputMessage.c_str());
      inputMessage = request->getParam(PARAM_PW)->value();
      writeFile(SPIFFS, "/inputPW.txt", inputMessage.c_str());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
  
  }
}

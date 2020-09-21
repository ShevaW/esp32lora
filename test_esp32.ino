#include <WiFi.h>
#include <WebServer.h>
#include "BLEDevice.h"

/*Put your SSID & Password*/
const char* ssid = "EcozyWork";  // Enter SSID here
const char* password = "cdrgDq5Rtm";  //Enter Password here
WebServer server(80);

// Service und Characteristic des HM-10 Moduls
#define HM_MAC "f8:33:31:60:7e:f6"
static BLEUUID serviceUUID("0000FFE0-0000-1000-8000-00805F9B34FB");
static BLEUUID charUUID("0000FFE1-0000-1000-8000-00805F9B34FB");

//Variables
String val;
String EingangDaten;
BLEClient*  pClient;
static boolean connect = true; 
static boolean connected = false;
static BLEAddress *pServerAddress;
static BLERemoteCharacteristic* pRemoteCharacteristic;

static void notifyCallback 
(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify)
{
  EingangDaten = "";
  for (int i = 0; i < length; i++)EingangDaten += char(*pData++); // Append byte as character to string. Change to the next memory location 
}

bool connectToServer(BLEAddress pAddress)
{
  Serial.println("Trying to Connect with.... ");
  Serial.println(pAddress.toString().c_str());
  pClient = BLEDevice::createClient();
  pClient->connect(pAddress);
  // Obtaining a reference to required service
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);

  if (pRemoteService == nullptr)
  {
    Serial.print("Gefunden falsche UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  // reference to required characteristic
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Gefunden falsche Characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    return false;
  }
  pRemoteCharacteristic->registerForNotify(notifyCallback);
  return true;
}

String SendHTML(String hallsensor){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP32 </title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="<script>\n";
ptr +="setInterval(loadDoc,200);\n";
ptr +="function loadDoc() {\n";
ptr +="var xhttp = new XMLHttpRequest();\n";
ptr +="xhttp.onreadystatechange = function() {\n";
ptr +="if (this.readyState == 4 && this.status == 200) {\n";
ptr +="document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
ptr +="};\n";
ptr +="xhttp.open(\"GET\", \"/\", true);\n";
ptr +="xhttp.send();\n";
ptr +="}\n";
ptr +="</script>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP32 </h1>\n";
  ptr +="<p>val: ";
  ptr +=hallsensor;
  ptr +="</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void handle_OnConnect() {
 val=EingangDaten;
  server.send(200, "text/html", SendHTML(val)); 
}
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Start");
  BLEDevice::init("");
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
if (connect == true) 
  {
    pServerAddress = new BLEAddress(HM_MAC);
    if (connectToServer(*pServerAddress))
    {
      connected = true;
      connect = false;
    }
    else
    {
      Serial.println("Connection does not work");
    }
  }

  if (connected) 
  { 
    Serial.println("CONNECTED");
    //Serial.println(EingangDaten);
  }
  server.handleClient();
}

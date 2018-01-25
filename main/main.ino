#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

char* wifiPass = "Feather1234";
char* wifiSSID = "feather";
char* fileIndexHtml = "/index.html";
int connectionCount = 0;

void setup()
{
  Serial.begin(115200);  
  delay(100);
  Serial.print("[SETUP] Starting ");
  Serial.println("");
  setupFileSystem();
  setupWifiHotspot();
  delay(1000);
  setupServer();
  Serial.println("\n[SETUP] Done\n");
}

void setupFileSystem() {
  Serial.println("\n[FS] File System Setup");
  Serial.println("[FS] -----------------");
  SPIFFS.begin();
  Serial.println("[FS] SPIFFS started, listing files...");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    File f = dir.openFile("r");
    Serial.printf("[FS] %s / size: %d\n", dir.fileName().c_str(), (int) f.size());
    f.close();
  }
}

void setupWifiHotspot() {
  Serial.println("\n[WIFI] Wireless Soft Accesspoint Setup");
  Serial.println("[WIFI] -------------------------------");
  Serial.print("[WIFI] Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("[WIFI] Setting soft-AP ... ");
  Serial.println(WiFi.softAP(wifiSSID,wifiPass,true) ? "Ready" : "Failed!");

  Serial.print("[WIFI] Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  Serial.printf("[WIFI] MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  Serial.printf("[WIFI] Wifi Status = %d\n",WiFi.status());
  Serial.printf("[WIFI] SSID = %s\n",wifiSSID);
  Serial.printf("[WIFI] Password = %s\n",wifiPass);
}

void setupServer() {
  Serial.println("\n[SERVER] HTTP Server Setup");
  Serial.println("[SERVER] -----------------");
  if (MDNS.begin("esp8266")) {
    Serial.println("[SERVER] MDNS responder started");
  }
   
  Serial.printf("[SERVER] Server at %s\n", WiFi.localIP().toString().c_str());

  server.serveStatic("/", SPIFFS, fileIndexHtml);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[SERVER] HTTP server started");  
}

void loop() {
  
  server.handleClient();
  
  if (WiFi.softAPgetStationNum() != connectionCount) {
    connectionCount = WiFi.softAPgetStationNum();
    Serial.printf("[WIFI] connections: %d\n",connectionCount);    
  }
  
}


void handleNotFound(){
  Serial.println("[SERVER] 404");
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

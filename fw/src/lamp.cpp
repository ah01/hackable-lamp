#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>

#include "rf.h"

#define RF_PIN 14
#define LIGHT_PIN 12

// Blue led (0 .. on)
#define LED_PIN 2

// Power of growe modules (Red led) - 1 .. on
#define PWR_PIN 15

//#define WIFI_MODE 1
#define WIFI_MODE 2

bool ap_mode = true;

const char* ssid = "Wireless Eniac";
const char* password = "***";

const char* ap_ssid = "Lampicka";
const char *myHostname = "lampicka";

const char* off = "11111F0FFF0FS";
const char* on = "11111F0FFFF0S";

bool state = false;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

DNSServer dnsServer;

ESP8266WebServer server(80);
RfTransmitter rf(RF_PIN, 150);

inline void ledOn()
{
    digitalWrite(LED_PIN, LOW);
}

inline void ledOff()
{
    digitalWrite(LED_PIN, HIGH);
}


/** Is this an IP? */
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".json")) return "application/json";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  return "text/plain";
}

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void send_rf(const char* code)
{
    Serial.print("Send: ");
    Serial.println(code);

    byte i = 6;
    while (--i > 0)
    {
        rf.send(code);
        delay(0);
    }
}

void onHandler()
{
    ledOff();
    state = true;
    send_rf(on);
    digitalWrite(LIGHT_PIN, LOW);
    server.send(200, "text/plain", "1");
    ledOn();
}

void offHandler()
{
    ledOff();
    state = false;
    send_rf(off);
    digitalWrite(LIGHT_PIN, HIGH);
    server.send(200, "text/plain", "0");
    ledOn();
}

void stateHandler()
{
    ledOff();
    server.send(200, "text/plain", state ? "1" : "0");
    ledOn();
}

void handleNotFound(){
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

bool captivePortal()
{
    if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname)+".local")) {
        Serial.println("Request redirected to captive portal");
        server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
        server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server.client().stop(); // Stop is needed because we sent no content length
        return true;
    }
    return false;
}

void handleRoot() {
    ledOff();
    Serial.println("ROOT");

    if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
        ledOn();
        return;
    }

    handleFileRead("/");
    ledOn();
}


void setup_wifi_client()
{
    Serial.println("WIFI Mode: Client");

    WiFi.begin(ssid, password);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup_wifi_ap()
{
    Serial.println("WIFI Mode: AP");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(ap_ssid);

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
}

void setup()
{
    // LED
    pinMode(LED_PIN, OUTPUT);
    pinMode(LIGHT_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);
    digitalWrite(LIGHT_PIN, HIGH);

    // power on
	pinMode(PWR_PIN, OUTPUT);
	digitalWrite(PWR_PIN, LOW);

    rf.begin();

    Serial.begin(9600);

    Serial.println("Files:");
    SPIFFS.begin();
    {
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
            Serial.printf("FS File: %s, size: %i\n", fileName.c_str(), fileSize);
        }
        Serial.printf("\n");
    }

    setup_wifi_ap();

    server.on("/", handleRoot);

    server.on("/api/on", onHandler);
    server.on("/api/off", offHandler);
    server.on("/api/state", stateHandler);

    if (ap_mode)
    {
        server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
        server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    }

    server.onNotFound([](){
        if(!handleFileRead(server.uri()))
            server.send(404, "text/plain", "FileNotFound");
    });

    server.begin();
    Serial.println("HTTP server started");

    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(PWR_PIN, HIGH);
}

void loop()
{
    if (ap_mode)
    {
        dnsServer.processNextRequest();
    }

    server.handleClient();
}

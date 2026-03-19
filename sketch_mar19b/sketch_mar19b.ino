#include <ESPAsyncWebServer.h>
#include"DHT.h"
#include<WiFi.h>
#define DHT_PIN 4
#define DHT_TYPE DHT11

AsyncWebServer server(80);

const char* ssid = "iPhone";
const char* pass = "jebel123";
const char* html = R"()";

DHT dht(DHT_PIN, DHT_TYPE);


void setup(){
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
  }
  Serial.print("Connected");
  Serial.print("IP : ");
  Serial.print(WiFi.localIP());

  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(200,"text/html",html);
  });
  
}

void loop (){
  delay(2000);
  float t = dht.readTemperature();  
  Serial.println("Temp : ");
  Serial.print(t);
}
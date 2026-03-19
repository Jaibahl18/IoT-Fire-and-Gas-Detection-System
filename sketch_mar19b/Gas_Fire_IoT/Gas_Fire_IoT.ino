#include <ESPAsyncWebServer.h>
#include"DHT.h"
#include<WiFi.h>
#define DHT_PIN 4
#define DHT_TYPE DHT11

AsyncWebServer server(80);
AsyncEventSource events("/events");
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASS;
const char* html = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gas and Fire Detection</title>
    <style>
        body{
            background-color: #07162C;
            height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 5%;
            color: #F4E9E6;
            font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;
        }
        .boxes{
            margin-top: 30px;
            display: flex;
            width: 100vw;
            justify-content: space-around;
        }
        .box {
            background-color: #1C345B;
            width: 40%;
            display: flex;
            flex-direction: column;
            text-align: center;
            
        }
        .box_head{
            color: #B02A3C;
            font-weight: 600;
        }

        h1{
            color: #F4E9E6;
            text-align: center;
            font-weight: 600;
        }
        .remarks p{
            text-align: center;
            
        }


    </style>
</head>
<body>
    <h1>Gas and Fire Detection</h1>
    <div class="boxes">
        <div class="gas box">
            <p class="box_head">Gas Stats</p>
            <p id="gas_value">gas value</p>
            <p id="gas_status">gas remark</p>
        </div>
        <div class="temp box">
            <p class="box_head">Heat Stats</p>
            <p id="temp_value">gas value</p>
            <p id="temp_status">gas remark</p>
        </div>
    </div>
    <div class="remarks">
        <p id="remarks">Lorem ipsum dolor sit amet consectetur adipisicing elit. Facilis atque sint magnam esse aut, dolor illo dignissimos nobis unde ratione!</p>
    </div>
    <button>Call Emergency</button>
    <script>
    let fire_stat = false;
    const source = new EventSource('/events');
    source.addEventListener('temperature',function(e){
        document.getElementById("temp_value").innerText = e.data;
        const temp = parseFloat(e.data);
        if (temp > 27){
            fire_stat = true;
            document.getElementById("temp_status").innerText="High";
            document.getElementById("remarks").innerText="Active Fire in the area. Practice caution";
            
        }
        else{
            fire_stat = false;
            document.getElementById("temp_status").innerText="Normal";
            document.getElementById("remarks").innerText="Conditions Normal";
        }

    });
</script>
</body>

</html>)";

DHT dht(DHT_PIN, DHT_TYPE);


void setup(){
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected");
  Serial.print("IP : ");
  Serial.print(WiFi.localIP());

  server.addHandler(&events);


  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(200,"text/html",html);
  });
  server.begin();
  
}

void loop (){
  delay(2000);
  float t = dht.readTemperature();
    
  events.send(String(t).c_str(),"temperature",millis());
}
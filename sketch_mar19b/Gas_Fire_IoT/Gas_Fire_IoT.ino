#include <ESPAsyncWebServer.h>
#include <MQUnifiedsensor.h>
#include "secrets.h"
#include"DHT.h"
#include<WiFi.h>
#define DHT_PIN 4
#define DHT_TYPE DHT11
#define ON_LED 19
#define CONNECTING_LED 32

#define Board               "ESP-32"
#define Pin                 34        
#define Voltage_Resolution  3.3       
#define ADC_Bit_Resolution  12        
#define RatioMQ6CleanAir    10       

MQUnifiedsensor MQ6(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, "MQ-6");

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
    let gas_stat = false;
    function update_remark(){
        if (gas_stat && fire_stat){
            document.getElementById("remarks").innerText="Active Fire DUE to gas Leak !!!!";
        }
        else if (gas_stat){
            document.getElementById("remarks").innerText="Active gas leak, no ignition yet !!!";
        }
        else if(fire_stat){
            document.getElementById("remarks").innerText="Active Fire NOT DUE to gas Leak !!!!";
        }
        else{
            document.getElementById("remarks").innerText="Normal";
        }
    }
    
    const source = new EventSource('/events');
    source.addEventListener('temperature',function(e){
        document.getElementById("temp_value").innerText = e.data;
        const temp = parseFloat(e.data);
        
        if (temp > 27){
            fire_stat = true;
            document.getElementById("temp_status").innerText="High";
            update_remark();
            
        }
        else{
            fire_stat = false;
            document.getElementById("temp_status").innerText="Normal";
            update_remark();
            
        }

    });
    source.addEventListener('gas',function(e){
        document.getElementById("gas_value").innerText = e.data;
        const gas = parseFloat(e.data);
        if (gas > 25){
            gas_stat = true;
            document.getElementById("gas_status").innerText="leak";
            update_remark();
            
            
        }
        else{
            gas_stat = false;
            document.getElementById("gas_status").innerText="Normal";
            update_remark();
        }

    });
</script>
</body>

</html>)";

DHT dht(DHT_PIN, DHT_TYPE);


void setup(){
  Serial.begin(115200);
  pinMode(ON_LED, OUTPUT);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(CONNECTING_LED, 0); 
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    ledcWrite(0,255);
    delay(200);
    ledcWrite(0,155);
    delay(200);
    ledcWrite(0,55);
    delay(200);
    ledcWrite(0,0);
    delay(200);
    ledcWrite(0,55);
    delay(200);
    ledcWrite(0,155);
    delay(200);
  }
  Serial.print("Connected");
  digitalWrite(ON_LED,HIGH);
  ledcWrite(0,0);
  delay(500);
  Serial.print("IP : ");
  Serial.print(WiFi.localIP());

  server.addHandler(&events);


  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(200,"text/html",html);
  });
digitalWrite(ON_LED,LOW);
ledcWrite(0,255);
  MQ6.setRegressionMethod(1);  // exponential curve
MQ6.setA(1000.5);            // LPG curve constants
MQ6.setB(-2.186);
MQ6.init();

// calibrate in clean air
float calcR0 = 0;
for(int i = 1; i <= 10; i++){
    MQ6.update();
    calcR0 += MQ6.calibrate(RatioMQ6CleanAir);
    delay(500);
}
MQ6.setR0(calcR0 / 10);
digitalWrite(ON_LED,HIGH);
ledcWrite(0,0);
server.begin();
  
}

void loop (){
  delay(2000);
  float t = dht.readTemperature();
  MQ6.update();
  float lpg = MQ6.readSensor();
    
  events.send(String(t).c_str(),"temperature",millis());
  events.send(String(lpg).c_str(),"gas",millis());
  
  Serial.println(lpg);
}
#include <DHT11.h>

#include <Adafruit_BMP085.h>
#include <Adafruit_I2CDevice.h>

#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#endif

// Data wire is connected to GPIO 4
#include "DHT.h"
#define DHTPIN 4 
#define DHTTYPE DHT22

Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);

// Values
String temperatureC = "";
String pressure = "";
String gas = "";
int gas_value;
int temp_value;
char gas_buffer[16];
char temp_buffer[16];

// Combustible gas pin 
const int gas_sensor_pin = 25;

// Pressure sensor pin (assuming analog sensor, if digital, update accordingly)
const int pressure_sensor_pin = 26;

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000;

// Replace with your network credentials
const char* ssid = "42Berlin_Student";
const char* password = "metaverse_student#";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readTemp() {
  String s;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float hic = dht.computeHeatIndex(t, h, false);
  // temp_value = DHT11.read(DHT11PIN);
  itoa(hic, temp_buffer, 10);
  return (temp_buffer);
}

String readGas() {
  gas_value = analogRead(gas_sensor_pin);
  itoa(gas_value, gas_buffer, 10);
  return String(gas_buffer);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP Weather Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Celsius</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tachometer-alt" style="color:#00add6;"></i> 
    <span class="ds-labels">Pressure</span> 
    <span id="pressure">%PRESSURE%</span>
    <sup class="units">hPa</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#ff9800;"></i> 
    <span class="ds-labels">Gas Sensor</span> 
    <span id="gas">%GAS%</span>
    <sup class="units">ppm</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  
  // Temperature
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();

  // Pressure
  xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
  xhttp.send();

  // Gas Sensor
  xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("gas").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/gas", true);
  xhttp.send();

}, 10000) ;
</script>
</html>)rawliteral";

// Replaces placeholder with sensor values
String processor(const String& var){
  if(var == "TEMPERATUREC"){
    return temperatureC;
  } else if(var == "PRESSURE"){
    return pressure;
  } else if(var == "GAS"){
    return gas;
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Routes for sensor values
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", pressure.c_str());
  });
  server.on("/gas", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", gas.c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){
  if ((millis() - lastTime) > timerDelay) {
    temperatureC = readTemp();
    //pressure = readPressure();
    gas = readGas();
    lastTime = millis();
  }
}

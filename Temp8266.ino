#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Connection parms
const char* ssid = "TISCALI-1191";
const char* password = "8HRTRDRLTD";
const char* mqtt_server = "192.168.1.100";
const char* MQTTuser = "MQTTtemp";
const char* MQTTpwd = "Rickyale.temp.73";

// PubSubClient Settings
WiFiClient espClient;
PubSubClient client(espClient);
String switch1;
String strTopic;
String strPayload;
const char* soglia1 = "3300";
const char* soglia2 = "3900";
char temp_send[10];
char hum_send[10];

int buzzer = D2;     //buzzer connesso al pin 3
int Watt;

#define DHTPIN D1     // Digital pin connected to the DHT sensor

#define DHTTYPE    DHT22     // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float temp = 0.0;
float hum = 0.0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 30 seconds
const long interval = 30000;  

// Misc variables
unsigned long timestamp; 

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timestamp = millis();
}


void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);

  if(strTopic == "sensor/Watt"){
    switch1 = String((char*)payload);
    Watt = switch1.toInt();
    //Serial.println(Watt);
    
    if(Watt <= 3300){
      //Serial.println("soglia1");
      noTone(buzzer);  
    }
    
    if(Watt > 3300 && Watt < 3900){
      //Serial.println("soglia2");
      tone(buzzer,1000,500);  
    }

    if(Watt > 3900){
      //Serial.println("soglia3");
      tone(buzzer,500);  
    }
  }

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_Temp", MQTTuser, MQTTpwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("sensor/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retryinge
      delay(5000);
    }
  }
}
 
void setup()
{
  Serial.begin(115200);
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(buzzer,OUTPUT);
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("esp8266-TEMP");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
  
}

void loop()
{
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(10);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float temp = dht.readTemperature();
    //Serial.println(temp);
    // Read Humidity
    float hum = dht.readHumidity();
    //Serial.println(hum);
    String(temp).toCharArray(temp_send, 5);
    String(hum).toCharArray(hum_send, 3);
    //Serial.println(temp_send);
    //Serial.println(hum_send);
    client.publish("sensor/Temp", temp_send);
    client.publish("sensor/Hum", hum_send);
  }


}

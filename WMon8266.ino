#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// Connection parms
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* MQTTuser = "";
const char* MQTTpwd = "";

// PubSubClient Settings
WiFiClient espClient;
PubSubClient client(espClient);
String switch1;
String strTopic;
String strPayload;
char watt_send[50];

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


String readSerial()
{
  while (Serial.available() < 1)
    delay(100);

  char inData[20] = "xxxxxxxxxxxxxxxxxxx"; // Allocate some space for the string
  inData[0] = '\0';
  char inChar = -1; // Where to store the character read
  byte index = 0; // Index into array; where to store the character
  delay(100); // to get all data in one shot
  while (Serial.available() > 0) // Don't read unless there you know there is data
  {
    if (index >= 20) // One less than the size of the array
      break;
    inChar = Serial.read(); // Read a character
    if (inChar == '\r') // is \r\n
      break;
    inData[index] = inChar; // Store it
    index++; // Increment where to write next
    inData[index] = '\0'; // Null terminate the string
    //Serial.println(inChar);
  }
  while (Serial.available() > 0) // Don't read unless there you know there is data
    Serial.read();
  String resp = String(inData);
  resp.trim();
  Serial.println(resp);
  return resp;
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_Emon", MQTTuser, MQTTpwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  Serial.begin(115200);
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("esp8266-EMON");

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

}

void loop()
{
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(10);
  // Get Values
  String fromSerial = readSerial();
  fromSerial.toCharArray(watt_send, fromSerial.length() + 1); //packaging up the data to publish to mqtt
  client.publish("sensor/Watt", watt_send);

}

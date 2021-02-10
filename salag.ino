#include <ESP8266WiFi.h>
#include <PubSubClient.h>

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
String saveswitch1 = " ";
String strTopic;
String strPayload;


// Misc variables
unsigned long timestamp; 

int salaggiu = 0; // Pin per tapparella salag giu
int salagsu = 2; // Pin per tapparella salag su
unsigned long startsalag = 0;
unsigned long endsalag = 31000;
bool firstshot = false;

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

  if(strTopic == "HA/salag/shade"){
    switch1 = String((char*)payload);
    Serial.println(switch1);

    if(switch1 == "RESET") {
      Serial.println("reset");
      saveswitch1 = " ";
      ESP.restart();
    }
    
    if(switch1 == "STOP") {
      Serial.println("FERMA TUTTO");
      digitalWrite(salagsu, LOW);
      digitalWrite(salaggiu, LOW);
      digitalWrite(salagsu, HIGH);
      digitalWrite(salaggiu, HIGH);
      startsalag = 0;
    }

    if(switch1 == "GIU" && switch1 != saveswitch1) {
      Serial.println("fai scendere");
      startsalag = millis();
      digitalWrite(salaggiu, LOW); 
      digitalWrite(salagsu, HIGH); 
      saveswitch1 = switch1;
    }
    
    if(switch1 == "SU" && switch1 != saveswitch1) {
      Serial.println("fai salire");
      startsalag = millis();
      digitalWrite(salagsu, LOW);
      digitalWrite(salaggiu, HIGH);
      saveswitch1 = switch1;
    }
  }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_salag", MQTTuser, MQTTpwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("HA/salag/#");
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
  client.setCallback(callback);
  pinMode(salaggiu, OUTPUT);
  digitalWrite(salaggiu, HIGH);
  pinMode(salagsu, OUTPUT);
  digitalWrite(salagsu, HIGH);

}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (startsalag > 0) {
    if ((millis() - startsalag) > endsalag 
     || (millis() - startsalag) < 0) {
      digitalWrite(salaggiu, HIGH);
      digitalWrite(salagsu, HIGH);      
      startsalag = 0;
      Serial.print("FINE TAPPARELLA");
    }
  }
}

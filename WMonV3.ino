#include <EmonLib.h>



// Create an instance
EnergyMonitor emon1;

int rede = 230.0; // Italia 230V  
unsigned long timestamp; 

void setup()
{
  Serial.begin(115200);   //Apro la comunicazione seriale
 
  emon1.current(1, 17.5);             // Current: input pin, calibration.

}

void loop()
{
  if ((millis() - timestamp) > 999) {
    timestamp = millis();
    double Irms = emon1.calcIrms(1480);  // Calculate Irms only
   //Serial.println(Irms); 
      
    int Watt = Irms*rede;
    //Invia a ESP8266 il valore della Corrente
    Serial.println(Watt); 
  
  }
    
}

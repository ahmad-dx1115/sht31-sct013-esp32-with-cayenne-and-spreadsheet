#define CAYENNE_PRINT Serial
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_SHT31.h>
#include <CayenneMQTTESP32.h>
#include <EmonLib.h>
#include <HTTPClient.h>
#include "time.h"

String GOOGLE_SCRIPT_ID = "AKfycbyO7WNr_Am92ntx35N44VW_gx2rKIUWaTYnGiPa2snMt6yQ5TW2ESREuzHetJ5pwHqkSQ";    // change Gscript ID

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 28800;
const int   daylightOffset_sec = 0;

bool enableHeater = false;
int loopCnt = 0;

EnergyMonitor emon1;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

char username[] =     "eb848d90-3602-11ec-9f5b-45181495093e";
char password[] =     "f492a57c1a5cb6cbb0e4bfc81dbf8ceedd9947d4";
char clientID[] =     "b518d890-d186-11ec-9f5b-45181495093e";
char ssid[]     =     "Redmi K50";
char wifiPassword[] = "qwerty1234";

void setup()
{
  Serial.begin(115200);
  emon1.current(34, 12.2);
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  
if (! sht31.begin(0x44))
{
  Serial.println("Couldn't find SHT31");
while (1) delay(1);
}
Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");


  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop()
{
  delay(1000);
  Cayenne.loop();

if (WiFi.status() == WL_CONNECTED) {
    static bool flag = false;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
  
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  double Power=Irms*240.0;
  Serial.print("Power=");
  Serial.print(Power);           // Apparent power
  Serial.print("W");

  Serial.print(" ");
  Serial.print("Current=");
  Serial.print(Irms);               // Irms
  Serial.print("A");
  Serial.println();
  Cayenne.virtualWrite(1,Irms,"current","a");
  Cayenne.virtualWrite(2,Power,"pow","w");
 

if (! isnan(t))
{
  Serial.print("Temperature= "); Serial.print(t); Serial.println("°C");
  Cayenne.virtualWrite(3,t,"temp","c");
}
else
{
Serial.println("Failed to read temperature");
}

if (! isnan(h))
{
  Serial.print("Humidity = "); Serial.print(h);Serial.println("%");
  Cayenne.virtualWrite(4,h,"rel_hum","p");
}
else
{
Serial.println("Failed to read humidity");
}
Serial.println();
  if (loopCnt >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    loopCnt = 0;
  }
  loopCnt++;
char timeStringBuff[50]; //50 chars should be enough
    strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    String asString(timeStringBuff);
    asString.replace(" ", "-");
    Serial.print("Time:");
    Serial.print(asString);
    Serial.println(); 
    String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"date="+asString+"&temperature="+String(t)+"&humidity="+String(h)+"&current="+String(Irms)+"&power="+String(Power);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
   
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
         
    }
    http.end();
  }


//delay(600000);
}

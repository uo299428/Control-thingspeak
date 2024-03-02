#include <Arduino.h>
#include <HTTPClient.h>
#include <Arduinojson.h>
#include "cloud.h"

const char* WRITE_KEY = "FN6I96FMOMOPGG3M";
const char* WRITE_URL = "http://api.thingspeak.com/update";
const char* READ_LAST_FEED_URL = 
  "https://api.thingspeak.com/channels/2440936/feeds.json?api_key=URU7SQOKO2ZROPHO&results=1";

// Maximum number of consecutive network errors and the related variable
const int MAX_NET_ERRORS = 10;
int net_errors = 0;

// Get control mode integer value from control mode string
int get_control_mode_int(char control_mode[])
{
    int control_mode_int = 0;

    if (!strcmp(control_mode, "on"))
        control_mode_int = 1;
    else if (!strcmp(control_mode, "temp_auto"))
        control_mode_int = 2;
    else if (!strcmp(control_mode, "hum_auto"))
        control_mode_int = 3;

    return control_mode_int;
}

// Get control mode string from control mode integer value
void get_control_mode_str(int control_mode_int, char control_mode[])
{
    if (control_mode_int == 1)
        strcpy(control_mode, "on");
    else if (control_mode_int == 2)
        strcpy(control_mode, "temp_auto");
    else if (control_mode_int == 3)
        strcpy(control_mode, "hum_auto");
    else
        strcpy(control_mode, "off");
}

// Read ThinkSpeak control parameters. It assumes that all control parameters are written in the same feed.
// It returns true when control parameters have been correctly read. 
bool read_cloud(char control_mode[], float &temp_setpoint, float &hum_setpoint)
{
    bool readed = false;
    HTTPClient http;
    http.begin(READ_LAST_FEED_URL);
    int code = http.GET();
    if (code > 0)
    {
        //restarting errors count 
        
        String payload = http.getString();
        DynamicJsonDocument doc(256);
        deserializeJson(doc, payload);

        // If the feed is correct
        if (!doc["feeds"][0].isNull())
        {
            Serial.printf("json readed\n");
            int source = doc["feeds"][0]["field7"];
            // If the feed comes from a remote computer
            if (!doc["feeds"][0]["field7"].isNull() and source == REMOTE_SOURCE)
            {
                if (!doc["feeds"][0]["field6"].isNull())
                {
					get_control_mode_str(doc["feeds"][0]["field6"], control_mode);
                }
                if (!doc["feeds"][0]["field3"].isNull())
                {
					temp_setpoint = doc["feeds"][0]["field3"];
                }
                if (!doc["feeds"][0]["field4"].isNull())
                {
					hum_setpoint = doc["feeds"][0]["field4"];
                }
                readed = true;
                Serial.printf("\tNew data readed from cloud:\n");
                Serial.printf("\tControl mode: %d (%s)\n", get_control_mode_int(control_mode), control_mode);
                Serial.printf("\tTemperaure setpoint: %2.1f\n", temp_setpoint);
                Serial.printf("\tHumidty setpoint: %2.0f\n", hum_setpoint);
                net_errors=0;
            }
            else
            {
                // ????????;
				Serial.printf("\tData readed from cloud but there are no changes\n");
                net_errors=0;
            }
        }
        else
        {        
			// ????????;
        Serial.printf("\tData readed from channel in cloud is null\n");
        net_errors+=1;
        }       
    }
    else
    {   
        // ????????;
        Serial.printf("ThingSpeak read request failed. Error code: %d\n\n", code);
        net_errors+=1;
    }      
    http.end();
    //code here to not change cpp.h creating new method for duplicated code
    if(net_errors >= MAX_NET_ERRORS)
        {
            Serial.println("\nMax net errors reached trying to connect to cloud");
            Serial.println("\nRestarting in 2 seconds...");
            delay(2000);
            ESP.restart();
        }
    return readed;
}

// Write temperature, humidity, control state, setpoints and control mode into ThinkSpeak channel
void write_cloud(float temp, float hum, int control_state, char control_mode[], float temp_setpoint, float hum_setpoint)
{
    HTTPClient http;
    http.begin(WRITE_URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String args = "api_key=" + String(WRITE_KEY) + "&field1=" + String(temp) + "&field2=" + String(hum) +
                  "&field3=" + String(temp_setpoint) + "&field4=" + String(hum_setpoint) + 
                  "&field5=" + String(control_state) + "&field6=" + String(get_control_mode_int(control_mode)) +
                  "&field7=" + String(BOARD_SOURCE);

    int code = http.POST(args);
    if (code > 0)
    {
        //restarting errors count 
		String payload = http.getString();
		if (!payload.compareTo("0"))
        {
            Serial.printf("\tThingSpeak error. Can not write\n");
            net_errors+=1;
        }
        else
        {
            Serial.printf("\tData writed in cloud:\n");
            Serial.printf("\tTemperature: %2.1f\n", temp);
            Serial.printf("\tHumidity: %2.1f\n", hum);
            Serial.printf("\tControl state: %d (%s)\n", control_state, control_state == 1? "on": "off");
            Serial.printf("\tControl mode: %d (%s)\n", get_control_mode_int(control_mode), control_mode);
            Serial.printf("\tTemperaure setpoint: %2.1f\n", temp_setpoint);
            Serial.printf("\tHumidty setpoint: %2.0f\n", hum_setpoint);
            net_errors=0;
        }
    }
    else
    {
		
        Serial.printf("ThingSpeak write request failed. Error code: %d\n\n", code);
        net_errors+=1;
        
    }

    http.end();
    if(net_errors >= MAX_NET_ERRORS)
        {
            Serial.println("\nMax net errors reached trying to connect to cloud");
            Serial.println("\nRestarting in 2 seconds...");
            delay(2000);
            ESP.restart();
        }
}

#include "hal.h"
#include "control.h"

// Control related constants and variables
eeprom_params_t eeprom_params;
const float TEMP_HYSTERESIS = 0.5; // Temperature hysteresis
const float HUM_HYSTERESIS = 5; // Humidity hysteresis
int control_state = OFF; // Current control status (ON or OFF)
float temp; // Current temperature
float hum; // Current humidity

// Temperature/humidity control
void control_temp_hum(void)
{
  // Read variables
  temp = getTemperature();
  hum = getHumidity();


  if (!strcmp(eeprom_params.control_mode, CONTROL_MODE_ON))
    setControlState(ON);
  else if (!strcmp(eeprom_params.control_mode, CONTROL_MODE_TEMP_AUTO))
  {
	   if(temp > eeprom_params.temp_setpoint + TEMP_HYSTERESIS/2 )
        setControlState(OFF);
        else if(temp < eeprom_params.temp_setpoint - TEMP_HYSTERESIS/2 )
        setControlState(ON);
  }
  else if (!strcmp(eeprom_params.control_mode, CONTROL_MODE_HUM_AUTO))
  {
      if(hum > eeprom_params.hum_setpoint + HUM_HYSTERESIS/2 )
        setControlState(ON);
        else if(hum < eeprom_params.hum_setpoint - HUM_HYSTERESIS/2 )
        setControlState(OFF);
  }
  else
      setControlState(OFF);

  control_state = getControlState();
  Serial.printf("\tControl results:\n");
            Serial.printf("\tTemperature: %2.1f\n", temp);
            Serial.printf("\tHumidity: %2.1f\n", hum);
            Serial.printf("\tControl state: %d (%s)\n", control_state, control_state == 1? "on": "off");
            Serial.printf("\tTemperaure setpoint: %2.1f\n", eeprom_params.temp_setpoint);
            Serial.printf("\tHumidty setpoint: %2.0f\n", eeprom_params.hum_setpoint);
}

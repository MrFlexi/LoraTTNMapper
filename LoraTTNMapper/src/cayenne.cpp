/*#include "cayenne.h"


//CAYENNE_OUT_DEFAULT()
//{

  Cayenne.virtualWrite(10, dataBuffer.data.panel_voltage, "voltage", "Millivolts");
  Cayenne.virtualWrite(12, dataBuffer.data.panel_current, "current", "Milliampere");
  //Cayenne.virtualWrite(12, ina3221.getBusVoltage_V(1)*ina3221.getCurrent_mA(1), "pow", "Watts");

  Cayenne.virtualWrite(20, dataBuffer.data.bus_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(21, dataBuffer.data.bus_current, "current", "Milliampere");
  //Cayenne.virtualWrite(22, pmu.getVbusCurrent()/1000*pmu.getVbusVoltage(), "pow", "Watts");

  Cayenne.virtualWrite(30, dataBuffer.data.bat_voltage, "voltage", "Volts");
  Cayenne.virtualWrite(31, dataBuffer.data.bat_charge_current, "current", "Milliampere");
  //Cayenne.virtualWrite(32, pmu.getBattChargeCurrent()*pmu.getBattVoltage()/1000, "pow", "Watts");
  Cayenne.virtualWrite(33, dataBuffer.data.bat_discharge_current, "current", "Milliampere");
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}

*/


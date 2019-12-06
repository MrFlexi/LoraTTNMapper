

#include "globals.h"
#include "dash.h"

AsyncWebServer async_server(80);

void btnCallback(const char* id){
  Serial.println("Button pressed on Dash, ID - "+String(id));

}

void create_web_dash(void)
{
  ESPDash.init(async_server); // Initiate ESPDash and attach your Async webasync_server instance
  // Add Respective Cards

  ESPDash.addNumberCard("PANEL_VOLTAGE", "Panel Voltage", 0);
  ESPDash.addNumberCard("PANEL_CURRENT", "Panel Current", 0); 
  //ESPDash.addNumberCard("PANEL_WATT", "Panel mWatt", 0); 

  ESPDash.addNumberCard("BAT_BUS_VOLTAGE", "Bus Voltage", 0);
  ESPDash.addNumberCard("BAT_BUS_CURRENT", "Bus Current", 0);
  //ESPDash.addNumberCard("BAT_BUS_WATT", "Bus mWatt", 0);
  
  ESPDash.addNumberCard("BAT_VOLTAGE", "Bat Voltage", 0);
  ESPDash.addNumberCard("BAT_CHR_CURRENT", "Charging Current", 0);
  //ESPDash.addNumberCard("BAT_CHR_WATT", "Bat Charg mWatt", 0);
  ESPDash.addNumberCard("BAT_DIS_CURRENT", "Discharge Current", 0);
  

  ESPDash.attachButtonClick(btnCallback);
    // Add Respective Cards
    //ESPDash.addButtonCard("POWER_ON", "Power Rail ON");
    //ESPDash.addButtonCard("POWER_OFF", "Power Rail OFF");
  

  async_server.begin();
}

void update_web_dash(void)
{

  #if (HAS_INA)
  ESPDash.updateNumberCard("PANEL_VOLTAGE", ina3221.getBusVoltage_V(1)*1000);
  ESPDash.updateNumberCard("PANEL_CURRENT", ina3221.getCurrent_mA(1));
  //ESPDash.updateNumberCard("PANEL_WATT", ina3221.getCurrent_mA(1)*ina3221.getBusVoltage_V(1));
  #endif
  
  ESPDash.updateNumberCard("BAT_BUS_VOLTAGE", pmu.getVbusVoltage());
  ESPDash.updateNumberCard("BAT_BUS_CURRENT", pmu.getVbusCurrent());
   //ESPDash.updateNumberCard("BAT_BUS_WATT", pmu.getVbusCurrent()/1000*pmu.getVbusVoltage());

  ESPDash.updateNumberCard("BAT_VOLTAGE", pmu.getBattVoltage());
  ESPDash.updateNumberCard("BAT_CHR_CURRENT", pmu.getBattChargeCurrent());
  //ESPDash.updateNumberCard("BAT_CHR_WATT", pmu.getBattChargeCurrent()*pmu.getBattVoltage()/1000);
  ESPDash.updateNumberCard("BAT_DIS_CURRENT", pmu.getBattDischargeCurrent());

}




#pragma once
void setup_wifi_counter();
void wifi_counter_loop();  
u_int16_t wifi_count_get();
int getMacListCountlastMinutes(int minutes);
String get_wificounter_filename();
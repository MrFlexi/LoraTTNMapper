# Windows
# create py -m venv venv  
# activate in cmd  venv\Scripts\activate


import json
from datetime import datetime

# Gegebene JSON-Daten
json_data = '''
{
    "string": "Hello World",
    "mac_list": [
        {"mac_adr": "DC:39:6F:3B:69:B7", "date": "19700101", "first_seen": "000002", "last_seen": "164810"},
        {"mac_adr": "B0:F2:08:C0:1B:41", "date": "01240707", "first_seen": "102521", "last_seen": "164810"},
        {"mac_adr": "3A:22:E2:96:C4:58", "date": "01240707", "first_seen": "094715", "last_seen": "094715"},
        {"mac_adr": "34:31:C4:B8:68:BF", "date": "01240707", "first_seen": "094715", "last_seen": "094715"}
    ]
}
'''

# JSON-Datei vom Laufwerk lesen
with open('maclist.json', 'r') as file:
    data = json.load(file)

# Anzahl der MAC-Adressen
mac_count = len(data['mac_list'])

def calculate_time_diff(first_seen, last_seen):
    first_seen_time = datetime.strptime(first_seen, '%H%M%S')
    last_seen_time = datetime.strptime(last_seen, '%H%M%S')
    return last_seen_time - first_seen_time

# Zeitdifferenzen berechnen und in der mac_list speichern
for mac in data['mac_list']:
    mac['time_diff'] = calculate_time_diff(mac['first_seen'], mac['last_seen'])

# Liste nach Zeitdifferenz sortieren
data['mac_list'] = sorted(data['mac_list'], key=lambda x: x['time_diff'])

# Anzahl der MAC-Adressen
mac_count = len(data['mac_list'])

# Ausgabe für jede MAC-Adresse
for mac in data['mac_list']:
    mac_adr = mac['mac_adr']
    date = mac['date']
    first_seen = mac['first_seen']
    last_seen = mac['last_seen']
    time_diff = mac['time_diff']
    
    # Zeiten in hh:mm:ss Format
    first_seen_formatted = datetime.strptime(first_seen, '%H%M%S').strftime('%H:%M:%S')
    last_seen_formatted = datetime.strptime(last_seen, '%H%M%S').strftime('%H:%M:%S')
    date_formatted = datetime.strptime(date, '%Y%m%d').strftime('%Y.%m.%d"')
    
    # Ergebnisse ausgeben
    if int(date) > 20240101:
        print(f"MAC-Adresse: {mac_adr} " + f"Date: {date_formatted} " + f"First: {first_seen_formatted} " + f"Last: {last_seen_formatted} "+ f"Differenz: {time_diff}")
    

# Gesamtanzahl der MAC-Adressen ausgeben
print(f"Anzahl der MAC-Adressen: {mac_count}")

import os
import pandas as pd
import glob
from datetime import datetime
import matplotlib.pyplot as plt
from tabulate import tabulate
import json
import plotly.express as px

# Windows
# create py -m venv venv  
# activate in cmd  py gant


# Verzeichnis mit den JSON-Dateien
directory = ''
print("hallo")

# Alle JSON-Dateien im Verzeichnis ladepy gantn
json_files = glob.glob(os.path.join(directory, '*.json'))


# Initialisiere eine leere Liste für die DataFrames
df_list = []

# JSON-Dateien einlesen und Daten in DataFrames umwandeln
for file in json_files:
    with open(file, 'r') as f:
        data = pd.read_json(f)
        # Extrahiere das Array "mac_list"
        mac_list = data.get('mac_list', [])
        
        # Überführe das Array in einen Pandas DataFrame
        
        # Use pd.json_normalize to convert the JSON to a DataFrame
        df_temp = pd.json_normalize(data['mac_list'])
        
        # Füge den DataFrame der Liste hinzu
        df_list.append(df_temp)     
        


# Kombiniere alle DataFrames in einen einzigen DataFrame
df = pd.concat(df_list, ignore_index=True)


# Filtere den DataFrame, um nur Zeilen mit einem Datum >= 20240101 zu behalten
# Eliminate all dates < 2024
df['date'] = pd.to_numeric(df['date'], errors='coerce')
df['first_seen'] = pd.to_numeric(df['first_seen'], errors='coerce')


df = df[(df.date > 20240101)]
#df['start'] = pd.to_datetime(df['date'], errors='coerce',format='%Y%m%d')

# Convert date and time columns to strings
df['date_str'] = df['date'].astype(str)
df['first_str'] = df['first_seen'].astype(str).str.zfill(6)  # Ensure time has 6 digits
df['last_str'] = df['last_seen'].astype(str).str.zfill(6)  # Ensure time has 6 digits

# Combine date and time into a single string
df['start_str'] = df['date_str'] + ' ' + df['first_str']
df['end_str'] = df['date_str'] + ' ' + df['last_str']

# Convert the combined string to a datetime object
df['start_time'] = pd.to_datetime(df['start_str'], format='%Y%m%d %H%M%S')
df['end_time'] = pd.to_datetime(df['end_str'], format='%Y%m%d %H%M%S')

# Drop intermediate columns if needed
df = df.drop(columns=['date_str', 'first_str', 'last_str', 'start_str', 'end_str'])

# Calculate the duration
df = df[(df.end_time != df.start_time)]
df['duration'] = df['end_time'] - df['start_time']

df = df.sort_values(by='duration', ascending=False)





# Ergebnis anzeigen
print(tabulate(df, headers = 'keys', tablefmt = 'psql'))

#Create a Gantt chart
fig = px.timeline(df, 
                  x_start='start_time', 
                  x_end='end_time', 
                  y='mac_adr', 
                  title='Gantt Diagram',
                  labels={'task': 'Tasks'},
                  color='mac_adr')

# Update layout for better readability
fig.update_layout(xaxis_title='Time', 
                  yaxis_title='Tasks', 
                  xaxis=dict(tickformat='%Y-%m-%d %H:%M:%S'))

# Show the plot
fig.show()

import asyncio
import websocket
import json

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

ws_url = "ws://192.168.178.42/ws"  # Die WebSocket-URL, die Sie verwenden möchten

#  .venv\Scripts\activate

gt_measurements = []

def calculate_points(measurements):
    points = []
    for angle, distance in measurements:
        x = round(distance * np.cos(np.radians(angle)))
        y = round(distance * np.sin(np.radians(angle)))
        points.append((x, y))
    return points

    
def extract_motion_sensor(data):
    try:
        #data = json.loads(json_data)
        if 'Sensors' in data and 'MotionSensor' in data['Sensors']:
            yaw = round( data['Sensors']['MotionSensor']['yaw'],1 )
        
        if 'Sensors' in data and 'DistanceSensor' in data['Sensors']:
            distance = round( data['Sensors']['DistanceSensor']['distance'] )
        
        return (yaw,distance)
    except json.JSONDecodeError as e:
        print("Fehler beim Dekodieren des JSON:", e)
        return None

# WebSocket-Event für Nachrichtenempfang hinzufügen
def on_open(ws):
    print("WebSocket connected!")

# Funktion zum Empfangen von Daten über WebSocket
def on_message(ws, message):
    json_data = json.loads(message)  # JSON-Daten parsen
    print("WS input")
    measurement = extract_motion_sensor(json_data)

    if measurement is not None:
        gt_measurements.append(measurement)


# Funktion zum Aktualisieren des Plots
def update(frame):
    points = calculate_points(gt_measurements)
    print(points)
    for x, y in points:
        ax.plot(x, y, 'bo') # 'bo' steht für blaue Kreise
    return line,

# Erstellung des leeren Plots
fig, ax = plt.subplots()
line, = ax.plot([], [], lw=2)

# Einstellungen des Plots
ax.set_aspect('equal')
ax.axhline(0, color='black',linewidth=0.5)
ax.axvline(0, color='black',linewidth=0.5)
ax.set_xlim(-500, 4000)
ax.set_ylim(-500, 4000)
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_title('Echtzeit-Visualisierung')

ws = websocket.WebSocketApp(ws_url, on_message=on_message, on_open=on_open)

# Animation erstellen
ani = FuncAnimation(fig, update, interval=100,save_count=100)
ws.run_forever()
plt.show()



import asyncio
import websockets
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


# Funktion zum Empfangen von Daten über WebSocket
async def receive_data(websocket, path):
     global gt_measurements
     async for message in websocket:
        json_data = json.loads(message)  # JSON-Daten parsen
        print("WS input")
        measurement = extract_motion_sensor(json_data)
        if measurement is not None:
            gt_measurements.append(measurement)
            update_plot()

def update_plot():
    points = calculate_points(gt_measurements)
    last_point = points[-1]
    # Extract x and y data into separate lists using list comprehension
    x_data = [t[0] for t in points]
    y_data = [t[1] for t in points]
    plt.clf()  # Clear the current plot
    plt.scatter(x_data,y_data)  # Plot the points
    plt.xlim(-1500, 1500)  # Set x-axis limits
    plt.ylim(-1500, 1500)  # Set y-axis limits
    plt.xlabel('X')
    plt.ylabel('Y')
    
    x_line = []
    y_line = []

    # Add origin (0, 0) as the starting point of the line
    x_line.append(0)
    y_line.append(0)

# Iterate through the data points and add their coordinates to the line
    if last_point is not None:
        print(last_point)
        x_line.append(last_point[0])
        y_line.append(last_point[1])

        # Plot the line
        plt.plot(x_line, y_line, color='red', linestyle='-', linewidth=2)
    
    plt.title('X-Y Points')
    plt.grid(True)  # Show grid
    num_points = len(x_data)
    plt.text(0, 3500, f'n= {num_points}  w={last_point}', fontsize=12, bbox=dict(facecolor='white', alpha=0.5))
    plt.pause(0.01)  # Pause to allow plot to update

# Main function to connect to WebSocket server
async def main():
    async with websockets.connect(ws_url) as websocket:
        await receive_data(websocket, None)

# Run the main function
asyncio.run(main())



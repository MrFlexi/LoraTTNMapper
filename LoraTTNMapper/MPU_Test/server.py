from flask import Flask, render_template
from flask_sock import Sock
from time import sleep
import time, datetime
import threading
import json

app = Flask(__name__)

#app.config['SOCK_SERVER_OPTIONS'] = {'ping_interval': 25}
sock = Sock(app)

client_list = []

MPU={     
    "yaw" : 0, 
    "pitch" : 0,  
    "roll" : 10,      
    "ServoLeft" :-90,
    "ServoRight" :0,
    "Ki": 1,
    "Kp": 1,
    "Kd": 1
} 

def send_time():
    while True:
        sleep(1)       
        MPU["roll"] = MPU["roll"] + 1
        clients = client_list.copy()
        for client in clients:
            try:
                time_str = { 'text': datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S') }
                client.send(json.dumps(MPU))
            except:
                client_list.remove(client)


@app.route('/')
def index():
    return render_template('index.html')

@sock.route('/echo')
def echo(sock):
    client_list.append(sock)
    sock.send("Webservice connected")    
    while True:
        data = sock.receive()
        print(data)
                
        
if __name__ == '__main__':
    t = threading.Thread(target=send_time)
    t.daemon = True
    t.start()
    app.run(debug = True)
  
import paho.mqtt.client as mqtt
from flask import Flask, render_template, request
import json
import sqlite3

app = Flask(__name__)

def dict_factory(cursor, row):
    d = {}
    for idx, col in enumerate(cursor.description):
        d[col[0]] = row[idx]
    return d

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/esp32/rfidLeitura")
    
def on_message(client, userdata, message):
    if message.topic == "/esp32/rfidLeitura":
        print("Leitura da TAG RFID")

        rfidreadings_json = json.loads(message.payload)

        print(rfidreadings_json)
        
        conn=sqlite3.connect('chave_estrangeira.db')
        
        c=conn.cursor()
		
        try:
          c.execute("""INSERT INTO leitura (UID, data, hora, credito) VALUES ((?), date('now'), time('now', '-2 hour'), 4)""", (rfidreadings_json['UID'],) )
          conn.commit()
        except sqlite3.Error as e:
            print "Database error: ", e
        except Exception as e:
            print "Exception in _query: ", e
        finally:
            if conn:
                conn.close()

mqttc=mqtt.Client()
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.connect("localhost",1883,60)
mqttc.loop_start()

@app.route("/")
def main():   
   conn=sqlite3.connect('chave_estrangeira.db')

   conn.row_factory = dict_factory
   c=conn.cursor()
   c.execute("SELECT l.UID as uid, l.data as data, c.name as name, l.credito as credito FROM leitura as l JOIN apresentacao as a ON l.UID = a.id_leitura JOIN cadastro as c ON c.UID = a.id_cadastro ORDER BY c.name DESC LIMIT 20")
   readings = c.fetchall()
   print(readings)
   return render_template('main.html', readings=readings)

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=8181, debug=True)

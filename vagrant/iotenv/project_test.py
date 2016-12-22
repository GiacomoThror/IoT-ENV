from flask import Flask, render_template, request, redirect, url_for, flash, jsonify
from sqlalchemy import create_engine

from sqlalchemy.orm import sessionmaker
#from database_setup import Base, Restaurant, MenuItem

import paho.mqtt.client as mqtt

app = Flask(__name__)

msg_test = ""


@app.route('/')
@app.route('/iotenv/')
def iotenv():

    mqttc.loop_start()

    flash(msg_test)

    return render_template('iotenv.html')


def on_connect(mqttc, obj, flags, rc):
    print("rc: "+str(rc))


def on_message(mqttc, obj, msg):
    print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
    # json_ser = json.loads(msg.payload)
    # print (json_ser)
    # flash(msg.payload)
    msg_test=msg.payload
    flash(msg_test)


def on_publish(mqttc, obj, mid):
    print("mid: "+str(mid))


def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))


def on_log(mqttc, obj, level, string):
    print(string)

if __name__ == '__main__':

    mqttc = mqtt.Client(client_id="", clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")
    mqttc.on_message = on_message
    mqttc.on_connect = on_connect
    mqttc.on_publish = on_publish
    mqttc.on_subscribe = on_subscribe

    mqttc.connect(host="iot.eclipse.org", port=1883, keepalive=60, bind_address="")
    (rc, mid) = mqttc.subscribe(topic="iotenv/1", qos=2)

    app.secret_key = 'super_secrete_key'
    app.debug = True
    app.run(host='0.0.0.0', port=5000)

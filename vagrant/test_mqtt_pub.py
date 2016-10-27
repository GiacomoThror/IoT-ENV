#!/usr/bin/python

# Copyright (c) 2010-2013 Roger Light <roger@atchoo.org>
#
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Distribution License v1.0
# which accompanies this distribution.
#
# The Eclipse Distribution License is available at
#   http://www.eclipse.org/org/documents/edl-v10.php.
#
# Contributors:
#    Roger Light - initial implementation
# Copyright (c) 2010,2011 Roger Light <roger@atchoo.org>
# All rights reserved.

# This shows a simple example of an MQTT subscriber.

import sys
import time
import random
import json
try:
    import paho.mqtt.client as mqtt
except ImportError:
    # This part is only required to run the example from within the examples
    # directory when the module itself is not installed.
    #
    # If you have the module installed, just use "import paho.mqtt.client"
    import os
    import inspect
    cmd_subfolder = os.path.realpath(os.path.abspath(os.path.join(os.path.split(inspect.getfile(inspect.currentframe()))[0], "../src")))
    if cmd_subfolder not in sys.path:
        sys.path.insert(0, cmd_subfolder)
    import paho.mqtt.client as mqtt


class WheaterStation(object):
    """docstring for sensor"""

    def __init__(self, station_id, number_of_sensors):
        super(WheaterStation, self).__init__()
        self.station_id = station_id
        self.sensors_number = number_of_sensors
        self.datetime = time.strftime("%Y-%m-%d %H:%M:%S")
        self.topic = "iotenv/" + str(station_id)
        self.sensors = []
        for i in range(0, number_of_sensors):
            self.sensors.append(0)

    def getTimeNow(self):
        self.datetime = time.strftime("%Y-%m-%d %H:%M:%S")

    def randomValue(self, sensor_position):
        self.sensors[sensor_position] = random.random()

    def serialize(self):
        """Return object data in easily serializeable format"""
        json_ser = {}
        json_ser['station_id'] = self.station_id
        json_ser['datetime'] = self.datetime
        json_ser['sensors'] = self.sensors

        return json.dumps(json_ser)


def on_connect(mqttc, obj, flags, rc):
    print("rc: "+str(rc))


def on_message(mqttc, obj, msg):
    print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))


def on_publish(mqttc, obj, mid):
    print("mid: "+str(mid))
    pass


def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))


def on_log(mqttc, obj, level, string):
    print(string)


def main():

    print ("START PROGRAM")
    # If you want to use a specific client id, use
    # mqttc = mqtt.Client("client-id")
    # but note that the client id must be unique on the broker. Leaving the client
    # id parameter empty will generate a random id for you.
    mqttc = mqtt.Client(client_id="", clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")

    mqttc.on_message = on_message
    mqttc.on_connect = on_connect
    mqttc.on_publish = on_publish
    mqttc.on_subscribe = on_subscribe
    # Uncomment to enable debug messages
    # mqttc.on_log = on_log
    mqttc.connect(host="m2m.eclipse.org", port=1883, keepalive=60, bind_address="")

    mqttc.loop_start()

    w1 = WheaterStation(1, 2)
    while(1):
        w1.randomValue(0)
        w1.randomValue(1)
        w1.getTimeNow()
        print(w1.serialize())
        info = mqttc.publish(topic=w1.topic, payload=w1.serialize(), qos=2)
        info.wait_for_publish()
        time.sleep(10)

    print ("END PROGRAM")

if __name__ == '__main__':
    main()

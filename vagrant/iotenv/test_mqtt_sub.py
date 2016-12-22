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
# import json
# import sys
import paho.mqtt.client as mqtt

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

# from database_setup import Restaurant, Base, MenuItem

# engine = create_engine('sqlite:///weather_station.db')
# Bind the engine to the metadata of the Base class so that the
# declaratives can be accessed through a DBSession instance
# Base.metadata.bind = engine

# DBSession = sessionmaker(bind=engine)
# A DBSession() instance establishes all conversations with the database
# and represents a "staging zone" for all the objects loaded into the
# database session object. Any change made against the objects in the
# session won't be persisted into the database until you call
# session.commit(). If you're not happy about the changes, you can
# revert all of them back to the last commit by calling
# session.rollback()
# session = DBSession()


def on_connect(mqttc, obj, flags, rc):
    print("rc: "+str(rc))


def on_message(mqttc, obj, msg):
    print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
    # json_ser = json.loads(msg.payload)
    # print (json_ser)


def on_publish(mqttc, obj, mid):
    print("mid: "+str(mid))


def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))


def on_log(mqttc, obj, level, string):
    print(string)


def main():
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
    mqttc.connect(host="iot.eclipse.org", port=1883, keepalive=60, bind_address="")
    (rc, mid) = mqttc.subscribe(topic="iotenv/1", qos=2)

    mqttc.loop_forever()

if __name__ == '__main__':
    main()

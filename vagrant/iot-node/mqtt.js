var mqtt = require('mqtt')

var client = mqtt.connect('mqtt://iot.eclipse.org')


client.on('connect', () => {
  client.subscribe('iotenv/1');
})

client.on('message', (topic, message) => {
    console.log(message.toString());
})

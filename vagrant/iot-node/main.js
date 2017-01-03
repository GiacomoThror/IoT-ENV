var express = require('express');
var app = express();
var server = require('http').createServer(app);
var io = require('socket.io')(server);
var mqtt = require('mqtt');

var mqtt_client = mqtt.connect('mqtt://iot.eclipse.org');

app.use(express.static(__dirname + '/bower_components'));
app.get('/', function(req, res, next) {
    res.sendFile(__dirname + '/index.html');

});

server.listen(5000);

io.on('connection', function(client) {

    console.log('Client connected...');

    mqtt_client.subscribe('iotenv/1');

    client.on('join', function(data) {
        console.log(data);
        //client.emit('messages', 'Hello from server');
    });

});

mqtt_client.on('connect', () => {
    // mqtt_client.subscribe('iotenv/1');
});

mqtt_client.on('message', (topic, message) => {
    console.log(message.toString());
    io.emit("mqtt", message.toString());
});

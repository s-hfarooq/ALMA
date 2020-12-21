const express = require('express');
const path = require('path');
const app = express(),
      bodyParser = require("body-parser");
      port = 3080;
var net = require('net');

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../my-app/build')));
var client = new net.Socket();

// app.post('/colorChanger', (req, res) => {
//     console.log(req.body.color);
//     var client = new net.Socket();
//
//     client.connect(10000, '192.168.0.237', function() {
//     	console.log('Connected');
//       client.write(req.body.color);
//       client.destroy();
//       console.log('Destroyed');
//     });
//
// });

app.post('/connectChanger', (req, res) => {
    client.connect(10000, '192.168.1.126', function() {
    	console.log('Connected');
    });

    res.json({connection: "connected"})
});

app.post('/endConnection', (req, res) => {
    client.destroy();
    console.log('Destroyed');
    res.json({connection: "ended"})
});

app.post('/changeColor', (req, res) => {
    console.log(req.body.color);
    client.write(req.body.color);
    res.json({connection: "changed col"})
});

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, '../my-app/build/index.html'));
});

client.on('error', function(exception) {
  console.log('SOCKET ERROR');
  client.destroy();
})

client.on('close', function(exception) {
  console.log('SOCKET CLOSED');
})

app.listen(port, () => {
    console.log(`Server listening on the port::${port}`);
});

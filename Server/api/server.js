const express = require('express');
const path = require('path');
const app = express(),
      bodyParser = require("body-parser");
      port = 3080;
var net = require('net');
var lastSentTime = new Date();

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../my-app/build')));
var client = new net.Socket();

app.post('/connectChanger', (req, res) => {
  client.connect(10000, '192.168.1.126', function() {
  	console.log('Connected');
  });

  res.json({connection: "Connected"})
});

app.post('/endConnection', (req, res) => {
  client.destroy();
  console.log('Destroyed');
  res.json({connection: "Ended"})
});

app.post('/changeColor', (req, res) => {
  let currTime = new Date();
  if(currTime - lastSentTime > 20) {
    lastSentTime = currTime;
    console.log(req.body.color);
    client.write(req.body.color);
    res.json({ connection: "Changed col" })
  } else {
    res.json({ conection: "Not enough time between commands" })
  }
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

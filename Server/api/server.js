const express = require('express');
const path = require('path');
const app = express(),
      bodyParser = require("body-parser");
      port = 3080;
var net = require('net');


// place holder for the data
const users = [];

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../my-app/build')));

app.post('/colorChanger', (req, res) => {
    console.log(req.body.color);
    var client = new net.Socket();

    client.connect(10000, '192.168.0.237', function() {
    	console.log('Connected');
      client.write(req.body.color);
      client.destroy();
      console.log('Destroyed');
    });
});

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, '../my-app/build/index.html'));
});

app.listen(port, () => {
    console.log(`Server listening on the port::${port}`);
});

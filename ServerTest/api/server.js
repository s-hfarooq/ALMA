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

app.get('/api/users', (req, res) => {
  console.log('api/users called!')
  res.json(users);
});

app.post('/api/user', (req, res) => {
  const user = req.body.user;
  console.log('Adding user:::::', user);
  users.push(user);
  res.json("user addedd");
});

app.post('/post-feedback', function (req, res) {
    console.log(req.body.comment);
    var client = new net.Socket();

    client.connect(10000, '192.168.0.237', function() {
    	console.log('Connected');
      client.write(req.body.comment);
      client.destroy();
      console.log('Destroyed');
    });

    //res.send('Data received:\n' + JSON.stringify(req.body));
});

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, '../my-app/build/index.html'));
});

app.listen(port, () => {
    console.log(`Server listening on the port::${port}`);
});

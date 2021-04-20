#!/usr/bin/env node

const express = require('express');
const path = require('path');
const app = express(),
      bodyParser = require("body-parser");
      port = 3080;
var net = require('net');

var lastSentTime = new Date();

var spawn = require('child_process').spawn;
var child = spawn('python3',["i2c_raw.py"]);

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../frontend-controller/build')));

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, '../frontend-controller/build/index.html'));
});

app.post('/sendCommand', (req, res) => {
  console.log("COMMAND: " + req.body.color);

  let currTime = new Date();
  if(currTime - lastSentTime > 250) {
    child.stdin.setEncoding('utf-8');
    child.stdin.write(req.body.color + "\n");
    lastSentTime = currTime;
  }

  res.json({connection: "Sent"})
});

app.listen(port, () => {
  console.log(`Server listening on the port::${port}`);
});

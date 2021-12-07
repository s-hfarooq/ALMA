#!/usr/bin/env node

const express = require('express');
const path = require('path');
const app = express(),
      bodyParser = require("body-parser");
      port = 3080;
var net = require('net');
var cors = require('cors');

var lastSentTime = new Date();

// Spawn Python script to be able to send signals to ESP32 root node over i2c
var spawn = require('child_process').spawn;
var child = spawn('python3',["i2c_raw.py"]);

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../frontend-controller/build')));
app.use(cors());

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, '../frontend-controller/build/index.html'));
});


// Send command via Python script
app.post('/sendCommand', (req, res) => {
  console.log("COMMAND: " + req.body.color);

  // Only send commands every 250ms
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

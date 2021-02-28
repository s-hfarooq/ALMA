#!/usr/bin/env node

const express = require('express');
const path = require('path');
const app = express(),
      bodyParser = require("body-parser");
      port = 3080;
var net = require('net');

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../frontend-controller/build')));

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, '../frontend-controller/build/index.html'));
});

app.post('/sendCommand', (req, res) => {
  console.log("COMMAND: " + req.body.color);
  const spawn = require("child_process").spawn;
  const pythonProcess = spawn('python3',["i2cpi.py", req.body.color]);

  res.json({connection: "Sent"})
});

app.listen(port, () => {
  console.log(`Server listening on the port::${port}`);
});

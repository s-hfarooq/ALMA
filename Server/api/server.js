#!/usr/bin/env node

const express = require('express');
const path = require('path');
const app = express(),
      bodyParser = require("body-parser");
      port = 3080;
var net = require('net');

var lastSentTimeCeiling = new Date();
var lastSentTimeCouch = new Date();

var ceilingIP = '192.168.1.102';
var couchIP = '192.168.1.149';

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '../frontend-controller/build')));

var clientCeiling = new net.Socket();
var clientCouch = new net.Socket();

app.post('/connectChangerCeiling', (req, res) => {
  clientCeiling.connect(10000, ceilingIP, function() {
  	console.log('Connected');
  });

  res.json({connection: "Connected"})
});

app.post('/endConnectionCeiling', (req, res) => {
  clientCeiling.destroy();
  console.log('Destroyed');
  res.json({connection: "Ended"})
});

app.post('/changeColorCeiling', (req, res) => {
  let currTime = new Date();
  if(currTime - lastSentTimeCeiling > 20) {
    lastSentTimeCeiling = currTime;
    console.log(req.body.color);
    clientCeiling.write(req.body.color);
    res.json({ connection: "Changed col" })
  } else {
    res.json({ conection: "Not enough time between commands" })
  }
});

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, '../frontend-controller/build/index.html'));
});

clientCeiling.on('error', function(exception) {
  console.log('SOCKET ERROR');
  clientCeiling.destroy();
});

clientCeiling.on('close', function(exception) {
  console.log('SOCKET CLOSED');
});

app.post('/connectChangerCouch', (req, res) => {
  clientCouch.connect(10000, couchIP, function() {
  	console.log('Connected');
  });

  res.json({connection: "Connected"})
});

app.post('/endConnectionCouch', (req, res) => {
  clientCouch.destroy();
  console.log('Destroyed');
  res.json({connection: "Ended"})
});

app.post('/changeColorCouch', (req, res) => {
  let currTime = new Date();
  if(currTime - lastSentTimeCouch > 20) {
    lastSentTimeCouch = currTime;
    console.log(req.body.color);
    clientCouch.write(req.body.color);
    res.json({ connection: "Changed col" })
  } else {
    res.json({ conection: "Not enough time between commands" })
  }
});

clientCouch.on('error', function(exception) {
  console.log('SOCKET ERROR');
  clientCouch.destroy();
});

clientCouch.on('close', function(exception) {
  console.log('SOCKET CLOSED');
});

app.listen(port, () => {
  console.log(`Server listening on the port::${port}`);
});

var express = require('express');
var path = require('path');
var bodyParser = require('body-parser');
var net = require('net');

var app = express();

app.use(bodyParser.urlencoded({ extended: false }));
app.use(express.static(path.resolve(__dirname, 'public')));
console.log("starting...");

app.post('/post-feedback', function (req, res) {
    console.log(req.body.comment);
    var client = new net.Socket();
    
    client.connect(10000, '192.168.0.237', function() {
    	console.log('Connected');
      client.write(req.body.comment);
      client.destroy();
      console.log('Destroyed');
    });

    res.send('Data received:\n' + JSON.stringify(req.body));
});

app.listen(process.env.PORT || 3000, process.env.IP || '0.0.0.0' );

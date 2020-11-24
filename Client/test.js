var net = require('net');

var client = new net.Socket();
client.connect(10000, '192.168.0.237', function() {
	console.log('Connected');
  for(let i = 0; i < 10000000; i++) {
    str = 'col 255 0 ' + (i % 255);
    str2 = 'col2 0 255 ' + ((10000000 - i) % 255);
    client.write(str);
    client.write(str2);
  }
	// client.write('col 255 0 255');
  // client.write('col2 0 255 0');
});

client.on('data', function(data) {
	console.log('Received: ' + data);
	client.destroy(); // kill client after server's response
});

client.on('close', function() {
	console.log('Connection closed');
});

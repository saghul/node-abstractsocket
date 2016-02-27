'use strict';
const abs = require('./lib/abstract_socket');

const client = abs.connect('\0foo2', function() { //'connect' listener
    console.log('client connected');
});

client.on('data', function(data) {
    console.log(data.toString());
});

process.stdin.setEncoding('utf8');
process.stdin.on('readable', function() {
    const chunk = process.stdin.read();
    if (chunk !== null)
        client.write(chunk);
});

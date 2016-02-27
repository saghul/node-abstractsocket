'use strict';

const abs = require('./lib/abstract_socket');

const server = abs.createServer(function(c) { //'connection' listener
    console.log('client connected');
    c.on('end', function() {
        console.log('client disconnected');
    });
    c.write('hello\r\n');
    c.pipe(c);
});
server.listen('\0foo2');


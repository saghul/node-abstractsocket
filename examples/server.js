'use strict';

require('../lib/abstract_socket')
    .createServer(client => {
        console.log('client connected');
        client.on('end', () => {
            console.log('client disconnected');
        })
            .pipe(client)
            .write('hello from server\r\n');
    })
    .listen('\0foo2');


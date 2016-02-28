'use strict';

const spawn = require('child_process').spawn;
const should = require('should'); // jshint ignore: line
const abs = require('../lib/abstract_socket.js');

const SOCKET_NAME = '\0test312';
const SOME_DATA = 'asdqq\n';

describe('server', () => {
    describe('listening', () => {
        let server;
        beforeEach(() => (server = abs.createServer()) && server.listen(SOCKET_NAME));
        afterEach(() => server.close());

        it('should listen on abstract socket', () => exec('lsof -U')
            .then(output => output.should.containEql(`@${SOCKET_NAME.slice(1)}`)));

        it('should emit error when socket is busy', done => {
            const server = abs.createServer();
            server.listen(SOCKET_NAME);
            server.on('error', err => {
                err.syscall.should.equal('listen');
                done();
            });
        });
    });

    describe('client connections', () => {
        let server;
        beforeEach(() => (server = abs.createServer()) && server.listen(SOCKET_NAME));
        afterEach(() => server.close());

        it('should emit event when client connects', done => {
            server.on('connection', () => done());
            abs.connect(SOCKET_NAME);
        });

        it('should receive client data', done => {
            server.on('connection', client => {
                client.on('data', data => {
                    data.toString().should.equal(SOME_DATA);
                    done();
                });
            });
            abs.connect(SOCKET_NAME).write(SOME_DATA);
        });
    });

    describe('messages', () => {
        let server;
        beforeEach(() => (server = abs.createServer()) && server.listen(SOCKET_NAME));
        afterEach(() => server.close());

        it('should be received from the client', done => {
            server.on('connection', client => {
                client.on('data', data => {
                    data.toString().should.equal(SOME_DATA);
                    done();
                });
            });
            const client = abs.connect(SOCKET_NAME, () => {
                client.write(SOME_DATA);
            });
        });
    });
});

describe('client', () => {
    describe('should emit error', () => {
        it('when connecting to a non existent socket', done => {
            abs.connect('\0non-existent-socket').on('error', () => done());
        });

        it('when connecting to a non abstract socket', done => {
            abs.connect('non-abstract-socket').on('error', () => done());
        });
    });

    describe('connect callback', () => {
        let server;
        beforeEach(() => (server = abs.createServer()) && server.listen(SOCKET_NAME));
        afterEach(() => server.close());

        it('should be called when connected', done => {
            abs.connect(SOCKET_NAME, () => done());
        });

        it('should be called asynchronously', done => {
            let counter = 0;
            abs.connect(SOCKET_NAME, () => {
                counter.should.equal(1);
                done();
            });
            ++counter;
        });
    });

    describe('messages', () => {
        let server;
        beforeEach(() => (server = abs.createServer()) && server.listen(SOCKET_NAME));
        afterEach(() => server.close());

        it('should be sent to the server', done => {
            server.on('connection', client => {
                client.on('data', data => {
                    data.toString().should.equal(SOME_DATA);
                    done();
                });
            });
            const client = abs.connect(SOCKET_NAME, () => {
                client.write(SOME_DATA);
            });
        });
    });
});

function exec(cmd, options) {
    return new Promise((resolve, reject) => {
        let bin = cmd.split(' ').shift();
        let params = cmd.split(' ').slice(1);
        let child = spawn(bin, params, options);
        let res = new Buffer(0);
        let err = new Buffer(0);

        child.stdout.on('data', buf => res = Buffer.concat([res, buf], res.length + buf.length));
        child.stderr.on('data', buf => err = Buffer.concat([err, buf], err.length + buf.length));
        child.on('close', code => {
            return setImmediate(() => {
                // setImmediate is required because there are often still
                // pending write requests in both stdout and stderr at this point
                if (code) {
                    reject(err.toString());
                } else {
                    resolve(res.toString());
                }
            });
        });
        child.on('error', reject);
    });
}


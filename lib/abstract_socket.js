
var net = require('net');
var binding = require('bindings')('abstract_socket.node');

var socket  = binding.socket;
var bind    = binding.bind;
var connect = binding.connect;
var close   = binding.close;


function errnoException(errorno, syscall) {
  var e = new Error(syscall + ' ' + errorno);
  e.errno = e.code = errorno;
  e.syscall = syscall;
  return e;
}


exports.startListening = function(path, connectionListener, listeningListener) {
    var err = socket();
    if (err < 0)
        throw errnoException(err, 'socket');

    var handle = {fd: err};

    err = bind(err, path);
    if (err < 0) {
        close(handle.fd);
        throw errnoException(err, 'bind');
    }

    var server = net.createServer(connectionListener);
    server.listen(handle, listeningListener);
    return server;
};


exports.connect = function(path, connectListener) {
    var err = socket();
    if (err < 0)
        throw errnoException(err, 'socket');

    var options = {fd: err, readable: true, writable: true};

    // yes, connect is synchronous, so sue me
    err = connect(err, path);
    if (err < 0) {
        close(options.fd);
        throw errnoException(err, 'connect');
    }

    return new net.Socket(options);
};


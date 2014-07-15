# node-abstractsocket

Because I like my sockets like my Picasso paintings: abstract.


## Abstract what?

Go read this: http://man7.org/linux/man-pages/man7/unix.7.html, I'll wait.


## Examples

Server:

```js
// abstract echo server
var abs = require('./lib/abstract_socket');

var server = abs.createServer(function(c) { //'connection' listener
  console.log('client connected');
  c.on('end', function() {
    console.log('client disconnected');
  });
  c.write('hello\r\n');
  c.pipe(c);
});
server.listen('\0foo');

```

Client:

```js
var abs = require('./lib/abstract_socket');

var client = abs.connect('\0foo', function() { //'connect' listener
    console.log('client connected');
});

client.on('data', function(data) {
    console.log(data.toString());
});

process.stdin.setEncoding('utf8');
process.stdin.on('readable', function() {
    var chunk = process.stdin.read();
    if (chunk !== null)
        client.write(chunk);
});

```


## API

### abs.createServer(connectionListener)

Returns a new `AbstractSocketServer` object. `listen` can be called on
it passing the name of the abstract socket to bind to and listen, it follows
the API used for normal Unix domain sockets. NOTE: you must prepend the path with
the NULL byte ('\0') to indicate it's an abstract socket.

Throws an exception if the `socket(2)` system call fails.

### AbstractSocketServer.listen(name, [callback]

Binds the server to the specified abstract socket name.

Throws an exception if the `bind(2)` system call fails, or the given `name`
is invalid.

This function is asynchronous. When the server has been bound, 'listening' event
will be emitted. the last parameter callback will be added as an listener for the
'listening' event.

### abs.connect(name, connectListener)

Creates a connection to the given `path` in the abstract domain. NOTE: you must
prepend the path with the NULL byte ('\0') to indicate it's an abstract
socket.

Returns a new and connected net.Socket object.

Throws an exception if the `socket(2)` or `connect(2)` system calls fail,
or the given `name` is invalid.


## Thanks

I borrowed massive amounts of inspiration/code from node-unix-dgram by @bnoordhuis :-)

@mmalecki taught me how to inherit like a pro.


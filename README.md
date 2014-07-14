# node-abstractsocket

Because I like my sockets like my Picasso paintings: abstract.


## Abstract what?

Go read this: http://man7.org/linux/man-pages/man7/unix.7.html, I'll wait.


## API

### abs.startListening(path, connectionListener, [listeningListener])

Returns a new net.Server object which has been bound to the given path
and it's already listening. NOTE: you must not prepend the path with
the NULL byte ('\0') to indicate it's an abstract socket, it's done internally.

Throws an exception if the `socket(2)` or `listen(2)` system calls fail.

The optional `listeningListener` argument is added as a listener for the
`'listening'` event.

### abs.connect(path, connectListener)

Creates a connection to the given `path` in the abstract domain. NOTE: you must
not prepend the path with the NULL byte ('\0') to indicate it's an abstract
socket, it's done internally.

Returns a new and connected net.Socket object.

Throws an exception if the `socket(2)` or `connect(2)` system calls fail.


## Thanks

I borrowed massive amounts of inspiration/code from node-unix-dgram by @bnoordhuis :-)


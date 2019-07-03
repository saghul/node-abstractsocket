#if !defined(__linux__)
# error "Only Linux is supported"
#endif

#include <nan.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>


namespace {

using v8::FunctionTemplate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;


NAN_METHOD(Socket) {
    int fd;
    int type;

    assert(info.Length() == 0);

    type = SOCK_STREAM;
    type |= SOCK_NONBLOCK | SOCK_CLOEXEC;

    fd = socket(AF_UNIX, type, 0);
    if (fd == -1) {
        fd = -errno;
        goto out;
    }

out:
    info.GetReturnValue().Set(fd);
}


NAN_METHOD(Bind) {
    sockaddr_un s;
    socklen_t namelen;
    int err;
    int fd;
    unsigned int len;

    assert(info.Length() == 2);

    fd = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
    Nan::Utf8String path(info[1]);

    if ((*path)[0] != '\0') {
        err = -EINVAL;
        goto out;
    }

    len = path.length();
    if (len > sizeof(s.sun_path)) {
        err = -EINVAL;
        goto out;
    }

    memset(&s, 0, sizeof s);
    memcpy(s.sun_path, *path, len);
    s.sun_family = AF_UNIX;
    namelen = offsetof(struct sockaddr_un, sun_path) + len;

    err = 0;
    if (bind(fd, reinterpret_cast<sockaddr*>(&s), namelen))
        err = -errno;

out:
    info.GetReturnValue().Set(err);
}


NAN_METHOD(Connect) {
    sockaddr_un s;
    socklen_t namelen;
    int err;
    int fd;
    unsigned int len;

    assert(info.Length() == 2);

    fd = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
    Nan::Utf8String path(info[1]);

    if ((*path)[0] != '\0') {
        err = -EINVAL;
        goto out;
    }

    len = path.length();
    if (len > sizeof(s.sun_path)) {
        err = -EINVAL;
        goto out;
    }

    memset(&s, 0, sizeof s);
    memcpy(s.sun_path, *path, len);
    s.sun_family = AF_UNIX;
    namelen = offsetof(struct sockaddr_un, sun_path) + len;

    err = 0;
    if (connect(fd, reinterpret_cast<sockaddr*>(&s), namelen))
        err = -errno;

out:
    info.GetReturnValue().Set(err);
}


NAN_METHOD(Close) {
    int err;
    int fd;

    assert(info.Length() == 1);
    fd = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

    // POSIX 2008 states that it unspecified what the state of a file descriptor
    // is if close() is interrupted by a signal and fails with EINTR.  This is a
    // problem for multi-threaded programs since if the fd was actually closed
    // it may already be reused by another thread hence it is unsafe to attempt
    // to close it again.  In 2012, POSIX approved a clarification that aimed
    // to deal with this mess:  http://austingroupbugs.net/view.php?id=529#c1200
    //
    // The short summary is that if the fd is never valid after close(), as is
    // the case in Linux, then <unistd.h> should add:
    //
    //      #define POSIX_CLOSE_RESTART 0
    //
    // and the new posix_close() should be implemented as something like:
    //
    //      int posix_close(int fd, int flags) {
    //          int r = close(fd);
    //          if (r < 0 && errno == EINTR)
    //              return 0 or set errno to EINPROGRESS;
    //          return r;
    //      }
    //
    // In contrast, on systems where EINTR means the close() didn't happen (like
    // HP-UX), POSIX_CLOSE_RESTART should be non-zero and if passed as flag to
    // posix_close() it should automatically retry close() on EINTR.
    //
    // Of course this is nice and all, but apparently adding one constant and
    // a trivial wrapper was way too much effort for the glibc project:
    //      https://sourceware.org/bugzilla/show_bug.cgi?id=16302
    //
    // But since we actually only care about Linux, we don't have to worry about
    // all this anyway.  close() always means the fd is gone, even if an error
    // occurred.  This elevates EINTR to the status of real error, since it
    // implies behaviour associated with close (e.g. flush) was aborted and can
    // not be retried since the fd is gone.

    err = 0;
    if (close(fd))
        err = -errno;

    info.GetReturnValue().Set(err);
}


void Initialize(Local<Object> target) {
    Nan::Set(target, Nan::New("socket").ToLocalChecked(),
             Nan::GetFunction(Nan::New<FunctionTemplate>(Socket)).ToLocalChecked());
    Nan::Set(target, Nan::New("bind").ToLocalChecked(),
             Nan::GetFunction(Nan::New<FunctionTemplate>(Bind)).ToLocalChecked());
    Nan::Set(target, Nan::New("connect").ToLocalChecked(),
             Nan::GetFunction(Nan::New<FunctionTemplate>(Connect)).ToLocalChecked());
    Nan::Set(target, Nan::New("close").ToLocalChecked(),
             Nan::GetFunction(Nan::New<FunctionTemplate>(Close)).ToLocalChecked());
}


} // anonymous namespace

NODE_MODULE(abstract_socket, Initialize)

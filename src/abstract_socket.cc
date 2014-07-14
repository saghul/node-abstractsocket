// -D_GNU_SOURCE makes SOCK_NONBLOCK etc. available on linux
#undef  _GNU_SOURCE
#define _GNU_SOURCE

#if !defined(__linux__)
# error "Only Linux is supported"
#endif

#include <nan.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>


namespace {

using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;


#if !defined(SOCK_NONBLOCK)
void SetNonBlock(int fd) {
  int flags;
  int r;

  flags = fcntl(fd, F_GETFL);
  assert(flags != -1);

  r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  assert(r != -1);
}
#endif


#if !defined(SOCK_CLOEXEC)
void SetCloExec(int fd) {
  int flags;
  int r;

  flags = fcntl(fd, F_GETFD);
  assert(flags != -1);

  r = fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
  assert(r != -1);
}
#endif


NAN_METHOD(Socket) {
  NanScope();
  int fd;
  int type;

  assert(args.Length() == 0);

  type = SOCK_STREAM;
#if defined(SOCK_NONBLOCK)
  type |= SOCK_NONBLOCK;
#endif
#if defined(SOCK_CLOEXEC)
  type |= SOCK_CLOEXEC;
#endif

  fd = socket(AF_UNIX, type, 0);
  if (fd == -1) {
    fd = -errno;
    goto out;
  }

#if !defined(SOCK_NONBLOCK)
  SetNonBlock(fd);
#endif
#if !defined(SOCK_CLOEXEC)
  SetCloExec(fd);
#endif

out:
  NanReturnValue(NanNew(fd));
}


NAN_METHOD(Bind) {
  NanScope();
  sockaddr_un s;
  socklen_t namelen;
  int err;
  int fd;
  unsigned int len;

  assert(args.Length() == 2);

  fd = args[0]->Int32Value();
  String::Utf8Value path(args[1]);

  len = path.length();
  if (len > sizeof(s.sun_path)-1) {
      err = -EINVAL;
      goto out;
  }

  memset(&s, 0, sizeof s);
  memcpy(s.sun_path + 1, *path, len);
  s.sun_family = AF_UNIX;
  namelen = offsetof(struct sockaddr_un, sun_path) + 1 + len;

  err = 0;
  if (bind(fd, reinterpret_cast<sockaddr*>(&s), namelen))
    err = -errno;

out:
  NanReturnValue(NanNew(err));
}


NAN_METHOD(Connect) {
  NanScope();
  sockaddr_un s;
  socklen_t namelen;
  int err;
  int fd;
  unsigned int len;

  assert(args.Length() == 2);

  fd = args[0]->Int32Value();
  String::Utf8Value path(args[1]);

  len = path.length();
  if (len > sizeof(s.sun_path)-1) {
      err = -EINVAL;
      goto out;
  }

  memset(&s, 0, sizeof s);
  memcpy(s.sun_path + 1, *path, len);
  s.sun_family = AF_UNIX;
  namelen = offsetof(struct sockaddr_un, sun_path) + 1 + len;

  err = 0;
  if (connect(fd, reinterpret_cast<sockaddr*>(&s), namelen))
    err = -errno;

out:
  NanReturnValue(NanNew(err));
}


void Initialize(Handle<Object> target) {
  target->Set(NanNew("socket"),
              NanNew<FunctionTemplate>(Socket)->GetFunction());

  target->Set(NanNew("bind"),
              NanNew<FunctionTemplate>(Bind)->GetFunction());

  target->Set(NanNew("connect"),
              NanNew<FunctionTemplate>(Connect)->GetFunction());
}


} // anonymous namespace

NODE_MODULE(abstract_socket, Initialize)

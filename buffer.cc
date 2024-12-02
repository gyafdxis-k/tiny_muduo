#include "buffer.h"
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
// 从fd上读取数据 poller工作在LT模式
// buffer缓冲区是有大小的！但是从fd上读数据的时候，却不知道tcp数据最终的大小
ssize_t Buffer::readFd(int fd, int *saveErrno) {
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    const size_t writeable = writeableBytes();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);
    if (n < 0) {
        *saveErrno = errno;
    } else if (n <= writeable) {
        writeIndex_ += n;
    } else {
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *saveErrno = errno;
    }
    return n;
}

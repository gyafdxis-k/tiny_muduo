#pragma once
#include <vector>
#include <string>
class Buffer {

public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writeIndex_(kCheapPrepend) {

        }
    size_t readableBytes() const {
        return writeIndex_ - readerIndex_;
    }

    size_t writeableBytes() const {
        return buffer_.size() - writeIndex_; 
    }

    size_t prependableBytes() const {
        return readerIndex_;
    }

    const char* peek() const {
        return begin() + readerIndex_;
    }

    // onMessage string <- Buffer
    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            // len == readableBytes(); readerIndex += len - writerIndex;
            retrievelAll();
        }
    }

    void retrievelAll() {
        readerIndex_ = writeIndex_ = kCheapPrepend;
    }

    // onMessage 函数上报的buffer数据，转成string 类型返回
    std::string retrievelAllAsString() {
        return retrievelAsString(readableBytes());
    }
    std::string retrievelAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void ensureWriteableBytes(size_t len) {
        if (writeableBytes() < len) {
            makeSpace(len);
        }
    }

    // 把data_len内存上的数据添加到writeable缓冲区当中
    void append(const char *data, size_t len) {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writeIndex_ += len;
    }


    char* beginWrite() {
        return begin() + writeIndex_;
    }
    
    const char* beginWrite() const {
        return begin() + writeIndex_;
    }

    ssize_t readFd(int fd, int *saveErrno);

    ssize_t writeFd(int fd, int *saveErrno);

private:
    char* begin() {
        return &*buffer_.begin();
    }

    const char* begin() const {
        return &*buffer_.begin();
    }
    void makeSpace(size_t len) {
        if (writeableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writeIndex_ + len);
        } else {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writeIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writeIndex_ = readerIndex_ + readable;
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writeIndex_;
};
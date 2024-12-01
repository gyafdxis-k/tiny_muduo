#pragma once
/*
noncopyable被继承后，派生类可以正常的构造和析构，但是不能拷贝构造和复制构造
*/
class noncopyable {
    public:
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
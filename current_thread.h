#pragma once
namespace CurrentThread
{
    extern __thread int t_cached_tid;
    void cacheTid();
    inline int tid() {
        if (__builtin_expect(t_cached_tid == 0, 0)) {
            cacheTid();
        }
        return t_cached_tid;
    }
} // namespace CurrentThrea

#include "current_thread.h"
#include <unistd.h>
#include <sys/syscall.h>
namespace CurrentThread
{
    __thread int t_cache_tid = 0;
    void cacheTid()
    {
        if (t_cached_tid == 0)
        {
            t_cache_tid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }

} // namespace CurrentThread

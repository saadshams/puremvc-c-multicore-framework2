#include "puremvc/platform.h"
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#else
    #include <unistd.h>
    #include <sys/resource.h>
    #include <sys/time.h>
    #if defined(__linux__)
        #include <stdlib.h>
    #endif
#endif

int process_stats_collect(struct process_stats *out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));

#if defined(_WIN32)
    HANDLE hProcess = GetCurrentProcess();

    // 1. Memory Stats
    PROCESS_MEMORY_COUNTERS memCounters;
    if (GetProcessMemoryInfo(hProcess, &memCounters, sizeof(memCounters))) {
        out->max_rss_mb = (double)memCounters.PeakWorkingSetSize / (1024.0 * 1024.0);
        out->current_rss_mb = (double)memCounters.WorkingSetSize / (1024.0 * 1024.0);
        out->major_page_faults = (long)memCounters.PageFaultCount;
    }

    // 2. CPU Stats
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
        // Windows FILETIME is in 100-nanosecond intervals
        ULARGE_INTEGER ut, kt;
        ut.LowPart = userTime.dwLowDateTime;
        ut.HighPart = userTime.dwHighDateTime;
        kt.LowPart = kernelTime.dwLowDateTime;
        kt.HighPart = kernelTime.dwHighDateTime;

        out->user_cpu_sec = (double)ut.QuadPart / 10000000.0;
        out->system_cpu_sec = (double)kt.QuadPart / 10000000.0;
    }
    // Note: Context switches aren't easily available per-process on Windows without ETW.

#else
    // --- Existing POSIX Logic ---
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0) return -1;

#if defined(__APPLE__)
    out->max_rss_mb = usage.ru_maxrss / (1024.0 * 1024.0);
#elif defined(__linux__)
    out->max_rss_mb = usage.ru_maxrss / 1024.0;
#endif

    out->user_cpu_sec = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
    out->system_cpu_sec = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
    out->minor_page_faults = usage.ru_minflt;
    out->major_page_faults = usage.ru_majflt;
    out->voluntary_ctx_switches = usage.ru_nvcsw;
    out->involuntary_ctx_switches = usage.ru_nivcsw;

#if defined(__linux__)
    FILE *fp = fopen("/proc/self/statm", "r");
    if (fp) {
        long size = 0, resident = 0;
        if (fscanf(fp, "%ld %ld", &size, &resident) == 2) {
            long page_size = sysconf(_SC_PAGESIZE);
            out->current_rss_mb = (resident * page_size) / (1024.0 * 1024.0);
        }
        fclose(fp);
    }
#endif
#endif

    return 0;
}
